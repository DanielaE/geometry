// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2012 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2012 Mateusz Loskot, London, UK.

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include "function_requiring_a_point.hpp"

#include <boost/geometry/core/cs.hpp>

struct point
{
    point() : x(0), y(0) {} // initialize to suppress warnings
    float x, y;
};


namespace boost { namespace geometry { namespace traits {

template <> struct tag<point> { typedef point_tag type; };
template <> struct coordinate_type<point> { typedef float type; };
template <> struct coordinate_system<point> { typedef bg::cs::cartesian type; };
//template <> struct dimension<point> { enum { value = 2 }; };

template <> struct access<point, 0>
{
    static float get(point const& p) { return p.x; }
    static void set(point& p, float value) { p.x = value; }
};

template <> struct access<point, 1>
{
    static float get(point const& p) { return p.y; }
    static void set(point& p, float value) { p.y = value; }
};


}}} // namespace bg::traits

#if defined(_MSC_VER)
#pragma warning(disable: 4269) // 'const' automatic data initialized with compiler generated default constructor produces unreliable results
#endif

int main()
{
    point p1;
    const point p2;
    test::function_requiring_a_point(p1, p2);
    return 0;
}
