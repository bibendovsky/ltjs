/*

Source Port Utility Library

Copyright (c) 2018 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
OR OTHER DEALINGS IN THE SOFTWARE.

*/


//
// A replacement of "enum class" for flags.
//
//
// Example:
//
//    struct MyFlags : public EnumFlags
//    {
//        MyFlags(const Value flags = none) :
//            EnumFlags{flags}
//        {
//        }
//
//        enum : Value
//        {
//            my_flag_01 = 1 << 0,
//            my_flag_02 = 1 << 1,
//        }; // enum
//    }; // MyFlags
//
//
//    auto flags = MyFlags{};
//    flags |= MyFlags::my_flag_01;
//    flags.set(MyFlags::my_flag_01 | MyFlags::my_flag_02);
//


#ifndef BIBENDOVSKY_SPUL_ENUM_FLAGS_INCLUDED
#define BIBENDOVSKY_SPUL_ENUM_FLAGS_INCLUDED


namespace bibendovsky
{
namespace spul
{


//
// A replacement of "enum class" for flags.
//
template<typename T = unsigned int>
class EnumFlagsT
{
public:
	using Value = T;


	// The default value (zero).
	static constexpr auto none = Value{};


	EnumFlagsT(
		const Value flags = {})
		:
		value_{flags}
	{
	}

	operator Value&()
	{
		return value_;
	}

	operator Value() const
	{
		return value_;
	}

	bool has_any(
		const Value flags) const
	{
		return (value_ & flags) != 0;
	}

	bool has_all(
		const Value flags) const
	{
		return (value_ & flags) == flags;
	}

	void set(
		const Value flags)
	{
		value_ |= flags;
	}

	void unset(
		const Value flags)
	{
		value_ &= ~flags;
	}

	void flip(
		const Value flags)
	{
		value_ ^= flags;
	}

	void clear()
	{
		value_ = none;
	}


private:
	Value value_;
}; // EnumFlagsT


using EnumFlags = EnumFlagsT<>;


} // spul
} // bibendovsky


#endif // !BIBENDOVSKY_SPUL_ENUM_FLAGS_INCLUDED
