#pragma once

#include "Vec.h"

struct Material {
    Vec3<double> ka; // Ambient
    Vec3<double> kd; // Diffuse
    Vec3<double> ks; // Specular
    Vec3<double> km; // Reflectivity

    double m; // Roughness (rms slope of microfacets)
    double ior;
};
