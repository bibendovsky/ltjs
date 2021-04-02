#ifndef LTJS_CIRCULAR_QUEUE_INCLUDED
#define LTJS_CIRCULAR_QUEUE_INCLUDED


#include <cassert>

#include <algorithm>
#include <type_traits>
#include <vector>

#include "ltjs_exception.h"
#include "ltjs_index_type.h"


namespace ltjs
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class RingBufferException :
	public Exception
{
public:
	explicit RingBufferException(
		const char* message)
		:
		Exception{"LTJS_CIRCULAR_QUEUE", message}
	{
	}
}; // RingBufferException

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

template<
	typename T
>
class CircularQueueIterator
{
public:
	CircularQueueIterator(
		T* elements,
		Index max_size,
		Index offset) noexcept
		:
		elements_{elements},
		max_size_{max_size},
		offset_{offset}
	{
		assert(elements_);
		assert(max_size_ >= 0);
		assert(offset_ >= 0 && offset <= max_size_);
	}

	T& operator*() const
	{
		assert(elements_);

		return elements_[offset_];
	}

	void operator++() noexcept
	{
		assert(elements_);

		advance_offset();
	}

	bool operator==(
		const CircularQueueIterator& rhs) const noexcept
	{
		return
			elements_ == rhs.elements_ &&
			max_size_ == rhs.max_size_ &&
			offset_ == rhs.offset_
		;
	}

	bool operator!=(
		const CircularQueueIterator& rhs) const noexcept
	{
		return !((*this) == rhs);
	}


private:
	T* elements_{};
	Index max_size_{};
	Index offset_{};


	void advance_offset()
	{
		offset_ -= 1;

		if (offset_ < 0)
		{
			offset_ += max_size_;
		}
	}
}; // CircularQueueIterator

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

template<
	typename T
>
class CircularQueue
{
public:
	using Element = T;
	using Iterator = CircularQueueIterator<T>;
	using CIterator = CircularQueueIterator<std::add_const_t<T>>;


	void clear()
	{
		size_ = 0;
		pop_offset_ = 0;
		push_offset_ = 0;
	}

	Index get_size() const noexcept
	{
		return size_;
	}

	Index get_max_size() const noexcept
	{
		return static_cast<Index>(elements_.size());
	}

	bool is_empty() const noexcept
	{
		return get_size() == 0;
	}

	void push(
		const Element& element)
	{
		if (get_size() == get_max_size())
		{
			throw RingBufferException{"Full queue."};
		}

		elements_[push_offset_] = element;
		advance_offset(push_offset_);
		size_ += 1;
	}

	Element pop()
	{
		if (is_empty())
		{
			throw RingBufferException{"Empty queue."};
		}

		size_ -= 1;

		const auto& element = elements_[pop_offset_];
		advance_offset(pop_offset_);
		return element;
	}

	void set_max_size(
		Index max_size)
	{
		if (max_size < 0)
		{
			throw RingBufferException{"Max size out of range."};
		}

		if (!is_empty())
		{
			throw RingBufferException{"Non-empty queue."};
		}

		clear();

		elements_.resize(max_size);
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
	Index size_{};
	Index pop_offset_{};
	Index push_offset_{};


	void advance_offset(
		Index& offset) const noexcept
	{
		offset -= 1;

		if (offset < 0)
		{
			offset += get_max_size();
		}
	}
}; // CircularQueue

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // ltjs


#endif // !LTJS_CIRCULAR_QUEUE_INCLUDED
