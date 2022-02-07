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

    auto const& GetBoundingBox() const { return m_bounding_box; }
    auto const& GetMaterial() const { return m_material; }

    void SetBoundingBox(BoundingBox&& bounding_box) { m_bounding_box = std::move(bounding_box); }

    virtual Vec3<double> GetNormal(Vec3<double> point) const = 0;
    virtual double Intersect(Ray ray, double min, double max) const = 0;
};
