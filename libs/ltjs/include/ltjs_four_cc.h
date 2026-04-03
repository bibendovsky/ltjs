/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// FourCC (four-character code)

#ifndef LTJS_FOUR_CC_INCLUDED
#define LTJS_FOUR_CC_INCLUDED

#include <cassert>
#include <cstdint>

namespace ltjs {

class FourCc
{
public:
	constexpr FourCc() noexcept = default;
	constexpr FourCc(char char_1, char char_2, char char_3, char char_4) noexcept
		:
		chars_{char_1, char_2, char_3, char_4}
	{}

	constexpr const char* get_chars() const noexcept
	{
		return chars_;
	}

	constexpr const char& operator[](int index) const noexcept
	{
		assert(index >= 0 && index < 4);
		return chars_[index];
	}

	static constexpr bool from_octets_n(const void* data, int data_size, FourCc& four_cc) noexcept
	{
		if (data == nullptr)
		{
			assert(false && "Null data.");
			return false;
		}
		if (data_size < 4)
		{
			assert(false && "Not enough data.");
			return false;
		}
		const char* const octets = static_cast<const char*>(data);
		four_cc = FourCc{octets[0], octets[1], octets[2], octets[3]};
		return true;
	}

private:
	using Chars = char[4];

	Chars chars_{};
};

inline constexpr bool operator==(const FourCc& a, const FourCc& b) noexcept
{
	return a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && a[3] == b[3];
}

inline constexpr bool operator!=(const FourCc& a, const FourCc& b) noexcept
{
	return !(a == b);
}

} // namespace ltjs

#endif // LTJS_FOUR_CC_INCLUDED
