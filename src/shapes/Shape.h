#pragma once

#include "../util/BoundingBox.h"
#include "../util/Material.h"

class Shape {
private:
    BoundingBox m_bounding_box;
    Material m_material;

public:
    Shape(auto material)
        : m_bounding_box(BoundingBox {})
        , m_material(material) {};

    Shape(auto bounding_box, auto material)
        : m_bounding_box(bounding_box)
        , m_material(material) {};

    virtual Vec3<double> GetNormal(Vec3<double> point) const = 0;
    virtual double Intersect(Ray ray, double min, double max) const = 0;
};
