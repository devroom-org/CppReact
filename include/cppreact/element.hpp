#pragma once

#include <cppreact/attribute.hpp>

#include <utility>
#include <vector>

namespace cppreact::details
{
	class dummy_element;

	class element_base
	{
	public:
		element_base(const element_base& element)
			: attributes_(element.attributes_), body_(element.body_), body_str_(element.body_str_), is_closing_tag_(element.is_closing_tag_)
		{}
		element_base(element_base&& element) noexcept
			: attributes_(std::move(element.attributes_)), body_(std::move(element.body_)), body_str_(std::move(element.body_str_)), is_closing_tag_(element.is_closing_tag_)
		{}
		virtual ~element_base() = default;

	protected:
		element_base() noexcept = default;
		element_base(const std::string& str)
			: body_str_(str)
		{}
		
	public:
		element_base& operator=(const element_base&) = delete;
		template<typename FirstAttribute_, typename... OtherAttributes_>
		element_base operator()(FirstAttribute_&& first_attribute, OtherAttributes_&&... other_attributes) const
		{
			element_base result(*this);
			result.add_attribute(std::forward<FirstAttribute_>(first_attribute));
			return result(std::forward<OtherAttributes_>(other_attributes)...);
		}
		template<typename Attribute_>
		element_base operator()(Attribute_&& attribute) const
		{
			element_base result(*this);
			result.add_attribute(std::forward<Attribute_>(attribute));
			return result;
		}
		element_base operator()() const
		{
			return *this;
		}
		element_base operator!() const
		{
			element_base result(*this);
			result.is_closing_tag_ = true;
			return result;
		}

	public:
		template<typename Element_>
		element_base& add_body(Element_&& element)
		{
			body_.push_back(std::forward<Element_>(element));
			return body_.back();
		}
		template<typename Attribute_>
		void add_attribute(Attribute_&& attribute)
		{
			attributes_.push_back(std::forward<Attribute_>(attribute));
		}

	public:
		bool is_closing_tag() const noexcept
		{
			return is_closing_tag_;
		}
		
	private:
		std::vector<attribute> attributes_;
		std::vector<element_base> body_;
		std::string body_str_;
		bool is_closing_tag_ = false;
	};

	template<typename AttributeData_>
	class element : public element_base
	{
	protected:
		using attribute_data = AttributeData_;

	public:
		element(const element& element)
			: element_base(element)
		{}
		element(element&& element) noexcept
			: element_base(std::move(element))
		{}
		virtual ~element() override = default;

	protected:
		element() noexcept = default;
		using element_base::element_base;

	public:
		element& operator=(const element&) = delete;
	};

	class dummy_element : public element<attribute_data<>>
	{
	public:
		dummy_element() noexcept = default;
		dummy_element(const dummy_element&) = delete;
		virtual ~dummy_element() override = default;

	public:
		dummy_element& operator=(const dummy_element&) = delete;
	};

	class string_element : public element<attribute_data<>>
	{
	public:
		string_element(const std::string& str)
			: element(str)
		{}
		string_element(const string_element&) = delete;
		virtual ~string_element() override = default;

	public:
		string_element& operator=(const string_element&) = delete;
	};

	const element_base& operator<(const dummy_element&, const element_base& element)
	{
		return element;
	}
	element_base operator<(const element_base& element_a, const element_base& element_b)
	{
		if (element_b.is_closing_tag())
		{
			return element_a;
		}
		else
		{
			element_base result(element_a);
			result.add_body(element_b);
			return result;
		}
	}
	const element_base& operator>(const dummy_element&, const element_base& element)
	{
		return element;
	}
	const element_base& operator>(const element_base& element, const dummy_element&)
	{
		return element;
	}
	element_base operator>(const element_base& element_a, const element_base& element_b)
	{
		if (element_b.is_closing_tag())
		{
			return element_a;
		}
		else
		{
			element_base result(element_a);
			result.add_body(element_b);
			return result;
		}
	}
}

namespace cppreact
{
	static const details::dummy_element $;
	static const details::dummy_element _;

	details::element_base operator""_s(const char* string, std::size_t len)
	{
		return details::string_element(std::string(string, len));
	}
}

namespace cppreact::details
{
	class a_element : public element<attribute_data<
		href_attribute
	>>
	{
	public:
		a_element() noexcept = default;
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