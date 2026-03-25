/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2021-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// Circular queue

#ifndef LTJS_CIRCULAR_QUEUE_INCLUDED
#define LTJS_CIRCULAR_QUEUE_INCLUDED

#include "ltjs_exception.h"
#include <cassert>
#include <string_view>
#include <type_traits>
#include <vector>

namespace ltjs {

template<typename T>
class CircularQueueIterator
{
public:
	CircularQueueIterator(T* elements, int max_size, int offset) noexcept
		:
		elements_{elements},
		max_size_{max_size},
		offset_{offset}
	{
		assert(elements_ != nullptr);
		assert(max_size_ >= 0);
		assert(offset_ >= 0 && offset <= max_size_);
	}

	T& operator*() const
	{
		assert(elements_ != nullptr);
		return elements_[offset_];
	}

	void operator++() noexcept
	{
		assert(elements_ != nullptr);
		advance_offset();
	}

	bool operator==(const CircularQueueIterator& rhs) const noexcept
	{
		return
			elements_ == rhs.elements_ &&
			max_size_ == rhs.max_size_ &&
			offset_ == rhs.offset_;
	}

	bool operator!=(const CircularQueueIterator& rhs) const noexcept
	{
		return !((*this) == rhs);
	}

private:
	T* elements_{};
	int max_size_{};
	int offset_{};

	void advance_offset() noexcept
	{
		offset_ -= 1;
		if (offset_ < 0)
		{
			offset_ += max_size_;
		}
	}
};

// =====================================

template<typename T>
class CircularQueue
{
public:
	using Element = T;
	using Iterator = CircularQueueIterator<Element>;
	using CIterator = CircularQueueIterator<std::add_const_t<Element>>;

	void clear() noexcept
	{
		size_ = 0;
		pop_offset_ = 0;
		push_offset_ = 0;
	}

	int get_size() const noexcept
	{
		return size_;
	}

	int get_max_size() const noexcept
	{
		return static_cast<int>(elements_.size());
	}

	bool is_empty() const noexcept
	{
		return get_size() == 0;
	}

	void push(const Element& element)
	{
		if (get_size() == get_max_size())
		{
			fail("Full queue.");
		}
		elements_[push_offset_] = element;
		advance_offset(push_offset_);
		size_ += 1;
	}

	Element pop()
	{
		if (is_empty())
		{
			fail("Empty queue.");
		}
		size_ -= 1;
		const auto& element = elements_[pop_offset_];
		advance_offset(pop_offset_);
		return element;
	}

	void set_max_size(int max_size)
	{
		if (max_size < 0)
		{
			fail("Max size out of range.");
		}
		if (!is_empty())
		{
			fail("Non-empty queue.");
		}
		clear();
		elements_.resize(static_cast<std::size_t>(max_size));
	}

	Iterator begin()
	{
		return Iterator{elements_.data(), get_max_size(), pop_offset_};
	}

	CIterator begin() const
	{
		return CIterator{elements_.data(), get_max_size(), pop_offset_};
	}

	Iterator end()
	{
		return Iterator{elements_.data(), get_max_size(), push_offset_};
	}

	CIterator end() const
	{
		return CIterator{elements_.data(), get_max_size(), push_offset_};
	}

private:
	using Elements = std::vector<Element>;

	Elements elements_{};
	int size_{};
	int pop_offset_{};
	int push_offset_{};

	[[noreturn]] static void fail(std::string_view message)
	{
		throw Exception{"LTJS_CIRCULAR_QUEUE", message};
	}

	void advance_offset(int& offset) const noexcept
	{
		offset -= 1;
		if (offset < 0)
		{
			offset += get_max_size();
		}
	}
};

} // namespace ltjs

#endif // LTJS_CIRCULAR_QUEUE_INCLUDED
