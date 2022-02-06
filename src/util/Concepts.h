#pragma once

#include <cassert>
#include <type_traits>

#define ASSERT_NOT_REACHED assert(false);

template <typename T>
concept Numeric = std::convertible_to<T, std::size_t>;

template <typename T, typename U>
struct is_same_cv : std::is_same<typename std::decay<T>::type, typename std::decay<U>::type>::type {
};
