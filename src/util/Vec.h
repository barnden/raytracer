#pragma once

#include <cmath>
#include <concepts>
#include <cstddef>

#include <ostream>
#include <utility>

#include "Concepts.h"

template <std::size_t N, Numeric T>
class Vec {
public:
    T x;
    T y;
    T z;
    T w;

    T& r = x;
    T& g = y;
    T& b = z;
    T& a = w;

    T* refs[4] = { &x, &y, &z, &w };

    Vec(T a, T b, T c, T d)
        : x(a)
        , y(b)
        , z(c)
        , w(d) {};

    Vec()
        : Vec(0, 0, 0, 0) {};

    Vec(T a, T b, T c)
        : Vec(a, b, c, 0) {};

    Vec(T a, T b)
        : Vec(a, b, 0, 0) {};

    Vec(T a)
        : Vec(a, a, a, a) {};

    Vec(Vec const& rhs)
        : Vec(rhs.x, rhs.y, rhs.z, rhs.w) {};

    auto inline& at(auto idx) const
    {
        if (idx >= N)
            throw "Index exceeds vector contents";

        return *refs[idx];
    }

    auto inline& operator[](auto idx) { return at(idx); }
    auto inline const& operator[](auto idx) const { return std::as_const(at(idx)); }

    template <typename F>
    auto inline constexpr for_each(F&& func)
    {
        [&]<std::size_t... I>(std::index_sequence<I...>)
        {
            (func(at(I), I), ...);
        }
        (std::make_index_sequence<N> {});
    }

    template <typename F>
    auto inline constexpr for_each_const(F&& func) const
    {
        [&]<std::size_t... I>(std::index_sequence<I...>)
        {
            (func(std::as_const(at(I)), I), ...);
        }
        (std::make_index_sequence<N> {});
    }

    auto inline operator=(auto const& rhs)
    {
        for_each([&rhs](T& a, auto idx) { a = rhs[idx]; });

        return *this;
    }

    auto inline operator*=(auto const& rhs)
    {
        for_each([&rhs](T& a, auto idx) {
            if constexpr (is_same_cv<decltype(rhs), Vec<N, T>>::value)
                a *= rhs[idx];
            else
                a *= rhs;
        });

        return *this;
    }

    friend auto inline operator*(Vec<N, T> const& lhs, auto const& rhs)
    {
        auto result = lhs;

        result *= rhs;

        return result;
    }

    template <Numeric Scalar>
    friend auto inline operator*(Scalar const& lhs, Vec<N, T> const& rhs)
    {
        return rhs * lhs;
    }

    auto inline operator/=(auto const& rhs)
    {
        for_each([&rhs](T& a, auto idx) {
            if constexpr (is_same_cv<decltype(rhs), Vec<N, T>>::value)
                a /= rhs[idx];
            else
                a /= rhs;
        });

        return *this;
    }

    friend auto inline operator/(Vec<N, T> const& lhs, auto const& rhs)
    {
        auto result = lhs;

        result /= rhs;

        return result;
    }

    template <Numeric Scalar>
    friend auto inline operator/(Scalar const& lhs, Vec<N, T> const& rhs)
    {
        auto result = Vec<N, T>(static_cast<T>(lhs));

        result /= rhs;

        return result;
    }

    auto inline operator+=(Vec<N, T> const& rhs)
    {
        for_each([&rhs](T& a, auto idx) { a += rhs[idx]; });

        return *this;
    }

    friend auto inline operator+(auto const& lhs, auto const& rhs)
    {
        auto result = lhs;

        result += rhs;

        return result;
    }

    auto inline operator-=(Vec<N, T> const& rhs)
    {
        for_each([&rhs](T& a, auto idx) { a -= rhs[idx]; });

        return *this;
    }

    friend auto inline operator-(auto const& lhs, auto const& rhs)
    {
        auto result = lhs;

        result -= rhs;

        return result;
    }

    __attribute__((flatten)) friend T inline dot(Vec<N, T> const& a, Vec<N, T> const& b)
    {
        auto sum = T{};

        a.for_each_const([&](T const& val, auto idx) { sum += val * b[idx]; });

        return sum;
    }

    template <typename U>
    U inline magnitude() const
    {
        return static_cast<U>(std::sqrt(dot(*this, *this)));
    }

    T inline magnitude() const { return magnitude<T>(); }

    auto inline normalize()
    {
        *this /= magnitude<double>();

        return *this;
    }

    friend auto inline normalize(Vec<N, T> const& vec)
    {
        auto result = vec;

        result.normalize();

        return result;
    }

    __attribute__((flatten)) friend std::ostream& operator<<(std::ostream& stream, Vec<N, T> const& vec)
    {
        vec.for_each_const([&](T const& val, auto idx) {
            if (idx > 0)
                stream << ", ";

            stream << val;
        });

        return stream;
    }
};

template <typename T>
__attribute__((flatten)) Vec<3, T> inline cross(Vec<3, T> a, Vec<3, T> b)
{
    return { a.y * b.z - a.z * b.y,
             a.z * b.x - a.x * b.z,
             a.x * b.y - a.y * b.x };
}

template <Numeric T>
using Vec2 = Vec<2, T>;

template <Numeric T>
using Vec3 = Vec<3, T>;

template <Numeric T>
using Vec4 = Vec<4, T>;
