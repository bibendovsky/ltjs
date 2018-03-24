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
// Calls specified callbacks on entering (optional) the scope and upon it's leaving.
//


#ifndef BIBENDOVSKY_SPUL_SCOPE_GUARD_INCLUDED
#define BIBENDOVSKY_SPUL_SCOPE_GUARD_INCLUDED


#include <functional>
#include <utility>


namespace bibendovsky
{
namespace spul
{


//
// Calls specified callbacks on entering (optional) the scope and upon it's leaving.
//
class ScopeGuard
{
public:
	using Callback = std::function<void()>;


	ScopeGuard() = delete;

	explicit ScopeGuard(
		const Callback& leave_callback)
		:
		leave_callback_{leave_callback}
	{
	}

	ScopeGuard(
		const Callback& enter_callback,
		const Callback& leave_callback)
		:
		ScopeGuard{leave_callback}
	{
		enter_callback();
	}

	ScopeGuard(
		const ScopeGuard& that) = delete;

	ScopeGuard& operator=(
		const ScopeGuard& that) = delete;

	ScopeGuard(
		ScopeGuard&& that)
		:
		leave_callback_{std::move(that.leave_callback_)}
	{
	}

	~ScopeGuard()
	{
		leave_callback_();
	}


private:
	Callback leave_callback_;
}; // ScopeGuard


} // spul
} // bibendovsky


#endif // !BIBENDOVSKY_SPUL_SCOPE_GUARD_INCLUDED
