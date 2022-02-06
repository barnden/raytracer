#pragma once

#include <cassert>
#include <cmath>
#include <concepts>
#include <cstddef>

#include <ostream>

#define ASSERT_NOT_REACHED assert(false);

template <typename T>
concept Numeric = std::convertible_to<T, std::size_t>;

template <size_t N, Numeric T>
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

    using Type = T;

    Vec()
        : x(0)
        , y(0)
        , z(0)
        , w(0) {};

    Vec(T a)
        : x(a)
        , y(a)
        , z(a)
        , w(a) {};

    Vec(T a, T b)
        : x(a)
        , y(b)
        , z(0)
        , w(0) {};

    Vec(T a, T b, T c)
        : x(a)
        , y(b)
        , z(c)
        , w(0) {};

    Vec(T a, T b, T c, T d)
        : x(a)
        , y(b)
        , z(c)
        , w(d) {};

    template <typename F>
    auto inline constexpr VectorOperation(F&& func, Vec<N, T> const& rhs)
    {
        func(x, rhs.x);

        if constexpr (N >= 2)
            func(y, rhs.y);

        if constexpr (N >= 3)
            func(z, rhs.z);

        if constexpr (N >= 4)
            func(w, rhs.w);
    }

    template <typename F, Numeric Scalar>
    auto inline constexpr VectorOperation(F&& func, Scalar const& rhs)
    {
        func(x, rhs);

        if constexpr (N >= 2)
            func(y, rhs);

        if constexpr (N >= 3)
            func(z, rhs);

        if constexpr (N >= 4)
            func(w, rhs);
    }

    auto inline operator*=(auto const& rhs)
    {
        VectorOperation([](T& a, T const& b) { a *= b; }, rhs);

        return *this;
    }

    __attribute__((flatten)) friend auto inline operator*(Vec<N, T> const& lhs, auto const& rhs)
    {
        auto result = lhs;

        result *= rhs;

        return result;
    }

    template <Numeric Scalar>
    __attribute__((flatten)) friend auto inline operator*(Scalar const& lhs, Vec<N, T> const& rhs)
    {
        return rhs * lhs;
    }

    auto inline operator/=(auto const& rhs)
    {
        VectorOperation([](T& a, T const& b) { a /= b; }, rhs);

        return *this;
    }

    __attribute__((flatten)) friend auto inline operator/(Vec<N, T> const& lhs, auto const& rhs)
    {
        auto result = lhs;

        result /= rhs;

        return result;
    }

    template <Numeric Scalar>
    __attribute__((flatten)) friend auto inline operator/(Scalar const& lhs, Vec<N, T> const& rhs)
    {
        auto result = Vec<N, T>(static_cast<T>(lhs));

        result /= rhs;

        return result;
    }

    auto inline operator+=(Vec<N, T> const& rhs)
    {
        x += rhs.x;

        if constexpr (N >= 2)
            y += rhs.y;

        if constexpr (N >= 3)
            z += rhs.z;

        if constexpr (N >= 4)
            w += rhs.w;

        return *this;
    }

    __attribute__((flatten)) friend auto inline operator+(auto const& lhs, auto const& rhs)
    {
        auto result = lhs;

        result += rhs;

        return result;
    }

    auto inline operator-=(Vec<N, T> const& rhs)
    {
        x -= rhs.x;

        if constexpr (N >= 2)
            y -= rhs.y;

        if constexpr (N >= 3)
            z -= rhs.z;

        if constexpr (N >= 4)
            w -= rhs.w;

        return *this;
    }

    __attribute__((flatten)) friend auto inline operator-(auto const& lhs, auto const& rhs)
    {
        auto result = lhs;

        result -= rhs;

        return result;
    }

    __attribute__((flatten)) friend T inline dot(Vec<N, T> a, Vec<N, T> b)
    {
        auto sum = a.x * b.x;

        if constexpr (N >= 2)
            sum += a.y * b.y;

        if constexpr (N >= 3)
            sum += a.z * b.z;

        if constexpr (N >= 4)
            sum += a.w * b.w;

        return sum;
    }

    template <typename U>
    __attribute__((flatten)) U inline magnitude() const
    {
        return static_cast<U>(std::sqrt(dot(*this, *this)));
    }

    __attribute__((flatten)) T inline magnitude() const { return magnitude<T>(); }

    __attribute__((flatten)) auto inline normalize()
    {
        *this /= magnitude<double>();

        return *this;
    }

    __attribute__((flatten)) friend auto inline normalize(Vec<N, T> const& vec)
    {
        auto result = vec;

        result.normalize();

        return result;
    }

    __attribute__((flatten)) friend std::ostream& operator<<(std::ostream& stream, Vec<N, T> const& vec)
    {
        stream << vec.x;

        if constexpr (N >= 2)
            stream << ", " << vec.y;

        if constexpr (N >= 3)
            stream << ", " << vec.z;

        if constexpr (N >= 4)
            stream << ", " << vec.w;

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
