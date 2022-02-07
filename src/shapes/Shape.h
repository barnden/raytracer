#pragma once

#include "../util/BoundingBox.h"
#include "../util/Material.h"

class Shape {
private:
    BoundingBox m_bounding_box;
    Material m_material;

public:
    Shape(Material material)
        : m_bounding_box(BoundingBox {})
        , m_material(material) {};

    Shape(BoundingBox bounding_box, Material material)
        : m_bounding_box(bounding_box)
        , m_material(material) {};

    auto const& get_bounding_box() const { return m_bounding_box; }
    auto const& get_material() const { return m_material; }

    void set_bounding_box(BoundingBox&& bounding_box) { m_bounding_box = std::move(bounding_box); }

    virtual Vec3<double> get_normal(Vec3<double> point) const = 0;
    virtual double find_intersection(Ray ray, double min, double max) const = 0;
};
