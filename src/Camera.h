#pragma once

#include "Scene.h"
#include "util/Ray.h"
#include "util/Record.h"
#include "util/Vec.h"

#ifndef RAYTRACER_MAX_RECURSION_DEPTH
#    define RAYTRACER_MAX_RECURSION_DEPTH 8
#endif

#ifndef RAYTRACER_EPSILON
#    define RAYTRACER_EPSILON 0.0075
#endif

#ifndef RAYTRACER_REFLECTIVITY_EPSILON
#    define RAYTRACER_REFLECTIVITY_EPSILON RAYTRACER_EPSILON
#endif

class Camera {
private:
    Vec3<double> m_eye;
    Vec3<double> m_look_at;
    Vec3<double> m_up;

    double m_fov_y;
    double m_focal_distance;

    uint32_t m_viewport_width;
    uint32_t m_viewport_height;

    Vec3<double> m_u;
    Vec3<double> m_v;
    Vec3<double> m_w;

    Vec3<double> m_focal_plane_origin;
    Vec3<double> m_focal_plane_center;
    double m_focal_plane_height;
    double m_focal_plane_width;
    double m_pixel_width;

public:
    Camera(auto eye, auto look_at, auto up, auto fov_y, auto focal_distance, auto width, auto height)
        : m_eye(eye)
        , m_look_at(look_at)
        , m_up(up)
        , m_focal_distance(focal_distance)
        , m_viewport_width(width)
        , m_viewport_height(height)
    {
        m_w = normalize(eye - look_at);
        m_u = normalize(cross(up, m_w));
        m_v = cross(m_w, m_u);

        m_focal_plane_height = 2 * focal_distance * tan(fov_y / 360. * M_PI);
        m_pixel_width = m_focal_plane_height / height;
        m_focal_plane_width = m_pixel_width * width;

        m_focal_plane_center = eye + focal_distance * normalize(look_at - eye);
        m_focal_plane_origin = m_focal_plane_center - (((m_focal_plane_width / 2.) * m_u) + ((m_focal_plane_height / 2.) * m_v));
    }

    void RenderChunk(Scene const& scene, std::size_t u, std::size_t v, auto& pixels) const
    {
        for (auto i = u << 6; i < (u + 1) << 6; i++)
            for (auto j = v << 6; j < (v + 1) << 6; j++) {
                auto look_at = m_focal_plane_origin + (.5 + i) * m_pixel_width * m_u + (.5 + j) * m_pixel_width * m_v;
                auto direction = normalize(look_at - m_eye);

                auto color = ComputeRay(scene, { look_at, direction }, 0., std::numeric_limits<double>::infinity(), 0);
                auto pixel_idx = (m_viewport_height - (j + 1)) * m_viewport_width + i;

                color.for_each([&](auto& a, auto idx) {
                    pixels[pixel_idx][idx] = static_cast<uint8_t>(std::min(255., 255. * a));
                });
            }
    }

    auto Render(Scene const& scene) const
    {
        std::vector<Vec3<uint8_t>> pixels(m_viewport_width * m_viewport_height, Vec3<uint8_t> { 0 });

        for (auto u = 0uz; u < (m_viewport_width >> 6); u++)
            for (auto v = 0uz; v < (m_viewport_height >> 6); v++)
                RenderChunk(scene, u, v, pixels);

        return pixels;
    }

    Vec3<double> ComputeRay(Scene const& scene, Ray ray, double min, double max, int depth) const
    {
        auto record = Record {};

        if (depth == RAYTRACER_MAX_RECURSION_DEPTH || !scene.Hit(ray, min, max, record))
            return { 0. };

        auto color = record.m_material.ka;
        auto E = normalize(ray.Origin() - record.m_point);

        for (auto&& light : scene.Lights()) {
            auto light_direction = light->m_position - record.m_point;
            auto light_time = light_direction.magnitude();

            light_direction.normalize();

            auto shadow_ray = Ray(record.m_point, light_direction);
            auto shadow_record = Record {};

            // There is an occluder between light and this point; continue to next light source.
            if (scene.Hit(shadow_ray, RAYTRACER_EPSILON, light_time, shadow_record))
                continue;

            // Phong reflection model for lighting
            auto LN = dot(light_direction, record.m_normal);
            auto R = normalize((2. * LN * record.m_normal) - light_direction);

            auto diffuse = std::max(0., LN) * record.m_material.kd;
            auto specular = std::pow(std::max(0., dot(R, E)), record.m_material.s) * record.m_material.ks;

            color += light->m_color * (diffuse + specular);
        }

        // Reflectivity of the object is so low that it is not worth rendering
        if (dot(record.m_material.km, record.m_material.km) <= RAYTRACER_REFLECTIVITY_EPSILON)
            return color;

        // Reflection ray originates from point of intersection of ray and object
        auto reflection_ray = Ray(record.m_point, normalize(ray.Direction() - 2. * dot(ray.Direction(), record.m_normal) * record.m_normal));
        auto reflected_color = record.m_material.km * ComputeRay(scene, reflection_ray, RAYTRACER_EPSILON, std::numeric_limits<double>::infinity(), depth + 1);

        // Add color and reflected color; clamp to values between [0, 1].
        color.for_each([&reflected_color](auto& v, auto idx) {
            v = std::min(1., v + reflected_color[idx]);
        });

        return color;
    }
};
