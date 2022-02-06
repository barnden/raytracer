#pragma once

#include <cassert>

#define ASSERT_NOT_REACHED assert(false);

template <typename T>
concept Numeric = std::convertible_to<T, std::size_t>;
