#pragma once
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Bbox_2.h>

namespace ciallo::geom
{
	using Kernel = CGAL::Simple_cartesian<float>;
	using Point = Kernel::Point_2;
	using Line = Kernel::Line_2;
	using Segment = Kernel::Segment_2;
	using Vector = Kernel::Vector_2;
	using Bbox = CGAL::Bbox_2; // Bounding box TODO:make a float version

	// using ExKernel = CGAL::Exact_predicates_exact_constructions_kernel; // Ex means exact, reserve for arrangement
}
