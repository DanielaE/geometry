// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2013, 2014.
// Modifications copyright (c) 2013-2014 Oracle and/or its affiliates.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_GET_TURN_INFO_FOR_ENDPOINT_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_GET_TURN_INFO_FOR_ENDPOINT_HPP

#include <boost/geometry/algorithms/detail/overlay/get_turn_info.hpp>

namespace boost { namespace geometry {

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay {

// TURN_OPERATION

enum turn_position { position_middle, position_front, position_back };

template <typename SegmentRatio>
struct turn_operation_linear
    : public turn_operation<SegmentRatio>
{
    turn_operation_linear()
        : position(position_middle)
        , is_collinear(false)
    {}

    turn_position position;
    bool is_collinear; // valid only for Linear geometry
};

template <typename Point1, typename Point2, typename TurnPoint, typename RobustPolicy>
class intersection_info
{
    typedef typename strategy_intersection
        <
            typename cs_tag<TurnPoint>::type,
            Point1,
            Point2,
            TurnPoint,
            RobustPolicy
        >::segment_intersection_strategy_type strategy;

public:
    typedef model::referring_segment<Point1 const> segment_type1;
    typedef model::referring_segment<Point2 const> segment_type2;
    typedef side_calculator<Point1, Point2> side_calculator_type;
    
    typedef typename strategy::return_type result_type;
    typedef typename boost::tuples::element<0, result_type>::type i_info_type; // intersection_info
    typedef typename boost::tuples::element<1, result_type>::type d_info_type; // dir_info

    intersection_info(Point1 const& pi, Point1 const& pj, Point1 const& pk,
                      Point2 const& qi, Point2 const& qj, Point2 const& qk,
                      RobustPolicy const& robust_policy)
        : m_result(strategy::apply(segment_type1(pi, pj),
                                   segment_type2(qi, qj),
                                   robust_policy))
        , m_side_calc(pi, pj, pk, qi, qj, qk)
        , m_robust_policy(robust_policy)
    {}

    inline Point1 const& pi() const { return m_side_calc.m_pi; }
    inline Point1 const& pj() const { return m_side_calc.m_pj; }
    inline Point1 const& pk() const { return m_side_calc.m_pk; }

    inline Point2 const& qi() const { return m_side_calc.m_qi; }
    inline Point2 const& qj() const { return m_side_calc.m_qj; }
    inline Point2 const& qk() const { return m_side_calc.m_qk; }

    inline side_calculator_type const& sides() const { return m_side_calc; }
    inline result_type const& result() const { return m_result; }
    inline i_info_type const& i_info() const { return m_result.template get<0>(); }
    inline d_info_type const& d_info() const { return m_result.template get<1>(); }

    // TODO: not it's more like is_spike_ip_p
    inline bool is_spike_p() const
    {
        if ( m_side_calc.pk_wrt_p1() == 0 )
        {
            if ( ! is_ip_j<0>() )
                return false;

            int const qk_p1 = m_side_calc.qk_wrt_p1();
            int const qk_p2 = m_side_calc.qk_wrt_p2();
                
            if ( qk_p1 == -qk_p2 )
            {
                if ( qk_p1 == 0 )
                {
                    return is_spike_of_collinear(pi(), pj(), pk());
                }
                        
                return true;
            }
        }
        
        return false;
    }

    // TODO: not it's more like is_spike_ip_q
    inline bool is_spike_q() const
    {
        if ( m_side_calc.qk_wrt_q1() == 0 )
        {
            if ( ! is_ip_j<1>() )
                return false;

            int const pk_q1 = m_side_calc.pk_wrt_q1();
            int const pk_q2 = m_side_calc.pk_wrt_q2();
                
            if ( pk_q1 == -pk_q2 )
            {
                if ( pk_q1 == 0 )
                {
                    return is_spike_of_collinear(qi(), qj(), qk());
                }
                        
                return true;
            }
        }
        
        return false;
    }

private:
    template <typename Point>
    inline bool is_spike_of_collinear(Point const& i, Point const& j, Point const& k) const
    {
        typedef model::referring_segment<Point const> seg_t;

        typedef strategy_intersection
            <
                typename cs_tag<Point>::type, Point, Point, Point, RobustPolicy
            > si;
        
        typedef typename si::segment_intersection_strategy_type strategy;
        
        typename strategy::return_type result
            = strategy::apply(seg_t(i, j), seg_t(j, k), m_robust_policy);
        
        return result.template get<0>().count == 2;
    }

    template <std::size_t OpId>
    bool is_ip_j() const
    {
        int arrival = d_info().arrival[OpId];
        bool same_dirs = d_info().dir_a == 0 && d_info().dir_b == 0;

        if ( same_dirs )
        {
            if ( i_info().count == 2 )
            {
                if ( ! d_info().opposite )
                {
                    return arrival != -1;
                }
                else
                {
                    return arrival != -1;
                }
            }
            else
            {
                return arrival == 0;
            }
        }
        else
        {
            return arrival == 1;
        }
    }

    result_type m_result;
    side_calculator_type m_side_calc;
    RobustPolicy const& m_robust_policy;
};

// SEGMENT_INTERSECTION RESULT

//                   C  H0  H1  A0  A1   O              IP1 IP2

// D0 and D1 == 0

// |-------->        2   0   0   0   0   F              i/i x/x
// |-------->
//
// |-------->        2   0   0   0   0   T              i/x x/i
// <--------|
//
// |----->           1   0   0   0   0   T              x/x
//       <-----|
//

// |--------->       2   0   0   0   1   T              i/x x/i
//      <----|
//
// |--------->       2   0   0   0   0   F              i/i x/x
//      |---->
//
// |--------->       2   0   0  -1   1   F              i/i u/x
// |---->
//
// |--------->       2   0   0  -1   0   T              i/x u/i
// <----|

// |------->         2   0   0   1  -1   F   and        i/i x/u
//     |------->     2   0   0  -1   1   F   symetric   i/i u/x
// |------->
//
//     |------->     2   0   0  -1  -1   T              i/u u/i
// <-------|
//
// |------->         2   0   0   1   1   T              i/x x/i
//     <-------|
//
// |-------->        2   0   0  -1   1   F              i/i u/x
//   |---->
//
// |-------->        2   0   0  -1   1   T              i/x u/i
//   <----|

//       |----->     1  -1  -1  -1  -1   T              u/u
// <-----|
//
//       |----->     1  -1   0  -1   0   F   and        u/x
// |----->           1   0  -1   0  -1   F   symetric   x/u
//       |----->

// D0 or D1 != 0

//          ^
//          |
//          +        1  -1   1  -1   1   F   and        u/x  (P is vertical)
// |-------->        1   1  -1   1  -1   F   symetric   x/u  (P is horizontal)
// ^
// |
// +
//
//          +
//          |
//          v
// |-------->        1   1   1   1   1   F              x/x  (P is vertical)
//
// ^
// |
// +
// |-------->        1  -1  -1  -1  -1   F              u/u  (P is vertical)
//
//      ^
//      |
//      +
// |-------->        1   0  -1   0  -1   F              u/u  (P is vertical)
//
//      +
//      |
//      v
// |-------->        1   0   1   0   1   F              u/x  (P is vertical)
//

class linear_intersections
{
public:
    template <typename Point1, typename Point2, typename IntersectionResult>
    linear_intersections(Point1 const& pi,
                         Point2 const& qi,
                         IntersectionResult const& result,
                         bool is_p_last, bool is_q_last)
    {
        int arrival_a = result.template get<1>().arrival[0];
        int arrival_b = result.template get<1>().arrival[1];
        bool same_dirs = result.template get<1>().dir_a == 0
                      && result.template get<1>().dir_b == 0;

        if ( same_dirs )
        {
            if ( result.template get<0>().count == 2 )
            {
                if ( ! result.template get<1>().opposite )
                {
                    ips[0].p_operation = operation_intersection;
                    ips[0].q_operation = operation_intersection;
                    ips[1].p_operation = union_or_blocked_same_dirs(arrival_a, is_p_last);
                    ips[1].q_operation = union_or_blocked_same_dirs(arrival_b, is_q_last);

                    ips[0].is_pi
                        = equals::equals_point_point(
                            pi, result.template get<0>().intersections[0]);
                    ips[0].is_qi
                        = equals::equals_point_point(
                            qi, result.template get<0>().intersections[0]);
                    ips[1].is_pj = arrival_a != -1;
                    ips[1].is_qj = arrival_b != -1;
                }
                else
                {
                    ips[0].p_operation = operation_intersection;
                    ips[0].q_operation = union_or_blocked_same_dirs(arrival_b, is_q_last);
                    ips[1].p_operation = union_or_blocked_same_dirs(arrival_a, is_p_last);
                    ips[1].q_operation = operation_intersection;

                    ips[0].is_pi = arrival_b != 1;
                    ips[0].is_qj = arrival_b != -1;
                    ips[1].is_pj = arrival_a != -1;
                    ips[1].is_qi = arrival_a != 1;
                }
            }
            else
            {
                BOOST_ASSERT(result.template get<0>().count == 1);
                ips[0].p_operation = union_or_blocked_same_dirs(arrival_a, is_p_last);
                ips[0].q_operation = union_or_blocked_same_dirs(arrival_b, is_q_last);

                ips[0].is_pi = arrival_a == -1;
                ips[0].is_qi = arrival_b == -1;
                ips[0].is_pj = arrival_a == 0;
                ips[0].is_qj = arrival_b == 0;
            }
        }
        else
        {
            ips[0].p_operation = union_or_blocked_different_dirs(arrival_a, is_p_last);
            ips[0].q_operation = union_or_blocked_different_dirs(arrival_b, is_q_last);

            ips[0].is_pi = arrival_a == -1;
            ips[0].is_qi = arrival_b == -1;
            ips[0].is_pj = arrival_a == 1;
            ips[0].is_qj = arrival_b == 1;
        }
    }

    struct ip_info
    {
        inline ip_info()
            : p_operation(operation_none), q_operation(operation_none)
            , is_pi(false), is_pj(false), is_qi(false), is_qj(false)
        {}

        operation_type p_operation, q_operation;
        bool is_pi, is_pj, is_qi, is_qj;
    };

    template <std::size_t I>
    ip_info const& get() const
    {
        BOOST_STATIC_ASSERT(I < 2);
        return ips[I];
    }
    
private:

    // only if collinear (same_dirs)
    static inline operation_type union_or_blocked_same_dirs(int arrival, bool is_last)
    {
        if ( arrival == 1 )
            return operation_blocked;
        else if ( arrival == -1 )
            return operation_union;
        else
            return is_last ? operation_blocked : operation_union;
            //return operation_blocked;
    }

    // only if not collinear (!same_dirs)
    static inline operation_type union_or_blocked_different_dirs(int arrival, bool is_last)
    {
        if ( arrival == 1 )
            //return operation_blocked;
            return is_last ? operation_blocked : operation_union;
        else
            return operation_union;
    }

    ip_info ips[2];
};

template <typename AssignPolicy, bool EnableFirst, bool EnableLast>
struct get_turn_info_for_endpoint
{
    BOOST_STATIC_ASSERT(EnableFirst || EnableLast);

    template<typename Point1,
             typename Point2,
             typename TurnInfo,
             typename IntersectionInfo,
             typename OutputIterator
    >
    static inline bool apply(Point1 const& pi, Point1 const& pj, Point1 const& pk,
                             Point2 const& qi, Point2 const& qj, Point2 const& qk,
                             bool is_p_first, bool is_p_last,
                             bool is_q_first, bool is_q_last,
                             TurnInfo const& tp_model,
                             IntersectionInfo const& inters,
                             method_type /*method*/,
                             OutputIterator out)
    {
        std::size_t ip_count = inters.i_info().count;
        // no intersection points
        if ( ip_count == 0 )
            return false;

        int segment_index0 = tp_model.operations[0].seg_id.segment_index;
        int segment_index1 = tp_model.operations[1].seg_id.segment_index;
        BOOST_ASSERT(segment_index0 >= 0 && segment_index1 >= 0);

        if ( !is_p_first && !is_p_last && !is_q_first && !is_q_last )
            return false;

        linear_intersections intersections(pi, qi, inters.result(), is_p_last, is_q_last);

        bool append0_last
            = analyse_segment_and_assign_ip(pi, pj, pk, qi, qj, qk,
                                            is_p_first, is_p_last, is_q_first, is_q_last,
                                            intersections.template get<0>(),
                                            tp_model, inters, 0, out);

        // NOTE: opposite && ip_count == 1 may be true!
        bool opposite = inters.d_info().opposite;

        // don't ignore only for collinear opposite
        bool result_ignore_ip0 = append0_last && ( ip_count == 1 || !opposite );

        if ( intersections.template get<1>().p_operation == operation_none )
            return result_ignore_ip0;
        
        bool append1_last
            = analyse_segment_and_assign_ip(pi, pj, pk, qi, qj, qk,
                                            is_p_first, is_p_last, is_q_first, is_q_last,
                                            intersections.template get<1>(),
                                            tp_model, inters, 1, out);

        // don't ignore only for collinear opposite
        bool result_ignore_ip1 = append1_last && !opposite /*&& ip_count == 2*/;

        return result_ignore_ip0 || result_ignore_ip1;
    }

    template <typename Point1,
              typename Point2,
              typename TurnInfo,
              typename IntersectionInfo,
              typename OutputIterator>
    static inline
    bool analyse_segment_and_assign_ip(Point1 const& pi, Point1 const& pj, Point1 const& pk,
                                       Point2 const& qi, Point2 const& qj, Point2 const& qk,
                                       bool is_p_first, bool is_p_last,
                                       bool is_q_first, bool is_q_last,
                                       linear_intersections::ip_info const& ip_info,
                                       TurnInfo const& tp_model,
                                       IntersectionInfo const& inters,
                                       int ip_index,
                                       OutputIterator out)
    {
#ifdef BOOST_GEOMETRY_DEBUG_GET_TURNS_LINEAR_LINEAR
        // may this give false positives for INTs?
        typename IntersectionResult::point_type const&
            inters_pt = result.template get<0>().intersections[ip_index];
        BOOST_ASSERT(ip_info.is_pi == equals::equals_point_point(pi, inters_pt));
        BOOST_ASSERT(ip_info.is_qi == equals::equals_point_point(qi, inters_pt));
        BOOST_ASSERT(ip_info.is_pj == equals::equals_point_point(pj, inters_pt));
        BOOST_ASSERT(ip_info.is_qj == equals::equals_point_point(qj, inters_pt));
#endif

        // TODO - calculate first/last only if needed
        bool is_p_first_ip = is_p_first && ip_info.is_pi;
        bool is_p_last_ip = is_p_last && ip_info.is_pj;
        bool is_q_first_ip = is_q_first && ip_info.is_qi;
        bool is_q_last_ip = is_q_last && ip_info.is_qj;
        bool append_first = EnableFirst && (is_p_first_ip || is_q_first_ip);
        bool append_last = EnableLast && (is_p_last_ip || is_q_last_ip);

        operation_type p_operation = ip_info.p_operation;
        operation_type q_operation = ip_info.q_operation;

        if ( append_first || append_last )
        {
            bool handled = handle_internal<0>(pi, pj, pk, qi, qj, qk,
                                              is_p_first_ip, is_p_last_ip,
                                              is_q_first_ip, is_q_last_ip,
                                              ip_info.is_qi, ip_info.is_qj,
                                              tp_model, inters, ip_index,
                                              p_operation, q_operation);
            if ( !handled )
            {
                handle_internal<1>(qi, qj, qk, pi, pj, pk,
                                   is_q_first_ip, is_q_last_ip,
                                   is_p_first_ip, is_p_last_ip,
                                   ip_info.is_pi, ip_info.is_pj,
                                   tp_model, inters, ip_index,
                                   q_operation, p_operation);
            }

            if ( p_operation != operation_none )
            {
                method_type method = endpoint_ip_method(ip_info.is_pi, ip_info.is_pj,
                                                        ip_info.is_qi, ip_info.is_qj);
                turn_position p_pos = ip_position(is_p_first_ip, is_p_last_ip);
                turn_position q_pos = ip_position(is_q_first_ip, is_q_last_ip);

                // handle spikes

                // P is spike and should be handled
                if ( !is_p_last
                  && ip_info.is_pj // this check is redundant (also in is_spike_p) but faster
                  && inters.i_info().count == 2
                  && inters.is_spike_p() )
                {
                    assign(pi, qi, inters.result(), ip_index, method, operation_blocked, q_operation,
                           p_pos, q_pos, tp_model, out);
                    assign(pi, qi, inters.result(), ip_index, method, operation_intersection, q_operation,
                           p_pos, q_pos, tp_model, out);
                }
                // Q is spike and should be handled
                else if ( !is_q_last
                       && ip_info.is_qj // this check is redundant (also in is_spike_q) but faster
                       && inters.i_info().count == 2
                       && inters.is_spike_q() )
                {
                    assign(pi, qi, inters.result(), ip_index, method, p_operation, operation_blocked,
                           p_pos, q_pos, tp_model, out);
                    assign(pi, qi, inters.result(), ip_index, method, p_operation, operation_intersection,
                           p_pos, q_pos, tp_model, out);
                }
                // no spikes
                else
                {
                    assign(pi, qi, inters.result(), ip_index, method, p_operation, q_operation,
                           p_pos, q_pos, tp_model, out);
                }
            }
        }

        return append_last;
    }

    // TODO: IT'S ALSO PROBABLE THAT ALL THIS FUNCTION COULD BE INTEGRATED WITH handle_segment
    //       however now it's lazily calculated and then it would be always calculated

    template<std::size_t G1Index,
             typename Point1,
             typename Point2,
             typename TurnInfo,
             typename IntersectionInfo
    >
    static inline bool handle_internal(Point1 const& i1, Point1 const& j1, Point1 const& /*k1*/,
                                       Point2 const& i2, Point2 const& j2, Point2 const& k2,
                                       bool first1, bool last1, bool first2, bool last2,
                                       bool ip_i2, bool ip_j2, TurnInfo const& tp_model,
                                       IntersectionInfo const& inters, int ip_index,
                                       operation_type & op1, operation_type & op2)
    {
        boost::ignore_unused_variable_warning(ip_index);
        boost::ignore_unused_variable_warning(tp_model);

        if ( !first2 && !last2 )
        {
            if ( first1 )
            {
#ifdef BOOST_GEOMETRY_DEBUG_GET_TURNS_LINEAR_LINEAR
                // may this give false positives for INTs?
                typename IntersectionResult::point_type const&
                    inters_pt = inters.i_info().intersections[ip_index];
                BOOST_ASSERT(ip_i2 == equals::equals_point_point(i2, inters_pt));
                BOOST_ASSERT(ip_j2 == equals::equals_point_point(j2, inters_pt));
#endif
                if ( ip_i2 )
                {
                    // don't output this IP - for the first point of other geometry segment
                    op1 = operation_none;
                    op2 = operation_none;
                    return true;
                }
                else if ( ip_j2 )
                {
                    side_calculator<Point1, Point2, Point2> side_calc(i2, i1, j1, i2, j2, k2);

                    std::pair<operation_type, operation_type>
                        operations = operations_of_equal(side_calc);

// TODO: must the above be calculated?
// wouldn't it be enough to check if segments are collinear?

                    if ( operations_both(operations, operation_continue) )
                    {
                        if ( op1 != operation_union 
                          || op2 != operation_union
                          || ! ( G1Index == 0 ? inters.is_spike_q() : inters.is_spike_p() ) )
                        {
                            // THIS IS WRT THE ORIGINAL SEGMENTS! NOT THE ONES ABOVE!
                            bool opposite = inters.d_info().opposite;

                            op1 = operation_intersection;
                            op2 = opposite ? operation_union : operation_intersection;
                        }
                    }
                    else
                    {
                        BOOST_ASSERT(operations_combination(operations, operation_intersection, operation_union));
                        //op1 = operation_union;
                        //op2 = operation_union;
                    }

                    return true;
                }
                // else do nothing - shouldn't be handled this way
            }
            else if ( last1 )
            {
#ifdef BOOST_GEOMETRY_DEBUG_GET_TURNS_LINEAR_LINEAR
                // may this give false positives for INTs?
                typename IntersectionResult::point_type const&
                    inters_pt = inters.i_info().intersections[ip_index];
                BOOST_ASSERT(ip_i2 == equals::equals_point_point(i2, inters_pt));
                BOOST_ASSERT(ip_j2 == equals::equals_point_point(j2, inters_pt));
#endif
                if ( ip_i2 )
                {
                    // don't output this IP - for the first point of other geometry segment
                    op1 = operation_none;
                    op2 = operation_none;
                    return true;
                }
                else if ( ip_j2 )
                {
                    side_calculator<Point1, Point2, Point2> side_calc(i2, j1, i1, i2, j2, k2);
                    
                    std::pair<operation_type, operation_type>
                        operations = operations_of_equal(side_calc);

// TODO: must the above be calculated?
// wouldn't it be enough to check if segments are collinear?

                    if ( operations_both(operations, operation_continue) )
                    {
                        if ( op1 != operation_blocked
                          || op2 != operation_union
                          || ! ( G1Index == 0 ? inters.is_spike_q() : inters.is_spike_p() ) )
                        {
                            // THIS IS WRT THE ORIGINAL SEGMENTS! NOT THE ONES ABOVE!
                            bool second_going_out = inters.i_info().count > 1;

                            op1 = operation_blocked;
                            op2 = second_going_out ? operation_union : operation_intersection;
                        }
                    }
                    else
                    {
                        BOOST_ASSERT(operations_combination(operations, operation_intersection, operation_union));
                        //op1 = operation_blocked;
                        //op2 = operation_union;
                    }

                    return true;
                }
                // else do nothing - shouldn't be handled this way
            }
            // else do nothing - shouldn't be handled this way
        }

        return false;
    }

    static inline method_type endpoint_ip_method(bool ip_pi, bool ip_pj, bool ip_qi, bool ip_qj)
    {
        if ( (ip_pi || ip_pj) && (ip_qi || ip_qj) )
            return method_touch;
        else
            return method_touch_interior;
    }

    static inline turn_position ip_position(bool is_ip_first_i, bool is_ip_last_j)
    {
        return is_ip_first_i ? position_front :
               ( is_ip_last_j ? position_back : position_middle );
    }

    template <typename Point1,
              typename Point2,
              typename IntersectionResult,
              typename TurnInfo,
              typename OutputIterator>
    static inline void assign(Point1 const& pi, Point2 const& qi,
                              IntersectionResult const& result,
                              int ip_index,
                              method_type method,
                              operation_type op0, operation_type op1,
                              turn_position pos0, turn_position pos1,
                              TurnInfo const& tp_model,
                              OutputIterator out)
    {
        TurnInfo tp = tp_model;
        
        //geometry::convert(ip, tp.point);
        //tp.method = method;
        base_turn_handler::assign_point(tp, method, result.template get<0>(), ip_index);

        tp.operations[0].operation = op0;
        tp.operations[1].operation = op1;
        tp.operations[0].position = pos0;
        tp.operations[1].position = pos1;

        // NOTE: this probably shouldn't be set for the first point
        // for which there is no preceding segment
        if ( result.template get<0>().count > 1 )
        {
            //BOOST_ASSERT( result.template get<1>().dir_a == 0 && result.template get<1>().dir_b == 0 );
            tp.operations[0].is_collinear = true;
            tp.operations[1].is_collinear = true;
        }
        else //if ( result.template get<0>().count == 1 )
        {
            if ( op0 == operation_blocked && op1 == operation_intersection )
            {
                tp.operations[0].is_collinear = true;
            }
            else if ( op0 == operation_intersection && op1 == operation_blocked )
            {
                tp.operations[1].is_collinear = true;
            }
        }

        AssignPolicy::apply(tp, pi, qi, result.template get<0>(), result.template get<1>());
        *out++ = tp;
    }

    template <typename SidePolicy>
    static inline std::pair<operation_type, operation_type> operations_of_equal(SidePolicy const& side)
    {
        int const side_pk_q2 = side.pk_wrt_q2();
        int const side_pk_p = side.pk_wrt_p1();
        int const side_qk_p = side.qk_wrt_p1();

        // If pk is collinear with qj-qk, they continue collinearly.
        // This can be on either side of p1 (== q1), or collinear
        // The second condition checks if they do not continue
        // oppositely
        if ( side_pk_q2 == 0 && side_pk_p == side_qk_p )
        {
            return std::make_pair(operation_continue, operation_continue);
        }

        // If they turn to same side (not opposite sides)
        if ( ! base_turn_handler::opposite(side_pk_p, side_qk_p) )
        {
            int const side_pk_q2 = side.pk_wrt_q2();
            // If pk is left of q2 or collinear: p: union, q: intersection
            if ( side_pk_q2 != -1 )
            {
                return std::make_pair(operation_union, operation_intersection);
            }
            else
            {
               return std::make_pair(operation_intersection, operation_union);
            }
        }
        else
        {
            // They turn opposite sides. If p turns left (or collinear),
           // p: union, q: intersection
            if ( side_pk_p != -1 )
            {
                return std::make_pair(operation_union, operation_intersection);
            }
           else
            {
                return std::make_pair(operation_intersection, operation_union);
            }
        }
   }

    static inline bool operations_both(
                            std::pair<operation_type, operation_type> const& operations,
                            operation_type const op)
    {
        return operations.first == op && operations.second == op;
    }

    static inline bool operations_combination(
                            std::pair<operation_type, operation_type> const& operations,
                            operation_type const op1, operation_type const op2)
    {
        return ( operations.first == op1 && operations.second == op2 )
            || ( operations.first == op2 && operations.second == op1 );
    }
};

}} // namespace detail::overlay
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_GET_TURN_INFO_FOR_ENDPOINT_HPP
