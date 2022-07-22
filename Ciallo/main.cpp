#include "pch.hpp"
#include "Application.hpp"
#include "Bezier.hpp"


int main()
{
    using namespace ciallo;
    std::vector<geom::Point> x{{0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.f}};
    geom::Bezier<3> y{x.begin(), x.end()};
    auto curves = y.split(0.5);
    std::cout << curves[0] << std::endl;
    std::cout << curves[1] << std::endl;

    Application a;
    a.run();
    return 0;
}