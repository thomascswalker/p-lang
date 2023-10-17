#pragma once

#include <string>
#include <iostream>

template <class Out, class In>
inline Out* Cast(In InValue)
{
	return dynamic_cast<Out*>(InValue);
}

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

inline std::string ReadFile(std::string FileName)
{
	std::ifstream Stream(FileName.c_str());
	return std::string(std::istreambuf_iterator<char>(Stream), std::istreambuf_iterator<char>());
};