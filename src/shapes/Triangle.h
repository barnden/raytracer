#pragma once

#include "../util/Ray.h"
#include "Shape.h"

class Triangle : public Shape {
private:
    Vec3<double> m_v0;
    Vec3<double> m_v1;
    Vec3<double> m_v2;

    Vec3<double> m_E1;
    Vec3<double> m_E2;

    Vec3<double> m_normal;

public:
    Triangle(auto v0, auto v1, auto v2, auto material)
        : Shape(material)
        , m_v0(v0)
        , m_v1(v1)
        , m_v2(v2)
        , m_E1(v1 - v0)
        , m_E2(v2 - v0)
    {
        auto min_point = Vec3<double> {
            std::min(m_v0.x, std::min(m_v1.x, m_v2.x)),
            std::min(m_v0.y, std::min(m_v1.y, m_v2.y)),
            std::min(m_v0.z, std::min(m_v1.z, m_v2.z))
        };

        auto max_point = Vec3<double> {
            std::max(m_v0.x, std::max(m_v1.x, m_v2.x)),
            std::max(m_v0.y, std::max(m_v1.y, m_v2.y)),
            std::max(m_v0.z, std::max(m_v1.z, m_v2.z))
        };

        m_bounding_box = BoundingBox(min_point, max_point);
        m_normal = normalize(cross(m_E1, m_E2));

        // If the determinant of the 3x3 matrix of (v0, v1, v2) is negative
        // then the points were given in anticlockwise order, and we need to
        // flip direction of the normal vector
        if (dot(cross(m_v0, m_v1), m_v2) >= 0.)
            m_normal *= -1.;
    }

    Vec3<double> GetNormal(Vec3<double>) const override
    {
        return m_normal;
    }

    double Intersect(Ray ray, double min, double max) const override
    {
        // Möller–Trumbore intersection algorithm
        auto S = ray.Origin() - m_v0;
        auto S1 = cross(ray.Direction(), m_E2);
        auto S2 = cross(S, m_E1);

        auto invS1E1 = 1. / dot(S1, m_E1);

        auto b1 = invS1E1 * dot(S1, S);
        auto b2 = invS1E1 * dot(S2, ray.Direction());

        if (b1 + b2 > 1. || b1 < 0. || b2 < 0.)
            return max;

        return std::min(std::max(min, invS1E1 * dot(S2, m_E2)), max);
    }
};
