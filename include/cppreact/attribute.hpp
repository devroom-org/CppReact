#pragma once

#include <string>
#include <type_traits>
#include <utility>

namespace cppreact::details
{
	template<typename... Attributes_>
	class attribute_data;
	template<typename FirstAttribute_, typename... OtherAttributes_>
	class attribute_data<FirstAttribute_, OtherAttributes_...>
	{
	public:
		template<typename Attribute_>
		static constexpr bool is_usable = std::is_same_v<FirstAttribute_, Attribute_> || attribute_data<OtherAttributes_...>::template is_usable<Attribute_>;
	};
	template<>
	class attribute_data<>
	{
	public:
		template<typename>
		static constexpr bool is_usable = false;
	};

	template<typename AttributeData_, typename... Attributes_>
	class attribute_data_checker;
	template<typename AttributeData_, typename FirstAttribute_, typename... OtherAttributes_>
	class attribute_data_checker<AttributeData_, FirstAttribute_, OtherAttributes_...>
	{
	public:
		static constexpr bool is_usable = AttributeData_::template is_usable<std::remove_reference_t<FirstAttribute_>> && attribute_data_checker<AttributeData_, OtherAttributes_...>::is_usable;
	};
	template<typename AttributeData_>
	class attribute_data_checker<AttributeData_>
	{
	public:
		static constexpr bool is_usable = true;
	};

	class attribute
	{
	public:
		explicit attribute(const std::string& name)
			: name_(name)
		{}
		attribute(const std::string& name, const std::string& value)
			: name_(name), value_(value)
		{}
		attribute(const attribute& attribute)
			: name_(attribute.name_), value_(attribute.value_)
		{}
		attribute(attribute&& attribute) noexcept
			: name_(std::move(attribute.name_)), value_(std::move(attribute.value_))
		{}
		virtual ~attribute() = default;
		
	public:
		attribute& operator=(const attribute&) = delete;

	public:
		std::string name() const
		{
			return name_;
		}
		std::string value() const
		{
			return value_;
		}
		void value(const std::string& new_value)
		{
			value_ = new_value;
		}
		void value(std::string&& new_value) noexcept
		{
			value_ = std::move(new_value);
		}

	private:
		std::string name_;
		std::string value_;
	};

	template<typename Attribute_>
	class attribute_creator : public attribute
	{
	public:
		explicit attribute_creator(const std::string& name)
			: attribute(name)
		{}
		attribute_creator(const std::string& name, const std::string& value)
			: attribute(name, value)
		{}
		attribute_creator(const attribute_creator& attribute) = default;
		attribute_creator(attribute_creator&& attribute) noexcept = default;
		virtual ~attribute_creator() override = default;

	public:
		Attribute_ operator=(const std::string& value) const
		{
			Attribute_ result(static_cast<const Attribute_&>(*this));
			result.value(value);
			return result;
		}
		Attribute_ operator=(std::string&& value) const
		{
			Attribute_ result(static_cast<const Attribute_&>(*this));
			result.value(std::move(value));
			return result;
		}
		attribute& operator=(const attribute_creator&) = delete;
	};
}

namespace cppreact::details
{
	class href_attribute : public attribute_creator<href_attribute>
	{
	public:
		href_attribute()
			: attribute_creator("href")
		{}
		explicit href_attribute(const std::string& value)
			: attribute_creator("href", value)
		{}
		href_attribute(const href_attribute& attribute) = default;
		href_attribute(href_attribute&& attribute) noexcept = default;
		virtual ~href_attribute() override = default;

	public:
		using attribute_creator<href_attribute>::operator=;
		href_attribute& operator=(const href_attribute&) = delete;
	};
}

namespace cppreact
{
	static const details::href_attribute href;
}