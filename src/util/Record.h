#pragma once

#include "Material.h"
#include "Vec.h"

struct Record {
    Material m_material;
    double m_time;
    Vec3<double> m_point;
    Vec3<double> m_normal;
};
