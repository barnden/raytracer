#pragma once

#include <complex>

#include "../util/Ray.h"
#include "Shape.h"

#ifndef RAYTRACER_TORUS_THRESHOLD
#    define RAYTRACER_TORUS_THRESHOLD 0.00001
#endif

#ifndef RAYTRACER_TORUS_EPSILON
#    define RAYTRACER_TORUS_EPSILON RAYTRACER_TORUS_THRESHOLD
#endif

class Torus : public Shape {
private:
    Vec3<double> m_center;
    double m_major_radius;
    double m_minor_radius;

public:
    Torus(auto center, auto major_radius, auto minor_radius, auto material)
        : Shape({ center - std::abs(major_radius + minor_radius), center + std::abs(major_radius + minor_radius) }, material)
        , m_center(center)
        , m_major_radius(major_radius)
        , m_minor_radius(minor_radius) {};

    Vec3<double> GetNormal(Vec3<double> point) const override
    {
        // Torii eqn. f(x, y, z) = (x^2 + y^2 + z^2 + R^2 - r^2)^2 - 4R^2 (x^2 + y^2)
        auto point_difference = point - m_center;
        auto common = 1. - m_major_radius / std::sqrt(point_difference.x * point_difference.x + point_difference.y * point_difference.y);

        // grad f is two times the Vec3 here, but we're normalizing so it doesn't matter
        return normalize(Vec3<double> { point_difference.x * common,
                                        point_difference.y * common,
                                        point_difference.z });
    }

    __attribute__((flatten)) double SmallestRealQuarticRoot(std::complex<double> a, std::complex<double> b, std::complex<double> c, std::complex<double> d) const
    {
        auto P = std::complex<double>(1.);
        auto Q = std::complex<double>(.4, .9);
        auto R = Q * Q;
        auto S = R * Q;

        std::complex<double> Phat, Qhat, Rhat, Shat;
        double smallest_root = std::numeric_limits<double>::infinity();

        auto static const is_within_threshold = [](auto const& a, auto const& b) {
            return std::abs(a - b) < RAYTRACER_TORUS_THRESHOLD;
        };
        auto static const evaluate_polynomial = [&a, &b, &c, &d](auto const& x) {
            return d + x * (c + x * (b + x * (a + x)));
        };
        auto static const get_next_point = [](auto const& x, auto const& p, auto const& q, auto const& r) {
            return x - evaluate_polynomial(x) / ((x - p) * (x - q) * (x - r));
        };

        while (true) {
            Phat = get_next_point(P, Q, R, S);
            Qhat = get_next_point(Q, P, R, S);
            Rhat = get_next_point(R, P, Q, S);
            Shat = get_next_point(S, P, Q, R);

            if (is_within_threshold(Phat, P)
                && is_within_threshold(Qhat, Q)
                && is_within_threshold(Rhat, R)
                && is_within_threshold(Shat, S))
                break;

            P = Phat;
            Q = Qhat;
            R = Rhat;
            S = Shat;
        }

        if (std::abs(Phat.imag()) < RAYTRACER_TORUS_EPSILON)
            smallest_root = std::min(smallest_root, Phat.real());

        if (std::abs(Qhat.imag()) < RAYTRACER_TORUS_EPSILON)
            smallest_root = std::min(smallest_root, Qhat.real());

        if (std::abs(Rhat.imag()) < RAYTRACER_TORUS_EPSILON)
            smallest_root = std::min(smallest_root, Rhat.real());

        if (std::abs(Shat.imag()) < RAYTRACER_TORUS_EPSILON)
            smallest_root = std::min(smallest_root, Shat.real());

        return smallest_root;
    }

    __attribute__((flatten)) double Intersect(Ray ray, double min, double max) const override
    {
        // From: http://cosinekitty.com/raytrace/chapter13_torus.html

        auto const P = ray.Origin() - m_center;
        auto const T = 4. * m_major_radius * m_major_radius;
        auto const G = T * (ray.Direction().x * ray.Direction().x + ray.Direction().y * ray.Direction().y);
        auto const H = 2. * T * (P.x * ray.Direction().x + P.y * ray.Direction().y);
        auto const I = T * (P.x * P.x + P.y * P.y);
        auto const J = dot(ray.Direction(), ray.Direction());
        auto const K = 2. * dot(ray.Direction(), P);
        auto const L = dot(P, P) + m_major_radius * m_major_radius - m_minor_radius * m_minor_radius;
        auto const M = 1. / (J * J);

        auto root = SmallestRealQuarticRoot(
            M * (2 * J * K),
            M * (2 * J * L + K * K - G),
            M * (2 * K * L - H),
            M * (L * L - I)
        );

        return std::min(std::max(root, min), max);
    }
};
