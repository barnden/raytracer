#pragma once

#include "Scene.h"
#include "util/Config.h"
#include "util/Ray.h"
#include "util/Record.h"
#include "util/Vec.h"

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
        m_w = normalize(look_at - eye);
        m_u = normalize(cross(m_w, up));
        m_v = cross(m_u, m_w);

        m_focal_plane_height = 2 * focal_distance * tan(fov_y / 360. * M_PI);
        m_pixel_width = m_focal_plane_height / height;
        m_focal_plane_width = m_pixel_width * width;

        m_focal_plane_center = eye + focal_distance * normalize(look_at - eye);
        m_focal_plane_origin = m_focal_plane_center - (((m_focal_plane_width / 2.) * m_u) + ((m_focal_plane_height / 2.) * m_v));
    }

    __attribute__((flatten)) void render_chunk(Scene const& scene, std::size_t u, std::size_t v, auto& pixels) const
    {
        for (auto i = u << 6; i < (u + 1) << 6; i++)
            for (auto j = v << 6; j < (v + 1) << 6; j++) {

#ifdef ENABLE_SSAA
                auto get_look_at = [&](double a, double b) {
                    return m_focal_plane_origin + (a + i) * m_pixel_width * m_u + (b + j) * m_pixel_width * m_v;
                };
                auto look_at = Vec3 { 0. };
                auto get_direction = [&]() { return normalize(look_at - m_eye); };

                auto color = scene.compute_ray_color({ look_at = get_look_at(.25, .25), get_direction() }, 0., std::numeric_limits<double>::infinity(), 0)
                             + scene.compute_ray_color({ look_at = get_look_at(.25, .75), get_direction() }, 0., std::numeric_limits<double>::infinity(), 0)
                             + scene.compute_ray_color({ look_at = get_look_at(.75, .25), get_direction() }, 0., std::numeric_limits<double>::infinity(), 0)
                             + scene.compute_ray_color({ look_at = get_look_at(.75, .75), get_direction() }, 0., std::numeric_limits<double>::infinity(), 0);

                color /= 4.;
#else
                auto look_at = m_focal_plane_origin + (.5 + i) * m_pixel_width * m_u + (.5 + j) * m_pixel_width * m_v;
                auto direction = normalize(look_at - m_eye);
                auto color = scene.compute_ray_color({ look_at, direction }, 0., std::numeric_limits<double>::infinity(), 0);
#endif
                auto pixel_idx = (m_viewport_height - (j + 1)) * m_viewport_width + i;

                color.for_each([&](auto& a, auto idx) {
                    pixels[pixel_idx][idx] = static_cast<uint8_t>(std::min(255., 255. * a));
                });
            }
    }

    __attribute__((flatten)) auto render(Scene const& scene) const
    {
        std::vector<Vec3<uint8_t>> pixels(m_viewport_width * m_viewport_height, Vec3<uint8_t> { 0 });

        for (auto u = 0uz; u < (m_viewport_width >> 6); u++)
            for (auto v = 0uz; v < (m_viewport_height >> 6); v++)
                render_chunk(scene, u, v, pixels);

        return pixels;
    }
};
