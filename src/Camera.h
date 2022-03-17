#pragma once

#include <atomic>
#include <mutex>
#include <thread>

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

    std::vector<Vec3<uint8_t>> m_pixels;
    std::vector<std::thread> m_workers;

    std::mutex m_mutex;

public:
    Camera(auto eye, auto look_at, auto up, auto fov_y, auto focal_distance, auto width, auto height)
        : m_eye(eye)
        , m_look_at(look_at)
        , m_up(up)
        , m_focal_distance(focal_distance)
        , m_viewport_width(width)
        , m_viewport_height(height)
        , m_pixels(m_viewport_width * m_viewport_height, Vec3<uint8_t> { 0 })
        , m_mutex {}
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

    friend __attribute__((flatten)) auto render_chunk(
        Camera const& camera,
        Scene const& scene,
        std::size_t u,
        std::size_t v)
    {
        // Chunks are 64 x 64 pixels, so total area of 4096 pixels.
        auto colors = std::vector<Vec3<double>>(4096uz);
        auto i0 = u << 6;
        auto j0 = v << 6;

        for (auto i = i0; i < i0 + 64; i++)
            for (auto j = j0; j < j0 + 64; j++) {
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
                auto look_at = camera.m_focal_plane_origin + (.5 + i) * camera.m_pixel_width * camera.m_u + (.5 + j) * camera.m_pixel_width * camera.m_v;
                auto direction = normalize(look_at - camera.m_eye);
                auto color = scene.compute_ray_color({ look_at, direction }, 0., std::numeric_limits<double>::infinity(), 0);
#endif
                auto idx = (j - j0) + ((i - i0) << 6);
                colors[idx] = color;
            }

        return colors;
    }

    static void render_worker(
        Camera const& camera,
        Scene const& scene,
        std::mutex& mutex,
        std::vector<Vec3<uint8_t>>& viewport,
        std::vector<std::pair<size_t, size_t>>& chunks,
        std::atomic_uint32_t& finished_workers)
    {
        auto lock = std::unique_lock<std::mutex>(mutex, std::defer_lock);

        auto pixels = std::vector<Vec3<double>> {};
        auto rendered_chunks = std::vector<std::pair<size_t, size_t>> {};

        while (true) {
            lock.lock();

            if (!chunks.size()) {
                lock.unlock();
                break;
            }

            auto chunk = chunks.back();
            chunks.pop_back();

            lock.unlock();
            rendered_chunks.push_back(chunk);

            auto render = render_chunk(camera, scene, chunk.first, chunk.second);
            pixels.insert(pixels.end(), render.begin(), render.end());
        }

        for (auto chunk_idx = 0uz; chunk_idx < rendered_chunks.size(); chunk_idx++) {
            auto u = rendered_chunks[chunk_idx].first;
            auto v = rendered_chunks[chunk_idx].second;

            auto i0 = u << 6;
            auto j0 = v << 6;

            lock.lock();
            for (auto i = i0; i < i0 + 64; i++)
                for (auto j = j0; j < j0 + 64; j++) {
                    auto const viewport_idx = (camera.m_viewport_height - (j + 1)) * camera.m_viewport_width + i;
                    auto const pixel_idx = (chunk_idx << 12) + (j - j0) + ((i - i0) << 6);
                    auto&& color = pixels[pixel_idx];

                    color.for_each_const([&](auto const& a, auto idx) {
                        viewport[viewport_idx][idx] = static_cast<uint8_t>(std::min(255., 255. * a));
                    });
                }
            lock.unlock();
        }

        finished_workers++;
    }

    __attribute__((flatten)) auto render(Scene const& scene)
    {
        auto chunks = std::vector<std::pair<size_t, size_t>> {};
        auto finished_workers = std::atomic_uint32_t {};

        for (auto u = 0uz; u < (m_viewport_width >> 6); u++)
            for (auto v = 0uz; v < (m_viewport_height >> 6); v++)
                chunks.push_back(std::make_pair(u, v));

        for (auto i = 0uz; i < MULTITHREAD_WORKERS; i++) {
            auto thread = std::thread(
                render_worker,
                std::ref(*this),
                std::ref(scene),
                std::ref(m_mutex),
                std::ref(m_pixels),
                std::ref(chunks),
                std::ref(finished_workers));

            thread.detach();

            m_workers.push_back(std::move(thread));
        }

        while (finished_workers != MULTITHREAD_WORKERS) { };

        return m_pixels;
    }
};
