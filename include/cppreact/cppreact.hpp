#pragma once

#include <cppreact/attribute.hpp>
#include <cppreact/element.hpp>

#include <memory>
#include <utility>

namespace cppreact::details
{
	class cppreact
	{
	public:
		explicit cppreact(const std::shared_ptr<element>& element)
			: element_(element)
		{}
		explicit cppreact(std::shared_ptr<element>&& element)
			: element_(std::move(element))
		{}
		cppreact(const cppreact& element) noexcept
			: element_(element.element_)
		{}
		cppreact(cppreact&& element) noexcept
			: element_(std::move(element.element_))
		{}
		~cppreact()
		{
			if (element_.use_count() == 2)
			{
				root_element_manager::erase(element_);
			}
		}

	public:
		cppreact& operator=(const cppreact& element) noexcept
		{
			element_ = element.element_;
			return *this;
		}
		cppreact& operator=(cppreact&& element) noexcept
		{
			element_ = std::move(element.element_);
			return *this;
		}
		const element& operator*() const noexcept
		{
			return *element_;
		}
		operator element() const
		{
			return *element_;
		}

	private:
		std::shared_ptr<element> element_;
	};
}

#define cppreact(expr) (cppreact::details::cppreact($ expr $))