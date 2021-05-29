#include "ltjs_char_conv.h"

#include <cassert>

#include <type_traits>


namespace ltjs
{


namespace detail
{


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

template<
	typename T
>
struct ToCharsIntegerTraits;

template<>
struct ToCharsIntegerTraits<std::int32_t>
{
	static constexpr auto is_signed = true;
	static constexpr auto max_buffer_size = 11;
}; // ToCharsIntegerTraits

template<>
struct ToCharsIntegerTraits<std::uint32_t>
{
	static constexpr auto is_signed = false;
	static constexpr auto max_buffer_size = 10;
}; // ToCharsIntegerTraits

template<>
struct ToCharsIntegerTraits<std::uint64_t>
{
	static constexpr auto is_signed = false;
	static constexpr auto max_buffer_size = 20;
}; // ToCharsIntegerTraits


template<
	typename T
>
struct ToCharsNopDigitFunctor
{
	static constexpr auto is_negative = false;

	constexpr T operator()(
		T value) const noexcept
	{
		return value;
	}
}; // ToCharsNopDigitFunctor


template<
	typename T
>
struct ToCharsNegDigitFunctor
{
	static constexpr auto is_negative = std::is_signed<T>::value;

	constexpr T operator()(
		T value) const noexcept
	{
		return -value;
	}
}; // ToCharsNegDigitFunctor


template<
	typename T,
	typename TFunctor,
	int TBase
>
Index to_chars(
	T value,
	const ToCharsFormat& format,
	char* dst_buffer,
	Index dst_buffer_size) noexcept
{
	constexpr auto lowercase_digits = "0123456789abcdef";
	constexpr auto uppercase_digits = "0123456789ABCDEF";

	const auto& digits = ((format & to_chars_format_uppercase) != 0 ? uppercase_digits : lowercase_digits);
	const auto digit_functor = TFunctor{};

	char buffer[ToCharsIntegerTraits<T>::max_buffer_size];

	auto size = Index{};

	do
	{
		if (size >= dst_buffer_size)
		{
			return 0;
		}

		buffer[size++] = digits[digit_functor(value % TBase)];
		value /= TBase;
	} while (value != 0);

	if (size >= dst_buffer_size)
	{
		return 0;
	}

	if (digit_functor.is_negative)
	{
		buffer[size++] = '-';
	}

	for (auto i = Index{}; i < size; ++i)
	{
		dst_buffer[i] = buffer[size - i - 1];
	}

	return size;
}

template<
	typename T,
	typename TFunctor
>
Index to_chars(
	T value,
	int base,
	const ToCharsFormat& format,
	char* dst_buffer,
	Index dst_buffer_size) noexcept
{
	switch (base)
	{
		case 10:
			return to_chars<T, TFunctor, 10>(value, format, dst_buffer, dst_buffer_size);

		case 16:
			return to_chars<T, TFunctor, 16>(value, format, dst_buffer, dst_buffer_size);

		default:
			return 0;
	}
}

struct ToCharsSignedTag{};
struct ToCharsUnsignedTag{};

template<
	typename T
>
Index to_chars(
	T value,
	int base,
	const ToCharsFormat& format,
	char* dst_buffer,
	Index dst_buffer_size,
	ToCharsSignedTag) noexcept
{
	if (value < 0)
	{
		return to_chars<T, ToCharsNegDigitFunctor<T>>(value, base, format, dst_buffer, dst_buffer_size);
	}
	else
	{
		return to_chars<T, ToCharsNopDigitFunctor<T>>(value, base, format, dst_buffer, dst_buffer_size);
	}
}

template<
	typename T
>
Index to_chars(
	T value,
	int base,
	const ToCharsFormat& format,
	char* dst_buffer,
	Index dst_buffer_size,
	ToCharsUnsignedTag) noexcept
{
	return to_chars<T, ToCharsNopDigitFunctor<T>>(value, base, format, dst_buffer, dst_buffer_size);
}

template<
	typename T
>
Index to_chars(
	T value,
	int base,
	const ToCharsFormat& format,
	char* dst_buffer,
	Index dst_buffer_size) noexcept
{
	assert(dst_buffer);
	assert(dst_buffer_size >= 0);

	using Tag = std::conditional_t<
		ToCharsIntegerTraits<T>::is_signed,
		ToCharsSignedTag,
		ToCharsUnsignedTag
	>;

	return to_chars<T>(value, base, format, dst_buffer, dst_buffer_size, Tag{});
}


} // detail


Index to_chars(
	std::int32_t value,
	int base,
	const ToCharsFormat& format,
	char* dst_buffer,
	Index dst_buffer_size) noexcept
{
	return detail::to_chars<std::int32_t>(value, base, format, dst_buffer, dst_buffer_size);
}

Index to_chars(
	std::uint32_t value,
	int base,
	const ToCharsFormat& format,
	char* dst_buffer,
	Index dst_buffer_size) noexcept
{
	return detail::to_chars<std::uint32_t>(value, base, format, dst_buffer, dst_buffer_size);
}

Index to_chars(
	std::uint64_t value,
	int base,
	const ToCharsFormat& format,
	char* dst_buffer,
	Index dst_buffer_size) noexcept
{
	return detail::to_chars<std::uint64_t>(value, base, format, dst_buffer, dst_buffer_size);
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

namespace detail
{


template<
	typename T
>
struct FromCharsIntegerTraits;

template<>
struct FromCharsIntegerTraits<std::int32_t>
{
	static constexpr auto is_signed = true;
	static constexpr auto max_hex_digits = 8;
}; // FromCharsIntegerTraits

template<>
struct FromCharsIntegerTraits<std::uint32_t>
{
	static constexpr auto is_signed = false;
	static constexpr auto max_hex_digits = 8;
}; // FromCharsIntegerTraits

template<>
struct FromCharsIntegerTraits<std::uint64_t>
{
	static constexpr auto is_signed = false;
	static constexpr auto max_hex_digits = 16;
}; // FromCharsIntegerTraits


template<
	typename T
>
bool from_chars_hex(
	const char* string,
	Index string_size,
	T& value) noexcept
{
	constexpr auto integer_traits = FromCharsIntegerTraits<T>{};

	if (string_size <= 0 || string_size > FromCharsIntegerTraits<T>::max_hex_digits)
	{
		return false;
	}

	value = 0;

	auto should_check_for_overflow = (
		FromCharsIntegerTraits<T>::is_signed &&
		string_size == FromCharsIntegerTraits<T>::max_hex_digits
	);

	for (auto i = Index{}; i < string_size; ++i)
	{
		const auto ch = string[i];

		auto digit = T{};

		if (ch >= '0' && ch <= '9')
		{
			digit = ch - '0';
		}
		else if (ch >= 'a' && ch <= 'f')
		{
			digit = 0xA + ch - 'a';
		}
		else if (ch >= 'A' && ch <= 'F')
		{
			digit = 0xA + ch - 'A';
		}
		else
		{
			return false;
		}

		if (should_check_for_overflow)
		{
			if (digit >= 8)
			{
				// Overflow.
				return false;
			}

			should_check_for_overflow = false;
		}

		value *= 16;
		value += digit;
	}

	return true;
};

template<
	typename T
>
bool from_chars(
	const char* string,
	Index string_size,
	int base,
	T& value) noexcept
{
	assert(string);

	switch (base)
	{
		case 16:
			return detail::from_chars_hex<T>(string, string_size, value);

		default:
			return false;
	}
};


} // detail


bool from_chars(
	const char* string,
	Index string_size,
	int base,
	std::int32_t& value) noexcept
{
	return detail::from_chars<std::int32_t>(string, string_size, base, value);
};

bool from_chars(
	const char* string,
	Index string_size,
	int base,
	std::uint32_t& value) noexcept
{
	return detail::from_chars<std::uint32_t>(string, string_size, base, value);
};

bool from_chars(
	const char* string,
	Index string_size,
	int base,
	std::uint64_t& value) noexcept
{
	return detail::from_chars<std::uint64_t>(string, string_size, base, value);
};

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


} // ltjs
