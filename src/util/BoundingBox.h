#pragma once

#include <limits>

#include "Ray.h"
#include "Vec.h"

class BoundingBox {
private:
    Vec3<double> m_min;
    Vec3<double> m_max;

public:
    BoundingBox()
        : m_min(Vec3<double>(-std::numeric_limits<double>::infinity()))
        , m_max(Vec3<double>(std::numeric_limits<double>::infinity())) {};

    BoundingBox(auto min, auto max)
        : m_min(min)
        , m_max(max) {};

    bool Intersect(Ray const& ray) const
    {
        auto min = (m_min - ray.Origin()) / ray.Direction();
        auto max = (m_max - ray.Origin()) / ray.Direction();

        auto tmin = std::min(min.x, max.x);
        auto tmax = std::max(min.x, max.x);

        tmin = std::max(tmin, std::min(std::min(min.y, max.y), tmax));
        tmax = std::max(tmax, std::max(std::max(min.y, max.y), tmin));

        tmin = std::max(tmin, std::min(std::min(min.z, max.z), tmax));
        tmax = std::max(tmax, std::max(std::max(min.z, max.z), tmin));

        return tmax > std::max(tmin, 0.);
    }
};
