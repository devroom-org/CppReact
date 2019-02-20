#pragma once

#include <cppreact/attribute.hpp>

#include <memory>
#include <ostream>
#include <utility>
#include <vector>

namespace cppreact::details
{
	class element
	{
	public:
		explicit element(const std::string& name)
			: name_(name)
		{}
		element(const std::string& name, const std::string& str)
			: name_(name), body_str_(str)
		{}
		element(const element& element)
			: name_(element.name_), attributes_(element.attributes_), body_(element.body_), body_str_(element.body_str_),
			is_closing_tag_(element.is_closing_tag_), parent_(element.parent_)
		{}
		element(element&& element) noexcept
			: name_(std::move(element.name_)), attributes_(std::move(element.attributes_)), body_(std::move(element.body_)), body_str_(std::move(element.body_str_)),
			is_closing_tag_(element.is_closing_tag_), parent_(element.parent_)
		{}
		virtual ~element() = default;
		
	public:
		element& operator=(const element&) = delete;
		element operator!() const
		{
			element result(*this);
			result.is_closing_tag_ = true;
			return result;
		}

	public:
		void add_body(const std::shared_ptr<element>& element)
		{
			body_.push_back(element);
		}
		template<typename Attribute_>
		void add_attribute(Attribute_&& attribute)
		{
			attributes_.push_back(std::forward<Attribute_>(attribute));
		}
		std::shared_ptr<const element> get_last_body() const
		{
			return body_.back();
		}
		std::shared_ptr<element> get_last_body()
		{
			return body_.back();
		}

	public:
		std::string name() const
		{
			return name_;
		}
		const std::vector<attribute>& attributes() const noexcept
		{
			return attributes_;
		}
		const std::vector<std::shared_ptr<element>>& body() const noexcept
		{
			return body_;
		}
		std::string body_str() const noexcept
		{
			return body_str_;
		}
		bool is_closing_tag() const noexcept
		{
			return is_closing_tag_;
		}
		std::weak_ptr<element> parent() const noexcept
		{
			return parent_;
		}
		void parent(const std::weak_ptr<element>& new_parent) noexcept
		{
			parent_ = new_parent;
		}
		
	private:
		std::string name_;
		std::vector<attribute> attributes_;
		std::vector<std::shared_ptr<element>> body_;
		std::string body_str_;
		bool is_closing_tag_ = false;

		std::weak_ptr<element> parent_;
	};

	template<typename AttributeData_>
	class element_creator : public element
	{
	protected:
		using attribute_data = AttributeData_;

	public:
		element_creator(const element_creator&) = delete;
		virtual ~element_creator() override = default;

	protected:
		explicit element_creator(const std::string& name)
			: element(name)
		{}
		element_creator(const std::string& name, const std::string& str)
			: element(name, str)
		{}

	public:
		element& operator=(const element&) = delete;
		template<typename FirstAttribute_, typename... OtherAttributes_>
		std::shared_ptr<element> operator()(FirstAttribute_&& first_attribute, OtherAttributes_&& ... other_attributes) const
		{
			static_assert(attribute_data_checker<attribute_data, FirstAttribute_, OtherAttributes_...>::is_usable);

			std::shared_ptr<element> result(std::make_shared<element>(*this));
			result->add_attribute(std::forward<FirstAttribute_>(first_attribute));
			return (*result)(std::forward<OtherAttributes_>(other_attributes)...);
		}
		template<typename Attribute_>
		std::shared_ptr<element> operator()(Attribute_&& attribute) const
		{
			static_assert(attribute_data_checker<attribute_data, Attribute_>::is_usable);

			std::shared_ptr<element> result(std::make_shared<element>(*this));
			result->add_attribute(std::forward<Attribute_>(attribute));
			return result;
		}
		std::shared_ptr<element>& operator()() const
		{
			return std::make_shared(*this);
		}
	};

	class dummy_element : public element_creator<attribute_data<>>
	{
	public:
		dummy_element()
			: element_creator("")
		{}
		dummy_element(const dummy_element&) = delete;
		virtual ~dummy_element() override = default;

	public:
		dummy_element& operator=(const dummy_element&) = delete;
	};

	class string_element : public element_creator<attribute_data<>>
	{
	public:
		string_element(const std::string& body_str)
			: element_creator("", body_str)
		{}
		string_element(const string_element&) = delete;
		virtual ~string_element() override = default;

	public:
		string_element& operator=(const string_element&) = delete;
	};

	const std::shared_ptr<element>& operator<(const dummy_element&, const std::shared_ptr<element>& element)
	{
		return element;
	}
	std::shared_ptr<element> operator<(const dummy_element&, const element& element)
	{
		return std::make_shared<details::element>(element);
	}
	const std::shared_ptr<element>& operator<(const std::shared_ptr<element>& element, const dummy_element&)
	{
		return element;
	}
	std::shared_ptr<element> operator<(const element& element, const dummy_element&)
	{
		return std::make_shared<details::element>(element);
	}
	std::shared_ptr<element> operator<(const std::shared_ptr<element>& element_a, const std::shared_ptr<element>& element_b)
	{
		if (element_b->is_closing_tag())
		{
			return element_a->parent().lock();
		}
		else
		{
			element_b->parent(element_a);
			return element_b;
		}
	}
	std::shared_ptr<element> operator<(const std::shared_ptr<element>& element_a, const element& element_b)
	{
		if (element_b.is_closing_tag())
		{
			return element_a->parent().lock();
		}
		else
		{
			std::shared_ptr<element> result(std::make_shared<element>(element_b));
			result->parent(element_a);
			element_a->add_body(result);
			return result;
		}
	}
	const std::shared_ptr<element>& operator>(const std::shared_ptr<element>& element, const dummy_element&)
	{
		return element;
	}
	std::shared_ptr<element> operator>(const std::shared_ptr<element>& element_a, const std::shared_ptr<element>& element_b)
	{
		if (element_b->is_closing_tag())
		{
			return element_a;
		}
		else
		{
			element_b->parent(element_a);
			element_a->add_body(element_b);
			return element_b;
		}
	}
	std::shared_ptr<element> operator>(const std::shared_ptr<element>& element_a, const element& element_b)
	{
		if (element_b.is_closing_tag())
		{
			return element_a;
		}
		else
		{
			std::shared_ptr<element> result(std::make_shared<element>(element_b));
			result->parent(element_a);
			element_a->add_body(result);
			return result;
		}
	}

	std::ostream& operator<<(std::ostream& stream, const element& element)
	{
		if (!element.name().empty())
		{
			stream << '<' << element.name();

			if (element.attributes().size())
			{
				for (const auto& attr : element.attributes())
				{
					stream << ' ' << attr.name() << "=\"" << attr.value() << '"';
				}
			}

			stream << '>';

			for (const auto& body : element.body())
			{
				stream << *body;
			}

			stream << "</" << element.name() << '>';
		}
		else
		{
			stream << element.body_str();
		}

		return stream;
	}
}

namespace cppreact
{
	static const details::dummy_element $;
	static const details::dummy_element _;

	details::element operator""_s(const char* string, std::size_t len)
	{
		return details::string_element(std::string(string, len));
	}
}

namespace cppreact::details
{
	class a_element : public element_creator<attribute_data<
		href_attribute
	>>
	{
	public:
		a_element()
			: element_creator("a")
		{}
		a_element(const a_element&) = delete;
		virtual ~a_element() override = default;

	public:
		a_element& operator=(const a_element&) = delete;
	};
}

namespace cppreact
{
	static const details::a_element a;
}