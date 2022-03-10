#pragma once

#include "Config.h"
#include "Vec.h"

#include <cmath>

namespace Raytracer {

namespace Lighting {
    auto inline schlick_approximation(auto r0, auto cos)
    {
        /**
         * Schlick Fresnel approximation
         * Approximates the Fresnel factor in specular reflection

         * IoR - index of reflection
         *  r0 = ((IoR1 - IoR2) / (IoR1 + IoR2)) ^ 2.
         *      Note: order of IoR1/IoR2 does not matter due to the squaring
         * cos = dot(N, V)
         *      N - normal between mediums
         *          alternatively, we can use the halfway vector
         *          H = normalize(L + V) instead of the normal
         *      V - viewing direction or light direction
         */

        return r0 + (1. - r0) * pow(1. - cos, 5.);
    }

    auto inline beckmann_distribution(auto HN, auto m)
    {
        /**
         * Beckmann Distribution
         * A physically based model of microfacet distribution
         *
         * HN - dot(N, H)
         * m - rms slope of microfacets on surface, i.e. roughness
         */

        auto cos2 = HN * HN;
        auto tan2 = (cos2 - 1.) / cos2;
        auto m2 = m * m;

        return std::exp(tan2 / m2) / (M_PI * m2 * cos2 * cos2);
    }

    auto inline geometric_attenuation(auto HN, auto LN, auto EN, auto EH)
    {
        // Geometric attenuation describes selfshadowing from microfacets

        auto common = 2. * HN / EH;
        return std::min(1., std::min(common * EN, common * LN));
    }
}

__attribute__((flatten)) auto specular(auto const& L, auto const& N, auto const& E, auto ior1, auto ior2, auto m)
{
    /**
     * Cook-Torrance Specular BRDF
     *
     * L - light direction
     * N - normal
     * E - viewing direction (typically denoted by V)
     * ior1/ior2 - index of refraction
     * m - roughness (see beckmann_distribution)
     */

    auto H = normalize(L + E); // Halfway vector

    auto EN = dot(E, N);
    auto LN = dot(L, N);
    auto HN = std::max(RAYTRACER_EPSILON, dot(H, N));

    auto G = Lighting::geometric_attenuation(HN, LN, EN, dot(E, H));
    auto D = Lighting::beckmann_distribution(HN, m);

    auto r0 = std::pow((ior1 - ior2) / (ior1 + ior2), 2.);
    auto F = Lighting::schlick_approximation(r0, EN);

    return (D * F * G) / (4. * EN * LN);
}

}
