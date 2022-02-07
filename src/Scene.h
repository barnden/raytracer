#pragma once

#include <memory>
#include <vector>

#include "shapes/Shape.h"
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

    bool find_intersection(Ray ray, double min, double max, Record& record) const
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
};
