#pragma once

#include "Vec.h"

class Ray {
private:
    Vec3<double> m_origin;
    Vec3<double> m_direction;

public:
    Ray(auto origin, auto direction)
        : m_origin(origin)
        , m_direction(direction) {};

    auto const& get_origin() const { return m_origin; }
    auto const& get_direction() const { return m_direction; }
    auto get_point(double time) const { return m_origin + m_direction * time; }
};
