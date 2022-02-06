#pragma once

#include "../util/Ray.h"
#include "Shape.h"

class Plane : public Shape {
private:
    Vec3<double> m_center;
    Vec3<double> m_normal;

public:
    Plane(auto center, auto normal, auto material)
        : Shape(material)
        , m_center(center)
        , m_normal(normal) {};

    Vec3<double> GetNormal(Vec3<double>) const override
    {
        return m_normal;
    }

    double Intersect(Ray ray, double min, double max) const override
    {
        auto const val = dot(m_center - ray.Origin(), m_normal) / dot(m_normal, ray.Direction());

        return std::min(std::max(val, min), max);
    }
};
