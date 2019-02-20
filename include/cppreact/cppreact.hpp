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
		cppreact(const cppreact&) = delete;
		~cppreact()
		{
			root_element_manager::erase(element_);
		}

	public:
		cppreact& operator=(const cppreact&) = delete;
		const element& operator*() const noexcept
		{
			return *element_;
		}

	private:
		std::shared_ptr<element> element_;
	};
}

#define cppreact(expr) *cppreact::details::cppreact($ expr $)