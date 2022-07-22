#include "pch.hpp"
#include "Application.hpp"
#include "Bezier.hpp"


int main()
{
    using namespace ciallo;
    std::vector<geom::Point> x{{0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.f}};
    geom::Bezier<3> y{x.begin(), x.end()};
    geom::Vector v{0.5f, 0.5f};
    auto vy = y + v;
    std::cout << vy << std::endl;

    CGAL::Aff_transformation_2<ciallo::geom::Kernel> rotaion(CGAL::TRANSLATION, geom::Vector{0.5f ,0.5f});
    std::cout << rotaion(geom::Point{1.f, 1.f}) << std::endl;

    Application a;
    a.run();
    return 0;
}