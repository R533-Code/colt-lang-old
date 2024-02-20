/*****************************************************************//**
 * @file   exit_recursion.h
 * @brief  Contains an exception used for unwinding a recursion.
 * Throughout the compiler, recursion cannot be avoided.
 * To avoid stack overflow, an integer must be keep track of recursion
 * depth.
 * Using checked_inc, the integer can be incremented, but on overflow
 * a ExitRecursionExcept will be thrown.
 * See `consume_lines_comment_throw` in `colt_lexer.cpp`.
 * 
 * @author RPC
 * @date   February 2024
 *********************************************************************/
#ifndef HG_COLT_EXIT_RECURSION
#define HG_COLT_EXIT_RECURSION

#include <exception>
#include "util/types.h"

namespace clt
{
	/// @brief Used to unwind a recursion (to avoid stack overflow)
	class ExitRecursionExcept
		: public std::exception
	{
	public:
		ExitRecursionExcept()
			: std::exception("ExitRecursionExcept") {}
	};

	template<meta::Integral T>
	/// @brief Increments an integer and throw on overflow (ExitRecursionExcept)
	/// @param a The integer to increment
	void checked_inc(T& a)
	{
		if (a == std::numeric_limits<T>::max())
			throw ExitRecursionExcept{};
		++a;
	}
}

#endif // !HG_COLT_EXIT_RECURSION