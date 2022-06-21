#pragma once
#pragma warning(disable:26812)

#include <iostream>
#include <string>
#include <exception>
#include <vector>
#include <algorithm>
#include <cstdarg>
#include <functional>
#include <map>

#define OPEN_NAMESPACE(n) namespace n {
#define CLOSE_NAMESPACE }

template<typename ... Args>
std::string strf(const std::string& format, Args ... args)
{
    int size_s = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1;
    if (size_s <= 0) { throw std::runtime_error("Error during formatting."); }
    auto size = static_cast<size_t>(size_s);
    std::unique_ptr<char[]> buf(new char[size]);
    std::snprintf(buf.get(), size, format.c_str(), args ...);
    return std::string(buf.get(), buf.get() + size - 1);
}

template <typename T>
struct GVec2 {
    T x;
    T y;

    inline bool operator==(const GVec2& v) const { return (x == v.x) && (y == v.y); }
    inline bool operator!=(const GVec2& v) const { return !(*this == v); }
};

static GVec2<int> invalid = { -1, -1 };