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

    void AddLight(auto light)
    {
        m_lights.push_back(light);
    }

    void AddShape(auto shape)
    {
        m_shapes.push_back(shape);
    }

    auto const& Lights() const { return m_lights; }

    auto const& Shapes() const { return m_shapes; }

    bool Hit(Ray ray, double min, double max, Record& record) const
    {
        auto time = max;

        for (auto&& shape : m_shapes) {
            if (!shape->GetBoundingBox().Intersect(ray))
                continue;

            auto time_of_intersection = shape->Intersect(ray, min, max);

            if (time_of_intersection > min && time_of_intersection < time) {
                time = time_of_intersection;

                auto point = ray.GetPoint(time);

                record = {
                    .m_material = shape->GetMaterial(),
                    .m_time = time,
                    .m_point = point,
                    .m_normal = shape->GetNormal(point)
                };
            }
        }

        return time != max;
    }
};
