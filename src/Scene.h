#pragma once

#include <memory>
#include <vector>

#include "shapes/Shape.h"
#include "util/Config.h"
#include "util/Light.h"
#include "util/Ray.h"
#include "util/Record.h"
#include "util/Vec.h"

class Scene {
private:
    std::vector<std::shared_ptr<Shape>> m_shapes;
    std::vector<std::shared_ptr<Light>> m_lights;

public:
    Scene()
        : m_shapes({})
        , m_lights({}) {};

    void add_light(auto light)
    {
        m_lights.push_back(light);
    }

    void add_shape(auto shape)
    {
        m_shapes.push_back(shape);
    }

    auto const& get_lights() const { return m_lights; }

    auto const& get_shapes() const { return m_shapes; }

    auto find_intersection(Ray const& ray, double min, double max, Record& record) const
    {
        auto time = max;

        for (auto&& shape : m_shapes) {
            if (!shape->get_bounding_box().find_intersection(ray))
                continue;

            auto time_of_intersection = shape->find_intersection(ray, min, max);

            if (time_of_intersection > min && time_of_intersection < time) {
                time = time_of_intersection;

                auto point = ray.get_point(time);

                record = {
                    .m_material = shape->get_material(),
                    .m_time = time,
                    .m_point = point,
                    .m_normal = shape->get_normal(point)
                };
            }
        }

        return time != max;
    }

    __attribute__((flatten)) auto compute_ray_color(Ray const& ray, double min, double max, int depth) const
    {
        auto record = Record {};

        if (depth == RAYTRACER_MAX_RECURSION_DEPTH || !find_intersection(ray, min, max, record))
            return Vec3<double> { 0. };

        auto color = record.m_material.ka;
        auto E = normalize(ray.get_origin() - record.m_point);

        for (auto&& light : m_lights) {
            auto light_direction = normalize(light->m_position - record.m_point);
            auto light_time = light_direction.magnitude();

            light_direction.normalize();

            auto shadow_ray = Ray(record.m_point, light_direction);
            auto shadow_record = Record {};

            // There is an occluder between light and this point; continue to next light source.
            if (find_intersection(shadow_ray, RAYTRACER_EPSILON, light_time, shadow_record))
                continue;

            auto LN = dot(light_direction, record.m_normal);
            auto R = normalize((2. * LN * record.m_normal) - light_direction);

            auto diffuse = std::max(0., LN) * record.m_material.kd;
            auto specular = std::pow(std::max(0., dot(R, E)), record.m_material.s) * record.m_material.ks;

            color += light->m_color * (diffuse + specular);
        }

        auto reflection_ray = Ray(record.m_point, normalize(ray.get_direction() - 2. * dot(ray.get_direction(), record.m_normal) * record.m_normal));
        auto reflected_color = record.m_material.km * compute_ray_color(reflection_ray, RAYTRACER_EPSILON, std::numeric_limits<double>::infinity(), depth + 1);

        // Add color and reflected color; clamp to values between [0, 1].
        color.for_each([&reflected_color](auto& v, auto idx) {
            v = std::min(1., v + reflected_color[idx]);
        });

        return color;
    }
};
