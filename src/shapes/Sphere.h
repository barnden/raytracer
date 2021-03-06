#pragma once

#include "../util/Ray.h"
#include "Shape.h"

class Sphere : public Shape {
private:
    Vec3<double> m_center;
    double m_radius;

public:
    Sphere(auto center, auto radius, auto material)
        : Shape({ center - radius, center + radius }, material)
        , m_center(center)
        , m_radius(radius) {};

    Vec3<double> get_normal(Vec3<double> point) const override
    {
        return normalize(point - m_center);
    }

    __attribute__((flatten)) double find_intersection(Ray ray, double min, double max) const override
    {
        auto oc = ray.get_origin() - m_center;
        auto b = dot(ray.get_direction(), oc);
        auto c = dot(oc, oc) - m_radius * m_radius;

        auto discriminant = b * b - c;

        if (discriminant < 0)
            return max;

        if (discriminant == 0)
            return std::max(-b, min);

        auto root = std::sqrt(discriminant);

        auto i0 = root - b;
        auto i1 = -(root + b);

        return std::min(std::max(std::min(i0, i1), min), max);
    }
};
