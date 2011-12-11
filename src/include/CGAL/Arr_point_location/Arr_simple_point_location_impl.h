// Copyright (c) 2006,2007,2008,2009,2010,2011 Tel-Aviv University (Israel).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org); you may redistribute it under
// the terms of the Q Public License version 1.0.
// See the file LICENSE.QPL distributed with CGAL.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/branches/releases/CGAL-3.9-branch/Arrangement_on_surface_2/include/CGAL/Arr_point_location/Arr_simple_point_location_impl.h $
// $Id: Arr_simple_point_location_impl.h 64639 2011-07-06 11:41:06Z lrineau $
// 
//
// Author(s)     : Ron Wein   <wein@post.tau.ac.il>
//                 (based on old version by Eyal Flato)
#ifndef CGAL_ARR_SIMPLE_POINT_LOCATION_FUNCTIONS_H
#define CGAL_ARR_SIMPLE_POINT_LOCATION_FUNCTIONS_H

/*! \file
 * Member-function definitions for the Arr_simple_point_location<Arrangement>
 * class.
 */

namespace CGAL {

//-----------------------------------------------------------------------------
// Locate the arrangement feature containing the given point.
//
template <class Arrangement>
Object Arr_simple_point_location<Arrangement>::locate (const Point_2& p) const
{
  // Go over the arrangement vertices and check whether one of them equals
  // the query point.
  typename Traits_adaptor_2::Equal_2    equal = geom_traits->equal_2_object();
  typename Arrangement::Vertex_const_iterator  vit;
  Vertex_const_handle                          vh;

  for (vit = p_arr->vertices_begin(); vit != p_arr->vertices_end(); ++vit)
  {
    vh = vit;
    if (equal (p, vh->point()))
      return (CGAL::make_object (vh));
  }

  // Go over arrangement halfedges and check whether one of them contains
  // the query point in its interior.
  typename Traits_adaptor_2::Is_in_x_range_2  is_in_x_range = 
                                        geom_traits->is_in_x_range_2_object();
  typename Traits_adaptor_2::Compare_y_at_x_2 compare_y_at_x = 
                                        geom_traits->compare_y_at_x_2_object();
  typename Arrangement::Edge_const_iterator   eit;
  Halfedge_const_handle                       hh;

  for (eit = p_arr->edges_begin(); eit != p_arr->edges_end(); ++eit)
  {
    hh = eit;

    if (is_in_x_range (hh->curve(), p) &&
        compare_y_at_x (p, hh->curve()) == EQUAL)
    {
      return (CGAL::make_object (hh));
    }
  }

  // Shoot a vertical ray from the query point.
  // The ray shooting returns either a vertex of a halfedge (or an empty
  // object).
  Object                        obj = _base_vertical_ray_shoot (p, true);
  const Vertex_const_handle    *p_vh;
  const Halfedge_const_handle  *p_hh;
  Face_const_handle             fh;

  if (obj.is_empty())
  {
    // We should return the unbounded face.
    fh = Face_const_handle (top_traits->initial_face());
    return (CGAL::make_object (fh));
  }

  p_hh = object_cast<Halfedge_const_handle> (&obj);
  if (p_hh != NULL)
  {
    // Make sure that the edge is directed from right to left, so that p
    // (which lies below it) is contained in its incident face. If necessary,
    // we take the twin halfedge.
    if ((*p_hh)->direction() == ARR_LEFT_TO_RIGHT)
      hh = (*p_hh)->twin();
    else
      hh = *p_hh;

    // Return the incident face.
    fh = hh->face();
    return (CGAL::make_object (fh));
  }

  // In case the ray-shooting returned a vertex, we have to locate the first
  // halfedge whose source vertex is v, rotating clockwise around the vertex
  // from "6 o'clock", and to return its incident face. 
  p_vh = object_cast<Vertex_const_handle> (&obj);
  CGAL_assertion (p_vh != NULL);

  hh = _first_around_vertex (*p_vh);
  fh = hh->face();
  return (CGAL::make_object (fh));
}

//-----------------------------------------------------------------------------
// Locate the arrangement feature which a vertical ray emanating from the
// given point hits (not inculding isolated vertices).
//
template <class Arrangement>
Object Arr_simple_point_location<Arrangement>::_base_vertical_ray_shoot
    (const Point_2& p,
     bool shoot_up) const
{
  // Set the results for comparison according to the ray direction.
  const Comparison_result point_above_under = (shoot_up ? SMALLER : LARGER);
  const Comparison_result curve_above_under = (shoot_up ? LARGER : SMALLER);

  // Go over all halfedges in the arrangement.
  typename Traits_adaptor_2::Is_vertical_2        is_vertical =
                                  geom_traits->is_vertical_2_object();
  typename Traits_adaptor_2::Compare_y_position_2 compare_y_position =
                                  geom_traits->compare_y_position_2_object();
  typename Traits_adaptor_2::Compare_y_at_x_right_2  compare_y_at_x_right =
                                  geom_traits->compare_y_at_x_right_2_object();
  typename Traits_adaptor_2::Compare_y_at_x_left_2   compare_y_at_x_left =
                                  geom_traits->compare_y_at_x_left_2_object();

  typename Dcel::Edge_const_iterator  eit = 
                                        top_traits->dcel().edges_begin();
  typename Dcel::Edge_const_iterator  e_end =
                                        top_traits->dcel().edges_end();
  const typename Dcel::Halfedge      *he;       // The current edge.
  const typename Dcel::Vertex        *vs, *vt;  // Its source and target.
  Comparison_result                   res_s;
  Comparison_result                   res;
  Comparison_result                   y_res;
  bool                                in_x_range;
  const typename Dcel::Halfedge   *closest_he = NULL; // The closest so far.
  const typename Dcel::Vertex     *cl_vs = NULL,      // Its source and target.
                                  *cl_vt = NULL;

  while (eit != e_end)
  {
    // Get the current edge and its source and target vertices.
    he = &(*eit);
    vs = he->opposite()->vertex();
    vt = he->vertex();

    // Determine whether p is in the x-range of the curve and above or below it
    // (according to the direction of the shoot).
    res_s = top_traits->compare_x (p, vs);

    if (res_s == EQUAL)
      in_x_range = true;
    else if ((res_s == SMALLER && he->direction() == ARR_LEFT_TO_RIGHT) ||
             (res_s == LARGER && he->direction() == ARR_RIGHT_TO_LEFT))
      in_x_range = false;
    else
      in_x_range = (res_s != top_traits->compare_x (p, vt));

    if (in_x_range)
      res = top_traits->compare_y_at_x (p, he);

    if (in_x_range && res == point_above_under)
    {
      if (closest_he == NULL)
      {
        // If no other x-monotone curve containing p in its x-range has been
        // found yet, take the current one as the vertically closest to p.
        closest_he = he;
        cl_vs = vs;
        cl_vt = vt;
      }
      else
      {
        // Compare with the vertically closest curve so far and detemine the
        // curve closest to p. We first check the case that the two curves
        // have a common endpoint (note that the two curves do not intersect
        // in their interiors). Observe that if such a common vertex exists,
        // it is certainly not a vertex at infinity, therefore it is
        // associated with a valid point.
        if ((cl_vs == vs && closest_he->direction() == eit->direction()) ||
            (cl_vs == vt && closest_he->direction() != eit->direction()))
        {
          CGAL_assertion (! cl_vs->has_null_point());

          if (closest_he->direction() == ARR_LEFT_TO_RIGHT)
          {
            // Both curves extend to the right from a common point.
            y_res = compare_y_at_x_right (closest_he->curve(),
                                          eit->curve(), 
                                          cl_vs->point());
          }
          else
          {
            // Both curves extend to the left from a common point.
            y_res = compare_y_at_x_left (closest_he->curve(),
                                         eit->curve(), 
                                         cl_vs->point());
          }
        }
        else if ((cl_vt == vs &&
                  closest_he->direction() != eit->direction()) ||
                 (cl_vt == vt &&
                  closest_he->direction() == eit->direction()))
        {
          CGAL_assertion (! cl_vt->has_null_point());

          if (closest_he->direction() == ARR_LEFT_TO_RIGHT)
          {
            // Both curves extend to the left from a common point.
            y_res = compare_y_at_x_left (closest_he->curve(),
                                         eit->curve(), 
                                         cl_vt->point());
          }
          else
          {
            // Both curves extend to the right from a common point.
            y_res = compare_y_at_x_right (closest_he->curve(),
                                          eit->curve(), 
                                          cl_vt->point());
          }
        }
        else
        {
          // In case the two curves do not have a common endpoint, but overlap
          // in their x-range (both contain p), just compare their positions.
          // Note that in this case one of the edges may be fictitious, so we
          // preform the comparsion symbolically in this case.
          if (closest_he->has_null_curve())
            y_res = curve_above_under;
          else if (eit->has_null_curve())
            y_res = point_above_under;
          else
            y_res = compare_y_position (closest_he->curve(),
                                        eit->curve());
        }
 
        // If the current edge is closer to the query point than the closest
        // edge so far, update the closest edge.
        if (y_res == curve_above_under)
        {
          closest_he = he;
          cl_vs = vs;
          cl_vt = vt;
        }
      }
    }

    if (in_x_range && res == EQUAL &&
        ! eit->has_null_curve() && is_vertical(eit->curve()))
    {
      // Check if the query point is one of the end-vertices of the vertical
      // edge.
      Comparison_result  res1 = top_traits->compare_xy (p, vs);
      Comparison_result  res2 = top_traits->compare_xy (p, vt);

      if (! ((res1 == EQUAL && res2 == curve_above_under) ||
             (res1 == curve_above_under && res2 == EQUAL)))
      {
        // The vertical ray overlaps an existing vertical edge containing p.
        // In this case simply return this edge.
        closest_he = he;
        return (CGAL::make_object (Halfedge_const_handle (closest_he)));
      }
    }

    // Move to the next edge.
    ++eit;
  }

  // If we did not locate a closest halfedge, return an empty object.
  if (closest_he == NULL)
    return Object();

  // If we found a fictitious edge, return it now.
  if (closest_he->has_null_curve())
    return (CGAL::make_object (Halfedge_const_handle (closest_he)));

  // If one of the closest edge's end vertices has the same x-coordinate
  // as the query point, return this vertex.
  if (! is_vertical (closest_he->curve()))
  {
    if (! cl_vs->has_null_point() &&
        geom_traits->compare_x_2_object() (cl_vs->point(),
                                           p) == EQUAL)
    {
      return (CGAL::make_object (Vertex_const_handle (cl_vs)));
    }
    else if (! cl_vt->has_null_point() &&
             geom_traits->compare_x_2_object() (cl_vt->point(),
                                                p) == EQUAL)
    {
      return (CGAL::make_object (Vertex_const_handle (cl_vt)));
    }
  }
  else
  {
    CGAL_assertion_code(
      Comparison_result  res1 = top_traits->compare_xy (p, cl_vs);
      Comparison_result  res2 = top_traits->compare_xy (p, cl_vt);
      );

    CGAL_assertion (res1 == res2);
    CGAL_assertion (res1 == point_above_under);

    if ((shoot_up && closest_he->direction() == ARR_LEFT_TO_RIGHT) ||
        (! shoot_up && closest_he->direction() == ARR_RIGHT_TO_LEFT))
      return (CGAL::make_object (Vertex_const_handle (cl_vs)));
    else
      return (CGAL::make_object (Vertex_const_handle (cl_vt)));
  }

  // Otherwise, return the closest edge.
  return (CGAL::make_object (Halfedge_const_handle (closest_he)));
}

//-----------------------------------------------------------------------------
// Locate the arrangement feature which a vertical ray emanating from the
// given point hits, considering isolated vertices.
//
template <class Arrangement>
Object Arr_simple_point_location<Arrangement>::_vertical_ray_shoot
    (const Point_2& p,
     bool shoot_up) const
{
  // Locate the arrangement feature which a vertical ray emanating from the
  // given point hits, when not considering the isolated vertices.
  // This feature may not exist, or be either a vertex of a halfedge.
  Object                 obj = _base_vertical_ray_shoot (p, shoot_up);
  bool                   found_vertex;
  Vertex_const_handle    closest_v;
  Halfedge_const_handle  closest_he;
  Halfedge_const_handle  invalid_he;

  const Vertex_const_handle *p_vh = object_cast<Vertex_const_handle> (&obj);
  
  if (p_vh != NULL)
  {
    found_vertex = true;
    closest_v = *p_vh;
  }
  else
  {
    const Halfedge_const_handle *p_hh = 
      object_cast<Halfedge_const_handle> (&obj);

    found_vertex = false;

    if (p_hh != NULL)
      closest_he = *p_hh;
  }

  // Set the result for comparison according to the ray direction.
  const Comparison_result point_above_under = (shoot_up ? SMALLER : LARGER);

  // Go over all isolated vertices in the arrangement.
  typename Traits_adaptor_2::Compare_x_2          compare_x =
                                        geom_traits->compare_x_2_object();
  typename Traits_adaptor_2::Compare_xy_2         compare_xy =
                                        geom_traits->compare_xy_2_object();
  typename Traits_adaptor_2::Compare_y_at_x_2     compare_y_at_x =
                                        geom_traits->compare_y_at_x_2_object();

  typename Arrangement::Vertex_const_iterator  vit;
  Vertex_const_handle                          vh;

  for (vit = p_arr->vertices_begin(); vit != p_arr->vertices_end(); ++vit)
  {
    vh = vit;
    if (! vh->is_isolated())
      continue;

    // The current isolated vertex should have the same x-coordinate as the
    // query point in order to be below or above it.
    if (compare_x (p, vh->point()) != EQUAL)
      continue;

    // Make sure the isolated vertex is above the query point (if we shoot up)
    // or below it (if we shoot down).
    if (compare_xy (p, vh->point()) != point_above_under)
      continue;

    // Check if the isolated vertex is closer to p than the current closest
    // object.
    if ((found_vertex &&
         (closest_v->is_at_open_boundary() ||
          compare_xy (vh->point(), closest_v->point()) ==
                                                         point_above_under)) ||
        (! found_vertex &&
         (closest_he == invalid_he ||
          closest_he->is_fictitious() ||
          compare_y_at_x (vh->point(), closest_he->curve()) == 
                                                         point_above_under)))
    {
      found_vertex = true;
      closest_v = vh;
    }
  }

  // If we found a vertex, return it.
  if (found_vertex)
    return (CGAL::make_object (closest_v));

  // If we have no halfedge above, return the initial face.
  if (closest_he == invalid_he)
  {
    Face_const_handle  uf = Face_const_handle (top_traits->initial_face());

    return (CGAL::make_object (uf));
  }

  // If we found a valid edge, return it.
  if (! closest_he->is_fictitious())
    return (CGAL::make_object (closest_he));
  
  // If we found a fictitious edge, we have to return a handle to its
  // incident unbounded face.
  if ((shoot_up && closest_he->direction() == ARR_LEFT_TO_RIGHT) ||
      (!shoot_up && closest_he->direction() == ARR_RIGHT_TO_LEFT))
  {
    closest_he = closest_he->twin();
  }

  Face_const_handle  fh = closest_he->face();
  return (CGAL::make_object (fh));
}

//-----------------------------------------------------------------------------
// Find the first halfedge with a given source vertex, when going clockwise
// from "6 o'clock" around this vertex.
//
template <class Arrangement>
typename Arr_simple_point_location<Arrangement>::Halfedge_const_handle
Arr_simple_point_location<Arrangement>::_first_around_vertex
    (Vertex_const_handle v) const
{
  // Travrse the incident halfedges of the current vertex and locate the
  // lowest one to its left and the topmost to its right.
  typename Traits_adaptor_2::Compare_y_at_x_right_2 compare_y_at_x_right =
                                  geom_traits->compare_y_at_x_right_2_object();
  typename Traits_adaptor_2::Compare_y_at_x_left_2  compare_y_at_x_left =
                                  geom_traits->compare_y_at_x_left_2_object();

  const Halfedge_const_handle   invalid_handle;
  Halfedge_const_handle         lowest_left;
  Halfedge_const_handle         top_right;

  typename Arrangement::Halfedge_around_vertex_const_circulator  first = 
                                                      v->incident_halfedges();
  typename Arrangement::Halfedge_around_vertex_const_circulator  curr = first;

  do 
  {
    // Check whether the current halfedge is defined to the left or to the
    // right of the given vertex.
    if (curr->direction() == ARR_LEFT_TO_RIGHT)
    {
      // The curve associated with the current halfedge is defined to the left
      // of v.
      if (lowest_left == invalid_handle ||
          (! curr->is_fictitious() &&
           compare_y_at_x_left (curr->curve(),
                                lowest_left->curve(), 
                                v->point()) == SMALLER))
      {
        lowest_left = curr;
      }
    }
    else
    {
      // The curve associated with the current halfedge is defined to the right
      // of v.
      if (top_right == invalid_handle ||
          (! curr->is_fictitious() &&
           compare_y_at_x_right (curr->curve(),
                                 top_right->curve(), 
                                 v->point()) == LARGER))
      {
        top_right = curr;
      }
    }

    ++curr;
  } while (curr != first);

  // The first halfedge we encounter is the lowest to the left, but if there
  // is no edge to the left, we first encounter the topmost halfedge to the 
  // right. Note that as the halfedge we located has v as its target, we now
  // have to return its twin.
  if (lowest_left != invalid_handle)
    return (lowest_left->twin());
  else
    return (top_right->twin());
}

} //namespace CGAL

#endif
