#pragma once
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Bbox_2.h>
#include <iterator>

namespace ciallo::geom
{
	using Kernel = CGAL::Simple_cartesian<float>;
	using Point = Kernel::Point_2;
	using Line = Kernel::Line_2;
	using Segment = Kernel::Segment_2;
	using Vector = Kernel::Vector_2;
	using Bbox = CGAL::Bbox_2; // Bounding box of double TODO:make a float version


	template <typename T, typename Iter>
	concept iter_value_same = std::is_same_v< T,
		typename std::iter_value_t<Iter>
	>;

	template <typename Iter>
	concept point_iter = iter_value_same<Point, Iter>;

	// using ExKernel = CGAL::Exact_predicates_exact_constructions_kernel; // Ex means exact, reserve for arrangement
}
