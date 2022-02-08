#include "util/BoundingBox.h"
#include "util/Material.h"
#include "util/Ray.h"
#include "util/Vec.h"

#include "shapes/Plane.h"
#include "shapes/Sphere.h"
#include "shapes/Torus.h"
#include "shapes/Triangle.h"

#include "Camera.h"
#include "Scene.h"

#include <cstddef>
#include <fstream>

int main(int argc, char** argv)
{
    using Vec3 = Vec3<double>;

    auto const width = 1024;
    auto const height = 768;

    auto camera = Camera(
        Vec3 { 0., 0., 6. },
        Vec3 { 0., 0., 1. },
        Vec3 { .25, .85, .5 },
        65.,
        1.,
        width,
        height);

    auto scene = Scene();

    scene.add_light(std::make_shared<Light>(Vec3 { 0., 3., -2. }, Vec3 { .2 }));
    scene.add_light(std::make_shared<Light>(Vec3 { -2., 1., 4. }, Vec3 { .5 }));

    scene.add_shape(std::make_shared<Sphere>(
        Vec3 { -1., -.7, 3. },
        .3,
        Material(
            Vec3 { .1 },
            Vec3 { .3, .6, .3 },
            Vec3 { 1. },
            Vec3 { .1, .4, .1 },
            100.)));

    scene.add_shape(std::make_shared<Sphere>(
        Vec3 { 1., -.5, 3. },
        .5,
        Material {
            Vec3 { .1 },
            Vec3 { 1., .0, 0. },
            Vec3 { 1. },
            Vec3 { 0. },
            10. }));

    scene.add_shape(std::make_shared<Sphere>(
        Vec3 { -1., 0., 0. },
        1.,
        Material {
            Vec3 { 0. },
            Vec3 { 0. },
            Vec3 { 0. },
            Vec3 { 1. },
            0. }));

    scene.add_shape(std::make_shared<Sphere>(
        Vec3 { 1., 0., -1. },
        1.,
        Material {
            Vec3 { 0. },
            Vec3 { 0. },
            Vec3 { 0. },
            Vec3 { 1. },
            0. }));

    scene.add_shape(std::make_shared<Plane>(
        Vec3 { -1., -3., 0. },
        Vec3 { 0., 1., 0. },
        Material {
            Vec3 { .1 },
            Vec3 { 1. },
            Vec3 { 0. },
            Vec3 { 0. },
            0. }));

    scene.add_shape(std::make_shared<Plane>(
        Vec3 { 0., 0., -3. },
        Vec3 { 1., 0., 1. },
        Material {
            Vec3 { .1 },
            Vec3 { 1. },
            Vec3 { 0. },
            Vec3 { 0. },
            0. }));

    scene.add_shape(std::make_shared<Triangle>(
        Vec3 { 2., 0., -1. },
        Vec3 { 2.75, 3., -.5 },
        Vec3 { 3., .5, 1. },
        Material {
            Vec3 { .1 },
            Vec3 { .2, .2, 1. },
            Vec3 { 1. },
            Vec3 { 0. },
            100. }));

    scene.add_shape(std::make_shared<Torus>(
        Vec3 { -1., -.7, 3. },
        .5,
        .1,
        Material {
            Vec3 { .1 },
            Vec3 { 0. },
            Vec3 { 1. },
            Vec3 { .8 },
            100. }));

    auto pixels = camera.render(scene);

    auto out = std::ofstream("test.pbm");

    out << "P3\n"
        << width << ' ' << height << "\n255\n";

    for (auto&& pixel : pixels)
        out << (+pixel.x) << '\t' << (+pixel.y) << '\t' << (+pixel.z) << '\t';

    return 0;
}
