#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "logging.h"

using namespace Logging;

namespace Core
{

	static int		   Depth = 0;
	static std::string GetIndent()
	{
		std::string Indent;
		int			I = 0;
		while (I < Depth)
		{
			Indent += "  ";
			I++;
		}
		return Indent;
	}
	static int WHILE_MAX_LOOP = 100;

#ifdef _DEBUG
	#define DEBUG_ENTER                                    \
		Debug("{}Entering {}.", GetIndent(), __FUNCSIG__); \
		Depth++;
	#define DEBUG_EXIT \
		Depth--;       \
		Debug("{}Exiting {}.", GetIndent(), __FUNCSIG__);
#else
	#define DEBUG_ENTER
	#define DEBUG_EXIT
#endif

#define CHECK_ERRORS         \
	if (Logger::GetInstance()->GetCount(LogLevel::_Error) > 0) \
	{                        \
		DEBUG_EXIT           \
		return false;        \
	}

	/// <summary>
	/// Cast the specified <paramref name="In"/> type to the specified <paramref name="Out"/> type.
	/// </summary>
	/// <typeparam name="In">The input variable type.</typeparam>
	/// <typeparam name="Out">The output variable type.</typeparam>
	/// <param name="InValue">The input variable.</param>
	/// <returns>The casted variable.</returns>
	template <class Out, class In>
	constexpr Out* Cast(In InValue)
	{
		return dynamic_cast<Out*>(InValue);
	}

	/// <summary>
	/// Returns whether the specified Value is within the specified Array.
	/// </summary>
	/// <typeparam name="T">The type of the check value and the check array.</typeparam>
	/// <param name="Array">The array to check.</param>
	/// <param name="Value">The value to check is inside the array.</param>
	/// <returns>Whether the value is inside the array.</returns>
	template <typename T>
	bool Contains(const std::vector<T> Array, T Value)
	{
		for (auto A : Array)
		{
			if (A == Value)
			{
				return true;
			}
		}
		return false;
	}

	/// <summary>
	/// Iterator which returns the Object and its Index in the vector. Similar to Python's
	/// 'enumerate()' method.
	/// </summary>
	/// <example>
	/// The below is an example of how to use the Enumerate method.
	/// <code>
	/// for (const auto&amp; [O, I] : Enumerate(Objects))
	/// {
	///		...
	/// }
	/// </code>
	/// </example>
	template <typename T, typename TIter = decltype(std::begin(std::declval<T>())),
			  typename = decltype(std::end(std::declval<T>()))>
	constexpr auto Enumerate(T&& Iterable)
	{
		struct Iterator
		{
			size_t Index;
			TIter  Iter;
			bool   operator!=(const Iterator& Other) const { return Iter != Other.Iter; }
			void   operator++()
			{
				++Index;
				++Iter;
			}
			auto operator*() const { return std::tie(Index, *Iter); }
		};
		struct IterableWrapper
		{
			T	 Iterable;
			auto begin() { return Iterator{ 0, std::begin(Iterable) }; }
			auto end() { return Iterator{ 0, std::end(Iterable) }; }
		};
		return IterableWrapper{ std::forward<T>(Iterable) };
	}

	/// <summary>
	/// Reads the specified <paramref name="FileName"/> into a <param>std::string</param>.
	/// </summary>
	/// <param name="FileName">The file to read.</param>
	/// ///<returns>The contents of the file.</returns>
	std::string ReadFile(const std::string& FileName);
} // namespace Core