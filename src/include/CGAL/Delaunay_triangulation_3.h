// Copyright (c) 1999-2004   INRIA Sophia-Antipolis (France).
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
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/branches/releases/CGAL-3.9-branch/Triangulation_3/include/CGAL/Delaunay_triangulation_3.h $
// $Id: Delaunay_triangulation_3.h 63930 2011-06-07 08:20:54Z sloriot $
//
//
// Author(s)     : Monique Teillaud <Monique.Teillaud@sophia.inria.fr>
//                 Sylvain Pion
//                 Andreas Fabri <Andreas.Fabri@sophia.inria.fr>

#ifndef CGAL_DELAUNAY_TRIANGULATION_3_H
#define CGAL_DELAUNAY_TRIANGULATION_3_H

#include <CGAL/basic.h>

#include <utility>
#include <vector>

#include <CGAL/Triangulation_3.h>
#include <CGAL/iterator.h>
#include <CGAL/Location_policy.h>

#ifndef CGAL_TRIANGULATION_3_DONT_INSERT_RANGE_OF_POINTS_WITH_INFO
#include <CGAL/Spatial_sort_traits_adapter_3.h>
#include <CGAL/internal/info_check.h>

#include <boost/tuple/tuple.hpp>
#include <boost/iterator/zip_iterator.hpp>
#include <boost/mpl/and.hpp>
#endif //CGAL_TRIANGULATION_3_DONT_INSERT_RANGE_OF_POINTS_WITH_INFO

#ifdef CGAL_DELAUNAY_3_OLD_REMOVE
#  error "The old remove() code has been removed.  Please report any issue you may have with the current one."
#endif

namespace CGAL {

template < class Gt,
           class Tds_ = Default,
           class Location_policy = Default >
class Delaunay_triangulation_3;


template < class Gt, class Tds_ >
class Delaunay_triangulation_3<Gt, Tds_>
  : public Triangulation_3<Gt, Tds_>
{
  typedef Delaunay_triangulation_3<Gt, Tds_> Self;
  typedef Triangulation_3<Gt,Tds_>           Tr_Base;

public:

  typedef typename Tr_Base::Triangulation_data_structure
                                     Triangulation_data_structure;
  typedef Gt                         Geom_traits;
  typedef Compact_location           Location_policy;

  typedef typename Gt::Point_3       Point;
  typedef typename Gt::Segment_3     Segment;
  typedef typename Gt::Triangle_3    Triangle;
  typedef typename Gt::Tetrahedron_3 Tetrahedron;

  // types for dual:
  typedef typename Gt::Line_3        Line;
  typedef typename Gt::Ray_3         Ray;
  //typedef typename Gt::Plane_3       Plane;
  typedef typename Gt::Object_3      Object;

  typedef typename Tr_Base::Cell_handle   Cell_handle;
  typedef typename Tr_Base::Vertex_handle Vertex_handle;

  typedef typename Tr_Base::Cell   Cell;
  typedef typename Tr_Base::Vertex Vertex;
  typedef typename Tr_Base::Facet  Facet;
  typedef typename Tr_Base::Edge   Edge;

  typedef typename Tr_Base::Cell_circulator  Cell_circulator;
  typedef typename Tr_Base::Facet_circulator Facet_circulator;
  typedef typename Tr_Base::Cell_iterator    Cell_iterator;
  typedef typename Tr_Base::Facet_iterator   Facet_iterator;
  typedef typename Tr_Base::Edge_iterator    Edge_iterator;
  typedef typename Tr_Base::Vertex_iterator  Vertex_iterator;

  typedef typename Tr_Base::Finite_vertices_iterator Finite_vertices_iterator;
  typedef typename Tr_Base::Finite_cells_iterator    Finite_cells_iterator;
  typedef typename Tr_Base::Finite_facets_iterator   Finite_facets_iterator;
  typedef typename Tr_Base::Finite_edges_iterator    Finite_edges_iterator;

  typedef typename Tr_Base::All_cells_iterator       All_cells_iterator;

  typedef typename Tr_Base::size_type size_type;
  typedef typename Tr_Base::Locate_type Locate_type;


#ifndef CGAL_CFG_USING_BASE_MEMBER_BUG_2
  using Tr_Base::cw;
  using Tr_Base::ccw;
  using Tr_Base::geom_traits;
  using Tr_Base::number_of_vertices;
  using Tr_Base::dimension;
  using Tr_Base::finite_facets_begin;
  using Tr_Base::finite_facets_end;
  using Tr_Base::finite_vertices_begin;
  using Tr_Base::finite_vertices_end;
  using Tr_Base::finite_cells_begin;
  using Tr_Base::finite_cells_end;
  using Tr_Base::finite_edges_begin;
  using Tr_Base::finite_edges_end;
  using Tr_Base::tds;
  using Tr_Base::infinite_vertex;
  using Tr_Base::next_around_edge;
  using Tr_Base::vertex_triple_index;
  using Tr_Base::mirror_vertex;
  using Tr_Base::coplanar;
  using Tr_Base::coplanar_orientation;
  using Tr_Base::orientation;
  using Tr_Base::adjacent_vertices;
  using Tr_Base::construct_segment;
  using Tr_Base::incident_facets;
  using Tr_Base::insert_in_conflict;
  using Tr_Base::is_infinite;
  using Tr_Base::is_valid_finite;
  using Tr_Base::locate;
  using Tr_Base::side_of_edge;
  using Tr_Base::side_of_segment;
  using Tr_Base::find_conflicts;
#endif

protected:

  Oriented_side
  side_of_oriented_sphere(const Point &p0, const Point &p1, const Point &p2,
	 const Point &p3, const Point &t, bool perturb = false) const;

  Bounded_side
  coplanar_side_of_bounded_circle(const Point &p, const Point &q,
		  const Point &r, const Point &s, bool perturb = false) const;

  // for dual:
  Point
  construct_circumcenter(const Point &p, const Point &q, const Point &r) const
  {
      return geom_traits().construct_circumcenter_3_object()(p, q, r);
  }

  Line
  construct_equidistant_line(const Point &p1, const Point &p2,
                             const Point &p3) const
  {
      return geom_traits().construct_equidistant_line_3_object()(p1, p2, p3);
  }

  Ray
  construct_ray(const Point &p, const Line &l) const
  {
      return geom_traits().construct_ray_3_object()(p, l);
  }

  Object
  construct_object(const Point &p) const
  {
      return geom_traits().construct_object_3_object()(p);
  }

  Object
  construct_object(const Segment &s) const
  {
      return geom_traits().construct_object_3_object()(s);
  }

  Object
  construct_object(const Ray &r) const
  {
      return geom_traits().construct_object_3_object()(r);
  }

  bool
  less_distance(const Point &p, const Point &q, const Point &r) const
  {
      return geom_traits().compare_distance_3_object()(p, q, r) == SMALLER;
  }

public:

  Delaunay_triangulation_3(const Gt& gt = Gt())
    : Tr_Base(gt)
  {}

  // copy constructor duplicates vertices and cells
  Delaunay_triangulation_3(const Delaunay_triangulation_3 & tr)
    : Tr_Base(tr)
  {
    CGAL_triangulation_postcondition( is_valid() );
  }

  template < typename InputIterator >
  Delaunay_triangulation_3(InputIterator first, InputIterator last,
                           const Gt& gt = Gt())
    : Tr_Base(gt)
  {
      insert(first, last);
  }

#ifndef CGAL_TRIANGULATION_3_DONT_INSERT_RANGE_OF_POINTS_WITH_INFO
  template < class InputIterator >
  std::ptrdiff_t
  insert( InputIterator first, InputIterator last,
          typename boost::enable_if<
            boost::is_base_of<
                Point,
                typename std::iterator_traits<InputIterator>::value_type
            >
          >::type* = NULL
  )
#else
  template < class InputIterator >
  std::ptrdiff_t
  insert( InputIterator first, InputIterator last)
#endif //CGAL_TRIANGULATION_3_DONT_INSERT_RANGE_OF_POINTS_WITH_INFO
  {
    size_type n = number_of_vertices();
    std::vector<Point> points (first, last);
    spatial_sort (points.begin(), points.end(), geom_traits());

    Vertex_handle hint;
    for (typename std::vector<Point>::const_iterator p = points.begin(), end = points.end();
            p != end; ++p)
        hint = insert(*p, hint);

    return number_of_vertices() - n;
  }
  
  
#ifndef CGAL_TRIANGULATION_3_DONT_INSERT_RANGE_OF_POINTS_WITH_INFO
private:  
  //top stands for tuple-or-pair
  template <class Info>
  const Point& top_get_first(const std::pair<Point,Info>& pair) const { return pair.first; }
  template <class Info>
  const Info& top_get_second(const std::pair<Point,Info>& pair) const { return pair.second; }
  template <class Info>
  const Point& top_get_first(const boost::tuple<Point,Info>& tuple) const { return boost::get<0>(tuple); }
  template <class Info>
  const Info& top_get_second(const boost::tuple<Point,Info>& tuple) const { return boost::get<1>(tuple); }

  template <class Tuple_or_pair,class InputIterator>
  std::ptrdiff_t insert_with_info(InputIterator first,InputIterator last)
  {
    size_type n = number_of_vertices();
    std::vector<std::ptrdiff_t> indices;
    std::vector<Point> points;
    std::vector<typename Triangulation_data_structure::Vertex::Info> infos;
    std::ptrdiff_t index=0;
    for (InputIterator it=first;it!=last;++it){
      Tuple_or_pair value=*it;
      points.push_back( top_get_first(value)  );
      infos.push_back ( top_get_second(value) );
      indices.push_back(index++);
    }

    typedef Spatial_sort_traits_adapter_3<Geom_traits,Point*> Search_traits;
    
    spatial_sort(indices.begin(),indices.end(),Search_traits(&(points[0]),geom_traits()));

    Vertex_handle hint;
    for (typename std::vector<std::ptrdiff_t>::const_iterator
      it = indices.begin(), end = indices.end();
      it != end; ++it){
      hint = insert(points[*it], hint);
      if (hint!=Vertex_handle()) hint->info()=infos[*it];
    }

    return number_of_vertices() - n;
  }
  
public:

  template < class InputIterator >
  std::ptrdiff_t
  insert( InputIterator first,
          InputIterator last,
          typename boost::enable_if<
            boost::is_same<
              typename std::iterator_traits<InputIterator>::value_type,
              std::pair<Point,typename internal::Info_check<typename Triangulation_data_structure::Vertex>::type>
            > >::type* =NULL
  )
  {
    return insert_with_info< std::pair<Point,typename internal::Info_check<typename Triangulation_data_structure::Vertex>::type> >(first,last);
  }

  template <class  InputIterator_1,class InputIterator_2>
  std::ptrdiff_t
  insert( boost::zip_iterator< boost::tuple<InputIterator_1,InputIterator_2> > first,
          boost::zip_iterator< boost::tuple<InputIterator_1,InputIterator_2> > last,
          typename boost::enable_if<
            boost::mpl::and_<
              boost::is_same< typename std::iterator_traits<InputIterator_1>::value_type, Point >,
              boost::is_same< typename std::iterator_traits<InputIterator_2>::value_type, typename internal::Info_check<typename Triangulation_data_structure::Vertex>::type >
            >
          >::type* =NULL
  )
  {
    return insert_with_info< boost::tuple<Point,typename internal::Info_check<typename Triangulation_data_structure::Vertex>::type> >(first,last);
  }
#endif //CGAL_TRIANGULATION_3_DONT_INSERT_RANGE_OF_POINTS_WITH_INFO
  
  Vertex_handle insert(const Point & p, Vertex_handle hint)
  {
    return insert(p, hint == Vertex_handle() ? this->infinite_cell() : hint->cell());
  }

  Vertex_handle insert(const Point & p, Cell_handle start = Cell_handle());

  Vertex_handle insert(const Point & p, Locate_type lt,
	               Cell_handle c, int li, int);
	
public: // internal methods
	
  template <class OutputItCells>
  Vertex_handle insert_and_give_new_cells(const Point  &p, 
                                          OutputItCells fit,
                                          Cell_handle start = Cell_handle() );
		
  template <class OutputItCells>
  Vertex_handle insert_and_give_new_cells(const Point& p,
                                          OutputItCells fit,
                                          Vertex_handle hint);

  template <class OutputItCells>
  Vertex_handle insert_and_give_new_cells(const Point& p,
                                          Locate_type lt,
                                          Cell_handle c, int li, int lj, 
                                          OutputItCells fit); 

public:	
	
#ifndef CGAL_NO_DEPRECATED_CODE
  CGAL_DEPRECATED Vertex_handle move_point(Vertex_handle v, const Point & p);
#endif  

  template <class OutputIteratorBoundaryFacets,
            class OutputIteratorCells,
            class OutputIteratorInternalFacets>
  Triple<OutputIteratorBoundaryFacets,
         OutputIteratorCells,
         OutputIteratorInternalFacets>
  find_conflicts(const Point &p, Cell_handle c,
	         OutputIteratorBoundaryFacets bfit,
                 OutputIteratorCells cit,
		 OutputIteratorInternalFacets ifit) const
  {
      CGAL_triangulation_precondition(dimension() >= 2);

      std::vector<Cell_handle> cells;
      cells.reserve(32);
      std::vector<Facet> facets;
      facets.reserve(64);

      if (dimension() == 2) {
          Conflict_tester_2 tester(p, this);
	  ifit = Tr_Base::find_conflicts
	    (c, tester,
	     make_triple(std::back_inserter(facets),
			 std::back_inserter(cells),
			 ifit)).third;
      }
      else {
          Conflict_tester_3 tester(p, this);
	  ifit = Tr_Base::find_conflicts
	    (c, tester,
	     make_triple(std::back_inserter(facets),
			 std::back_inserter(cells),
			 ifit)).third;
      }

      // Reset the conflict flag on the boundary.
      for(typename std::vector<Facet>::iterator fit=facets.begin();
          fit != facets.end(); ++fit) {
        fit->first->neighbor(fit->second)->tds_data().clear();
	*bfit++ = *fit;
      }

      // Reset the conflict flag in the conflict cells.
      for(typename std::vector<Cell_handle>::iterator ccit=cells.begin();
        ccit != cells.end(); ++ccit) {
        (*ccit)->tds_data().clear();
	*cit++ = *ccit;
      }
      return make_triple(bfit, cit, ifit);
  }

  template <class OutputIteratorBoundaryFacets, class OutputIteratorCells>
  std::pair<OutputIteratorBoundaryFacets, OutputIteratorCells>
  find_conflicts(const Point &p, Cell_handle c,
	         OutputIteratorBoundaryFacets bfit,
                 OutputIteratorCells cit) const
  {
      Triple<OutputIteratorBoundaryFacets,
             OutputIteratorCells,
	     Emptyset_iterator> t = find_conflicts(p, c, bfit, cit,
		                                   Emptyset_iterator());
      return std::make_pair(t.first, t.second);
  }

#ifndef CGAL_NO_DEPRECATED_CODE
  // Returns the vertices on the boundary of the conflict hole.
  template <class OutputIterator>
  OutputIterator
  vertices_in_conflict(const Point&p, Cell_handle c, OutputIterator res) const
  {
    return vertices_on_conflict_zone_boundary(p, c, res);
  }
#endif // CGAL_NO_DEPRECATED_CODE

  // Returns the vertices on the boundary of the conflict hole.
  template <class OutputIterator>
  OutputIterator
  vertices_on_conflict_zone_boundary(const Point&p, Cell_handle c,
                                     OutputIterator res) const
  {
      CGAL_triangulation_precondition(dimension() >= 2);

      // Get the facets on the boundary of the hole.
      std::vector<Facet> facets;
      find_conflicts(p, c, std::back_inserter(facets),
	             Emptyset_iterator(), Emptyset_iterator());

      // Then extract uniquely the vertices.
      std::set<Vertex_handle> vertices;
      if (dimension() == 3) {
          for (typename std::vector<Facet>::const_iterator i = facets.begin();
	       i != facets.end(); ++i) {
	      vertices.insert(i->first->vertex((i->second+1)&3));
	      vertices.insert(i->first->vertex((i->second+2)&3));
	      vertices.insert(i->first->vertex((i->second+3)&3));
          }
      } else {
          for (typename std::vector<Facet>::const_iterator i = facets.begin();
	       i != facets.end(); ++i) {
	      vertices.insert(i->first->vertex(cw(i->second)));
	      vertices.insert(i->first->vertex(ccw(i->second)));
          }
      }

      return std::copy(vertices.begin(), vertices.end(), res);
  }

  // REMOVE
  void remove(Vertex_handle v);

  // return new cells (internal)
  template <class OutputItCells>
  void remove_and_give_new_cells(Vertex_handle v, 
                                 OutputItCells fit);

  template < typename InputIterator >
  size_type remove(InputIterator first, InputIterator beyond)
  {
    CGAL_triangulation_precondition(!this->does_repeat_in_range(first, beyond));
    size_type n = number_of_vertices();
    while (first != beyond) {
      remove(*first);
      ++first;
    }
    return n - number_of_vertices();
  }
	
  template < typename InputIterator >
  size_type remove_cluster(InputIterator first, InputIterator beyond)
  {
    Self tmp;
    Vertex_remover<Self> remover (tmp);
    return Tr_Base::remove(first, beyond, remover);
  }

  // MOVE
  Vertex_handle move_if_no_collision(Vertex_handle v, const Point &p);

  Vertex_handle move(Vertex_handle v, const Point &p);

  // return new cells (internal)
  template <class OutputItCells>
  Vertex_handle move_if_no_collision_and_give_new_cells(Vertex_handle v, 
                                                        const Point &p, 
                                                        OutputItCells fit);

private:

  Bounded_side
  side_of_sphere(Vertex_handle v0, Vertex_handle v1,
		 Vertex_handle v2, Vertex_handle v3,
		 const Point &p, bool perturb) const;
public:

  // Queries
  Bounded_side
  side_of_sphere(Cell_handle c, const Point & p,
	         bool perturb = false) const
  {
      return side_of_sphere(c->vertex(0), c->vertex(1),
                            c->vertex(2), c->vertex(3), p, perturb);
  }

  Bounded_side
  side_of_circle( const Facet & f, const Point & p, bool perturb = false) const
  {
      return side_of_circle(f.first, f.second, p, perturb);
  }

  Bounded_side
  side_of_circle( Cell_handle c, int i, const Point & p,
	          bool perturb = false) const;

  Vertex_handle
  nearest_vertex_in_cell(const Point& p, Cell_handle c) const;

  Vertex_handle
  nearest_vertex(const Point& p, Cell_handle c = Cell_handle()) const;

  bool is_Gabriel(Cell_handle c, int i) const;
  bool is_Gabriel(Cell_handle c, int i, int j) const;
  bool is_Gabriel(const Facet& f)const ;
  bool is_Gabriel(const Edge& e) const;

  bool is_delaunay_after_displacement(Vertex_handle v, 
                                      const Point &p) const;

// Dual functions
  Point dual(Cell_handle c) const;

  Object dual(const Facet & f) const
  { return dual( f.first, f.second ); }

  Object dual(Cell_handle c, int i) const;

  Line dual_support(Cell_handle c, int i) const;

  bool is_valid(bool verbose = false, int level = 0) const;

  bool is_valid(Cell_handle c, bool verbose = false, int level = 0) const;

  template < class Stream> 		
  Stream& draw_dual(Stream & os)
    {
      for (Finite_facets_iterator fit = finite_facets_begin(),
                                  end = finite_facets_end();
           fit != end; ++fit) {
	Object o = dual(*fit);
	if      (const Segment *s = object_cast<Segment>(&o)) os << *s;
	else if (const Ray *r     = object_cast<Ray>(&o))     os << *r;
	else if (const Point *p   = object_cast<Point>(&o))   os << *p;
      }
      return os;
    }

protected:

  Vertex_handle
  nearest_vertex(const Point &p, Vertex_handle v, Vertex_handle w) const
  {
      // In case of equality, v is returned.
      CGAL_triangulation_precondition(v != w);

      if (is_infinite(v))
	  return w;
      if (is_infinite(w))
	  return v;
      return less_distance(p, w->point(), v->point()) ? w : v;
  }

  class Conflict_tester_3
  {
      const Point &p;
      const Self *t;

  public:

    Conflict_tester_3(const Point &pt, const Self *tr)
      : p(pt), t(tr) {}

    bool operator()(const Cell_handle c) const
    {
      return t->side_of_sphere(c, p, true) == ON_BOUNDED_SIDE;
    }
    Oriented_side compare_weight(const Point &, const Point &) const
    {
      return ZERO;
    }
    bool test_initial_cell(Cell_handle) const
    {
      return true;
    }
  };

  class Conflict_tester_2
  {
      const Point &p;
      const Self *t;

  public:

    Conflict_tester_2(const Point &pt, const Self *tr)
      : p(pt), t(tr) {}

    bool operator()(const Cell_handle c) const
    {
      return t->side_of_circle(c, 3, p, true) == ON_BOUNDED_SIDE;
    }
    Oriented_side compare_weight(const Point &, const Point &) const
    {
      return ZERO;
    }
    bool test_initial_cell(Cell_handle) const
    {
      return true;
    }
  };
  class Hidden_point_visitor
  {
  public:

    Hidden_point_visitor() {}

    template <class InputIterator>
    void process_cells_in_conflict(InputIterator, InputIterator) const {}
    void reinsert_vertices(Vertex_handle ) {}
    Vertex_handle replace_vertex(Cell_handle c, int index,
				 const Point &) {
      return c->vertex(index);
    }
    void hide_point(Cell_handle, const Point &) {}
  };

  class Perturbation_order {
      const Self *t;

  public:
      Perturbation_order(const Self *tr)
	  : t(tr) {}

      bool operator()(const Point *p, const Point *q) const {
	  return t->compare_xyz(*p, *q) == SMALLER;
      }
  };

  template < class DelaunayTriangulation_3 >
  class Vertex_remover;

  template < class DelaunayTriangulation_3 >
  class Vertex_inserter;

  friend class Perturbation_order;
  friend class Conflict_tester_3;
  friend class Conflict_tester_2;

  Hidden_point_visitor hidden_point_visitor;
};

template < class Gt, class Tds >
typename Delaunay_triangulation_3<Gt,Tds>::Vertex_handle
Delaunay_triangulation_3<Gt,Tds>::
insert(const Point & p, Cell_handle start)
{
    Locate_type lt;
    int li, lj;
    Cell_handle c = locate(p, lt, li, lj, start);
    return insert(p, lt, c, li, lj);
}

template < class Gt, class Tds >
typename Delaunay_triangulation_3<Gt,Tds>::Vertex_handle
Delaunay_triangulation_3<Gt,Tds>::
insert(const Point & p, Locate_type lt, Cell_handle c, int li, int lj)
{
  switch (dimension()) {
  case 3:
    {
      Conflict_tester_3 tester(p, this);
      Vertex_handle v = insert_in_conflict(p, lt, c, li, lj,
					   tester, hidden_point_visitor);
      return v;
    }// dim 3
  case 2:
    {
      Conflict_tester_2 tester(p, this);
      return insert_in_conflict(p, lt, c, li, lj,
				tester, hidden_point_visitor);
    }//dim 2
  default :
    // dimension <= 1
    // Do not use the generic insert.
    return Tr_Base::insert(p, c);
  }
}

template < class Gt, class Tds >
template <class OutputItCells>
typename Delaunay_triangulation_3<Gt,Tds>::Vertex_handle 
Delaunay_triangulation_3<Gt,Tds>::
insert_and_give_new_cells(const Point  &p, 
                          OutputItCells fit,
                          Cell_handle start)
{
  Vertex_handle v = insert(p, start);
  int dimension = this->dimension();
  if(dimension == 3) this->incident_cells(v, fit);
  else if(dimension == 2)
  {
    Cell_handle c = v->cell(), end = c;
    do {
      *fit++ = c;
      int i = c->index(v);
      c = c->neighbor((i+1)%3);
    } while(c != end);			
  } 
  else if(dimension == 1)
  {
    Cell_handle c = v->cell();
    *fit++ = c;
    *fit++ = c->neighbor((~(c->index(v)))&1);
  }
  else *fit++ = v->cell(); // dimension = 0
  return v;		
}

template < class Gt, class Tds >	
template <class OutputItCells>
typename Delaunay_triangulation_3<Gt,Tds>::Vertex_handle 
Delaunay_triangulation_3<Gt,Tds>::
insert_and_give_new_cells(const Point& p,
                          OutputItCells fit,
                          Vertex_handle hint)
{
  Vertex_handle v = insert(p, hint);
  int dimension = this->dimension();
  if(dimension == 3) this->incident_cells(v, fit);
  else if(dimension == 2)
  {
    Cell_handle c = v->cell(), end = c;
    do {
      *fit++ = c;
      int i = c->index(v);
      c = c->neighbor((i+1)%3);
    } while(c != end);			
  } 
  else if(dimension == 1)
  {
    Cell_handle c = v->cell();
    *fit++ = c;
    *fit++ = c->neighbor((~(c->index(v)))&1);
  }
  else *fit++ = v->cell(); // dimension = 0
  return v;
}

template < class Gt, class Tds >
template <class OutputItCells>
typename Delaunay_triangulation_3<Gt,Tds>::Vertex_handle 
Delaunay_triangulation_3<Gt,Tds>::
insert_and_give_new_cells(const Point& p,
                          Locate_type lt,
                          Cell_handle c, int li, int lj, 
                          OutputItCells fit)
{
  Vertex_handle v = insert(p, lt, c, li, lj);
  int dimension = this->dimension();
  if(dimension == 3) this->incident_cells(v, fit);
  else if(dimension == 2)
  {
    Cell_handle c = v->cell(), end = c;
    do {
      *fit++ = c;
      int i = c->index(v);
      c = c->neighbor((i+1)%3);
    } while(c != end);			
  } 
  else if(dimension == 1)
  {
    Cell_handle c = v->cell();
    *fit++ = c;
    *fit++ = c->neighbor((~(c->index(v)))&1);
  }
  else *fit++ = v->cell(); // dimension = 0		 
  return v;
}

#ifndef CGAL_NO_DEPRECATED_CODE
template < class Gt, class Tds >
typename Delaunay_triangulation_3<Gt,Tds>::Vertex_handle
Delaunay_triangulation_3<Gt,Tds>::
move_point(Vertex_handle v, const Point & p)
{
    CGAL_triangulation_precondition(! is_infinite(v));
    CGAL_triangulation_expensive_precondition(is_vertex(v));

    // Dummy implementation for a start.

    // Remember an incident vertex to restart
    // the point location after the removal.
    Cell_handle c = v->cell();
    Vertex_handle old_neighbor = c->vertex(c->index(v) == 0 ? 1 : 0);
    CGAL_triangulation_assertion(old_neighbor != v);

    remove(v);

    if (dimension() <= 0)
	return insert(p);
    return insert(p, old_neighbor->cell());
}
#endif

template <class Gt, class Tds >
template <class DelaunayTriangulation_3>
class Delaunay_triangulation_3<Gt, Tds>::Vertex_remover {
  typedef DelaunayTriangulation_3 Delaunay;
public:
  typedef Nullptr_t Hidden_points_iterator;

  Vertex_remover(Delaunay &tmp_) : tmp(tmp_) {}

  Delaunay &tmp;

  void add_hidden_points(Cell_handle) {}
  Hidden_points_iterator hidden_points_begin() { return NULL; }
  Hidden_points_iterator hidden_points_end() { return NULL; }

  Bounded_side side_of_bounded_circle(const Point &p, const Point &q,
    const Point &r, const Point &s, bool perturb = false) const {
    return tmp.coplanar_side_of_bounded_circle(p,q,r,s,perturb);
  }
};

template <class Gt, class Tds >
template <class DelaunayTriangulation_3>
class Delaunay_triangulation_3<Gt, Tds>::Vertex_inserter {
  typedef DelaunayTriangulation_3 Delaunay;
public:
  typedef Nullptr_t Hidden_points_iterator;

  Vertex_inserter(Delaunay &tmp_) : tmp(tmp_) {}

  Delaunay &tmp;

  void add_hidden_points(Cell_handle) {}
  Hidden_points_iterator hidden_points_begin() { return NULL; }
  Hidden_points_iterator hidden_points_end() { return NULL; }

  Vertex_handle insert(const Point& p,
		       Locate_type lt, Cell_handle c, int li, int lj) {
    return tmp.insert(p, lt, c, li, lj);
  }

  Vertex_handle insert(const Point& p, Cell_handle c) {
    return tmp.insert(p, c);
  }

  Vertex_handle insert(const Point& p) {
    return tmp.insert(p);
  }
};

template < class Gt, class Tds >
void
Delaunay_triangulation_3<Gt,Tds>::
remove(Vertex_handle v)
{
  Self tmp;
  Vertex_remover<Self> remover (tmp);
  Tr_Base::remove(v,remover);

  CGAL_triangulation_expensive_postcondition(is_valid());
}

template < class Gt, class Tds >
typename Delaunay_triangulation_3<Gt,Tds>::Vertex_handle
Delaunay_triangulation_3<Gt,Tds>::
move_if_no_collision(Vertex_handle v, const Point &p)
{
  Self tmp;
  Vertex_remover<Self> remover (tmp);
  Vertex_inserter<Self> inserter (*this);
  Vertex_handle res = Tr_Base::move_if_no_collision(v,p,remover,inserter);

  CGAL_triangulation_expensive_postcondition(is_valid());	
	return res;
}

template <class Gt, class Tds >
typename Delaunay_triangulation_3<Gt,Tds>::Vertex_handle
Delaunay_triangulation_3<Gt,Tds>::
move(Vertex_handle v, const Point &p) {
  CGAL_triangulation_precondition(!is_infinite(v));
  if(v->point() == p) return v;
  Self tmp;
  Vertex_remover<Self> remover (tmp);
  Vertex_inserter<Self> inserter (*this);
	return Tr_Base::move(v,p,remover,inserter);
}

template < class Gt, class Tds >
template <class OutputItCells>
void
Delaunay_triangulation_3<Gt,Tds>::
remove_and_give_new_cells(Vertex_handle v, OutputItCells fit)
{
  Self tmp;
  Vertex_remover<Self> remover (tmp);
  Tr_Base::remove_and_give_new_cells(v,remover,fit);

  CGAL_triangulation_expensive_postcondition(is_valid());
}

template < class Gt, class Tds >
template <class OutputItCells>
typename Delaunay_triangulation_3<Gt,Tds>::Vertex_handle
Delaunay_triangulation_3<Gt,Tds>::
move_if_no_collision_and_give_new_cells(Vertex_handle v, const Point &p,
  OutputItCells fit)
{
  Self tmp;
  Vertex_remover<Self> remover (tmp);
  Vertex_inserter<Self> inserter (*this);
  Vertex_handle res = 
    Tr_Base::move_if_no_collision_and_give_new_cells(v,p,
      remover,inserter,fit);

  CGAL_triangulation_expensive_postcondition(is_valid());	
	return res;
}

template < class Gt, class Tds >
Oriented_side
Delaunay_triangulation_3<Gt,Tds>::
side_of_oriented_sphere(const Point &p0, const Point &p1, const Point &p2,
	                const Point &p3, const Point &p, bool perturb) const
{
    CGAL_triangulation_precondition( orientation(p0, p1, p2, p3) == POSITIVE );

    Oriented_side os =
	geom_traits().side_of_oriented_sphere_3_object()(p0, p1, p2, p3, p);

    if (os != ON_ORIENTED_BOUNDARY || !perturb)
	return os;

    // We are now in a degenerate case => we do a symbolic perturbation.

    // We sort the points lexicographically.
    const Point * points[5] = {&p0, &p1, &p2, &p3, &p};
    std::sort(points, points+5, Perturbation_order(this) );

    // We successively look whether the leading monomial, then 2nd monomial
    // of the determinant has non null coefficient.
    // 2 iterations are enough (cf paper)
    for (int i=4; i>2; --i) {
        if (points[i] == &p)
            return ON_NEGATIVE_SIDE; // since p0 p1 p2 p3 are non coplanar
	                             // and positively oriented
        Orientation o;
        if (points[i] == &p3 && (o = orientation(p0,p1,p2,p)) != COPLANAR )
            return o;
        if (points[i] == &p2 && (o = orientation(p0,p1,p,p3)) != COPLANAR )
            return o;
        if (points[i] == &p1 && (o = orientation(p0,p,p2,p3)) != COPLANAR )
            return o;
        if (points[i] == &p0 && (o = orientation(p,p1,p2,p3)) != COPLANAR )
            return o;
    }

    CGAL_triangulation_assertion(false);
    return ON_NEGATIVE_SIDE;
}

template < class Gt, class Tds >
Bounded_side
Delaunay_triangulation_3<Gt,Tds>::
coplanar_side_of_bounded_circle(const Point &p0, const Point &p1,
	       const Point &p2, const Point &p, bool perturb) const
{
    // In dim==2, we should even be able to assert orient == POSITIVE.
    CGAL_triangulation_precondition( coplanar_orientation(p0, p1, p2)
	                             != COLLINEAR );

    Bounded_side bs =
      geom_traits().coplanar_side_of_bounded_circle_3_object()(p0, p1, p2, p);

    if (bs != ON_BOUNDARY || !perturb)
	return bs;

    // We are now in a degenerate case => we do a symbolic perturbation.

    // We sort the points lexicographically.
    const Point * points[4] = {&p0, &p1, &p2, &p};
    std::sort(points, points+4, Perturbation_order(this) );

    Orientation local = coplanar_orientation(p0, p1, p2);

    // we successively look whether the leading monomial, then 2nd monimial,
    // then 3rd monomial, of the determinant which has non null coefficient
    // [syl] : TODO : Probably it can be stopped earlier like the 3D version
    for (int i=3; i>0; --i) {
        if (points[i] == &p)
            return Bounded_side(NEGATIVE); // since p0 p1 p2 are non collinear
	                           // but not necessarily positively oriented
        Orientation o;
        if (points[i] == &p2
		&& (o = coplanar_orientation(p0,p1,p)) != COLLINEAR )
	    // [syl] : TODO : I'm not sure of the signs here (nor the rest :)
            return Bounded_side(o*local);
        if (points[i] == &p1
		&& (o = coplanar_orientation(p0,p,p2)) != COLLINEAR )
            return Bounded_side(o*local);
        if (points[i] == &p0
		&& (o = coplanar_orientation(p,p1,p2)) != COLLINEAR )
            return Bounded_side(o*local);
    }

    // case when the first non null coefficient is the coefficient of
    // the 4th monomial
    // moreover, the tests (points[] == &p) were false up to here, so the
    // monomial corresponding to p is the only monomial with non-zero
    // coefficient, it is equal to coplanar_orient(p0,p1,p2) == positive
    // so, no further test is required
    return Bounded_side(-local); //ON_UNBOUNDED_SIDE;
}

template < class Gt, class Tds >
Bounded_side
Delaunay_triangulation_3<Gt,Tds>::
side_of_sphere(Vertex_handle v0, Vertex_handle v1,
	       Vertex_handle v2, Vertex_handle v3,
	       const Point &p, bool perturb) const
{
    CGAL_triangulation_precondition( dimension() == 3 );

    if (is_infinite(v0)) {
	Orientation o = orientation(v2->point(), v1->point(), v3->point(), p);
	if (o != COPLANAR)
	    return Bounded_side(o);
	return coplanar_side_of_bounded_circle(v2->point(), v1->point(), v3->point(), p, perturb);
    }

    if (is_infinite(v1)) {
	Orientation o = orientation(v2->point(), v3->point(), v0->point(), p);
	if (o != COPLANAR)
	    return Bounded_side(o);
	return coplanar_side_of_bounded_circle(v2->point(), v3->point(), v0->point(), p, perturb);
    }

    if (is_infinite(v2)) {
	Orientation o = orientation(v1->point(), v0->point(), v3->point(), p);
	if (o != COPLANAR)
	    return Bounded_side(o);
	return coplanar_side_of_bounded_circle(v1->point(), v0->point(), v3->point(), p, perturb);
    }

    if (is_infinite(v3)) {
	Orientation o = orientation(v0->point(), v1->point(), v2->point(), p);
	if (o != COPLANAR)
	    return Bounded_side(o);
	return coplanar_side_of_bounded_circle(v0->point(), v1->point(), v2->point(), p, perturb);
    }

    return (Bounded_side) side_of_oriented_sphere(v0->point(), v1->point(), v2->point(), v3->point(), p, perturb);
}

template < class Gt, class Tds >
Bounded_side
Delaunay_triangulation_3<Gt,Tds>::
side_of_circle(Cell_handle c, int i,
	       const Point & p, bool perturb) const
  // precondition : dimension >=2
  // in dimension 3, - for a finite facet
  // returns ON_BOUNDARY if the point lies on the circle,
  // ON_UNBOUNDED_SIDE when exterior, ON_BOUNDED_SIDE
  // interior
  // for an infinite facet, considers the plane defined by the
  // adjacent finite facet of the same cell, and does the same as in
  // dimension 2 in this plane
  // in dimension 2, for an infinite facet
  // in this case, returns ON_BOUNDARY if the point lies on the
  // finite edge (endpoints included)
  // ON_BOUNDED_SIDE for a point in the open half-plane
  // ON_UNBOUNDED_SIDE elsewhere
{
  CGAL_triangulation_precondition( dimension() >= 2 );
  int i3 = 5;

  if ( dimension() == 2 ) {
    CGAL_triangulation_precondition( i == 3 );
    // the triangulation is supposed to be valid, ie the facet
    // with vertices 0 1 2 in this order is positively oriented
    if ( ! c->has_vertex( infinite_vertex(), i3 ) )
      return coplanar_side_of_bounded_circle( c->vertex(0)->point(),
					      c->vertex(1)->point(),
					      c->vertex(2)->point(),
					      p, perturb);
    // else infinite facet
    // v1, v2 finite vertices of the facet such that v1,v2,infinite
    // is positively oriented
    Vertex_handle v1 = c->vertex( ccw(i3) ),
                  v2 = c->vertex( cw(i3) );
    CGAL_triangulation_assertion(coplanar_orientation(v1->point(), v2->point(),
			         mirror_vertex(c, i3)->point()) == NEGATIVE);
    Orientation o = coplanar_orientation(v1->point(), v2->point(), p);
    if ( o != COLLINEAR )
	return Bounded_side( o );
    // because p is in f iff
    // it does not lie on the same side of v1v2 as vn
    int i_e;
    Locate_type lt;
    // case when p collinear with v1v2
    return side_of_segment( p,
			    v1->point(), v2->point(),
			    lt, i_e );
  }

  // else dimension == 3
  CGAL_triangulation_precondition( i >= 0 && i < 4 );
  if ( ( ! c->has_vertex(infinite_vertex(),i3) ) || ( i3 != i ) ) {
    // finite facet
    // initialization of i0 i1 i2, vertices of the facet positively
    // oriented (if the triangulation is valid)
    int i0 = (i>0) ? 0 : 1;
    int i1 = (i>1) ? 1 : 2;
    int i2 = (i>2) ? 2 : 3;
    CGAL_triangulation_precondition( coplanar( c->vertex(i0)->point(),
				               c->vertex(i1)->point(),
				               c->vertex(i2)->point(),
					       p ) );
    return coplanar_side_of_bounded_circle( c->vertex(i0)->point(),
					    c->vertex(i1)->point(),
					    c->vertex(i2)->point(),
					    p, perturb);
  }

  //else infinite facet
  // v1, v2 finite vertices of the facet such that v1,v2,infinite
  // is positively oriented
  Vertex_handle v1 = c->vertex( next_around_edge(i3,i) ),
                v2 = c->vertex( next_around_edge(i,i3) );
  Orientation o = (Orientation)
                  (coplanar_orientation( v1->point(), v2->point(),
			                 c->vertex(i)->point()) *
                  coplanar_orientation( v1->point(), v2->point(), p ));
  // then the code is duplicated from 2d case
  if ( o != COLLINEAR )
      return Bounded_side( -o );
  // because p is in f iff
  // it is not on the same side of v1v2 as c->vertex(i)
  int i_e;
  Locate_type lt;
  // case when p collinear with v1v2
  return side_of_segment( p,
			  v1->point(), v2->point(),
			  lt, i_e );
}

template < class Gt, class Tds >
typename Delaunay_triangulation_3<Gt,Tds>::Vertex_handle
Delaunay_triangulation_3<Gt,Tds>::
nearest_vertex_in_cell(const Point& p, Cell_handle c) const
// Returns the finite vertex of the cell c which is the closest to p.
{
    CGAL_triangulation_precondition(dimension() >= 0);

    Vertex_handle nearest = nearest_vertex(p, c->vertex(0), c->vertex(1));
    if (dimension() >= 2) {
	nearest = nearest_vertex(p, nearest, c->vertex(2));
        if (dimension() == 3)
	    nearest = nearest_vertex(p, nearest, c->vertex(3));
    }
    return nearest;
}

template < class Gt, class Tds >
typename Delaunay_triangulation_3<Gt,Tds>::Vertex_handle
Delaunay_triangulation_3<Gt,Tds>::
nearest_vertex(const Point& p, Cell_handle start) const
{
    if (number_of_vertices() == 0)
	return Vertex_handle();

    // Use a brute-force algorithm if dimension < 3.
    if (dimension() < 3) {
	Finite_vertices_iterator vit = finite_vertices_begin();
	Vertex_handle res = vit;
	++vit;
	for (Finite_vertices_iterator end = finite_vertices_end(); vit != end; ++vit)
	    res = nearest_vertex(p, res, vit);
	return res;
    }

    Locate_type lt;
    int li, lj;
    Cell_handle c = locate(p, lt, li, lj, start);
    if (lt == Tr_Base::VERTEX)
	return c->vertex(li);

    // - start with the closest vertex from the located cell.
    // - repeatedly take the nearest of its incident vertices if any
    // - if not, we're done.
    Vertex_handle nearest = nearest_vertex_in_cell(p, c);
    std::vector<Vertex_handle> vs;
    vs.reserve(32);
    while (true) {
	Vertex_handle tmp = nearest;
        adjacent_vertices(nearest, std::back_inserter(vs));
        for (typename std::vector<Vertex_handle>::const_iterator
		vsit = vs.begin(); vsit != vs.end(); ++vsit)
	    tmp = nearest_vertex(p, tmp, *vsit);
	if (tmp == nearest)
	    break;
	vs.clear();
	nearest = tmp;
    }

    return nearest;
}

// This is not a fast version.
// The optimized version needs an int for book-keeping in 
// tds() so as to avoiding the need to clear
// the tds marker in each cell (which is an unsigned char)
// Also the visitor in TDS could be more clever.
// The Delaunay triangulation which filters displacements
// will do these optimizations. 
template <class Gt, class Tds >
bool 
Delaunay_triangulation_3<Gt,Tds>::
is_delaunay_after_displacement(Vertex_handle v, const Point &p) const
{
  CGAL_triangulation_precondition(!this->is_infinite(v));	
  CGAL_triangulation_precondition(this->dimension() == 2);	
  CGAL_triangulation_precondition(!this->test_dim_down(v));
	if(v->point() == p) return true;
  Point ant = v->point();
  v->set_point(p);

  std::size_t size;

  // are incident cells well-oriented
  std::vector<Cell_handle> cells;
  cells.reserve(64);
  this->incident_cells(v, std::back_inserter(cells));
  size = cells.size();
  for(std::size_t i=0; i<size; i++)
  {
    Cell_handle c = cells[i];
    if(this->is_infinite(c)) continue;
    if(this->orientation(c->vertex(0)->point(), c->vertex(1)->point(), 
                         c->vertex(2)->point(), c->vertex(3)->point()) 
       != POSITIVE)
    {
      v->set_point(ant);
      return false;
    } 
  }

  // are incident bi-cells Delaunay?	
  std::vector<Facet> facets;
  facets.reserve(128);	
  this->incident_facets(v, std::back_inserter(facets));
  size = facets.size();
  for(std::size_t i=0; i<size; i++)
  {
    const Facet &f = facets[i];
    Cell_handle c = f.first;
    int j = f.second;
    Cell_handle cj = c->neighbor(j);
    int mj = this->mirror_index(c, j);
    Vertex_handle h1 = c->vertex(j);
    if(this->is_infinite(h1)) {
      if(this->side_of_sphere(c, cj->vertex(mj)->point(), true) 
         != ON_UNBOUNDED_SIDE) {
        v->set_point(ant);
        return false;
      }
    } else {
      if(this->side_of_sphere(cj, h1->point(), true) != ON_UNBOUNDED_SIDE) {
        v->set_point(ant);
        return false;
      }
    }
  }
		
  v->set_point(ant);
  return true;
}

template < class Gt, class Tds >
bool
Delaunay_triangulation_3<Gt,Tds>::
is_Gabriel(const Facet& f) const
{
  return is_Gabriel(f.first, f.second);
}

template < class Gt, class Tds >
bool
Delaunay_triangulation_3<Gt,Tds>::
is_Gabriel(Cell_handle c, int i) const
{
  CGAL_triangulation_precondition(dimension() == 3 && !is_infinite(c,i));
  typename Geom_traits::Side_of_bounded_sphere_3
    side_of_bounded_sphere =
    geom_traits().side_of_bounded_sphere_3_object();

  if ((!is_infinite(c->vertex(i))) &&
      side_of_bounded_sphere (
	c->vertex(vertex_triple_index(i,0))->point(),
	c->vertex(vertex_triple_index(i,1))->point(),
	c->vertex(vertex_triple_index(i,2))->point(),
	c->vertex(i)->point()) == ON_BOUNDED_SIDE ) return false;
    Cell_handle neighbor = c->neighbor(i);
  int in = neighbor->index(c);

  if ((!is_infinite(neighbor->vertex(in))) &&
      side_of_bounded_sphere(
	 c->vertex(vertex_triple_index(i,0))->point(),
	 c->vertex(vertex_triple_index(i,1))->point(),
	 c->vertex(vertex_triple_index(i,2))->point(),	
	 neighbor->vertex(in)->point()) == ON_BOUNDED_SIDE ) return false;

  return true;
}

template < class Gt, class Tds >
bool
Delaunay_triangulation_3<Gt,Tds>::
is_Gabriel(const Edge& e) const
{
  return is_Gabriel(e.first, e.second, e.third);
}

template < class Gt, class Tds >
bool
Delaunay_triangulation_3<Gt,Tds>::
is_Gabriel(Cell_handle c, int i, int j) const
{
  CGAL_triangulation_precondition(dimension() == 3 && !is_infinite(c,i,j));
  typename Geom_traits::Side_of_bounded_sphere_3
    side_of_bounded_sphere =
    geom_traits().side_of_bounded_sphere_3_object();

  Facet_circulator fcirc = incident_facets(c,i,j),
                   fdone(fcirc);
  Vertex_handle v1 = c->vertex(i);
  Vertex_handle v2 = c->vertex(j);
  do {
      // test whether the vertex of cc opposite to *fcirc
      // is inside the sphere defined by the edge e = (s, i,j)
      Cell_handle cc = (*fcirc).first;
      int ii = (*fcirc).second;
      if (!is_infinite(cc->vertex(ii)) &&
	   side_of_bounded_sphere( v1->point(),
				   v2->point(),
				   cc->vertex(ii)->point())
	  == ON_BOUNDED_SIDE ) return false;
  } while(++fcirc != fdone);
  return true;
}

template < class Gt, class Tds >
typename Delaunay_triangulation_3<Gt,Tds>::Point
Delaunay_triangulation_3<Gt,Tds>::
dual(Cell_handle c) const
{
  CGAL_triangulation_precondition(dimension()==3);
  CGAL_triangulation_precondition( ! is_infinite(c) );
  return c->circumcenter(geom_traits());
}


template < class Gt, class Tds >
typename Delaunay_triangulation_3<Gt,Tds>::Object
Delaunay_triangulation_3<Gt,Tds>::
dual(Cell_handle c, int i) const
{
  CGAL_triangulation_precondition(dimension()>=2);
  CGAL_triangulation_precondition( ! is_infinite(c,i) );

  if ( dimension() == 2 ) {
    CGAL_triangulation_precondition( i == 3 );
    return construct_object( construct_circumcenter(c->vertex(0)->point(),
		                                    c->vertex(1)->point(),
					            c->vertex(2)->point()) );
  }

  // dimension() == 3
  Cell_handle n = c->neighbor(i);
  if ( ! is_infinite(c) && ! is_infinite(n) )
    return construct_object(construct_segment( dual(c), dual(n) ));

  // either n or c is infinite
  int in;
  if ( is_infinite(c) )
    in = n->index(c);
  else {
    n = c;
    in = i;
  }
  // n now denotes a finite cell, either c or c->neighbor(i)
  int ind[3] = {(in+1)&3,(in+2)&3,(in+3)&3};
  if ( (in&1) == 1 )
      std::swap(ind[0], ind[1]);
  // in=0: 1 2 3
  // in=1: 3 2 0
  // in=2: 3 0 1
  // in=3: 1 0 2
  const Point& p = n->vertex(ind[0])->point();
  const Point& q = n->vertex(ind[1])->point();
  const Point& r = n->vertex(ind[2])->point();

  Line l = construct_equidistant_line( p, q, r );
  return construct_object(construct_ray( dual(n), l));
}



template < class Gt, class Tds >
typename Delaunay_triangulation_3<Gt,Tds>::Line
Delaunay_triangulation_3<Gt,Tds>::
dual_support(Cell_handle c, int i) const
{
  CGAL_triangulation_precondition(dimension()>=2);
  CGAL_triangulation_precondition( ! is_infinite(c,i) );

  if ( dimension() == 2 ) {
    CGAL_triangulation_precondition( i == 3 );
    return construct_equidistant_line( c->vertex(0)->point(),
		                       c->vertex(1)->point(),
			               c->vertex(2)->point() );
  }

  return construct_equidistant_line( c->vertex((i+1)&3)->point(),
		                     c->vertex((i+2)&3)->point(),
				     c->vertex((i+3)&3)->point() );
}


template < class Gt, class Tds >
bool
Delaunay_triangulation_3<Gt,Tds>::
is_valid(bool verbose, int level) const
{
  if ( ! tds().is_valid(verbose,level) ) {
    if (verbose)
	std::cerr << "invalid data structure" << std::endl;
    CGAL_triangulation_assertion(false);
    return false;
  }

  if ( infinite_vertex() == Vertex_handle() ) {
    if (verbose)
	std::cerr << "no infinite vertex" << std::endl;
    CGAL_triangulation_assertion(false);
    return false;
  }

  switch ( dimension() ) {
  case 3:
    {
      for(Finite_cells_iterator it = finite_cells_begin(), end = finite_cells_end(); it != end; ++it) {
	is_valid_finite(it);
	for(int i=0; i<4; i++ ) {
	  if ( !is_infinite
	       (it->neighbor(i)->vertex(it->neighbor(i)->index(it))) ) {
	    if ( side_of_sphere
		 (it,
		  it->neighbor(i)->vertex(it->neighbor(i)->index(it))->point())
		 == ON_BOUNDED_SIDE ) {
	      if (verbose)
		std::cerr << "non-empty sphere " << std::endl;
	      CGAL_triangulation_assertion(false);
	      return false;
	    }
	  }
	}
      }
      break;
    }
  case 2:
    {
      for(Finite_facets_iterator it = finite_facets_begin(), end = finite_facets_end(); it != end; ++it) {
	is_valid_finite((*it).first);
	for(int i=0; i<3; i++ ) {
	  if( !is_infinite
	      ((*it).first->neighbor(i)->vertex( (((*it).first)->neighbor(i))
						 ->index((*it).first))) ) {
	    if ( side_of_circle ( (*it).first, 3,
				  (*it).first->neighbor(i)->
				  vertex( (((*it).first)->neighbor(i))
					  ->index((*it).first) )->point() )
		 == ON_BOUNDED_SIDE ) {
	      if (verbose)
		std::cerr << "non-empty circle " << std::endl;
	      CGAL_triangulation_assertion(false);
	      return false;
	    }
	  }
	}
      }
      break;
    }
  case 1:
    {
      for(Finite_edges_iterator it = finite_edges_begin(), end = finite_edges_end(); it != end; ++it)
	is_valid_finite((*it).first);
      break;
    }
  }
  if (verbose)
      std::cerr << "Delaunay valid triangulation" << std::endl;
  return true;
}

template < class Gt, class Tds >
bool
Delaunay_triangulation_3<Gt,Tds>::
is_valid(Cell_handle c, bool verbose, int level) const
{
  if ( ! Tr_Base::is_valid(c,verbose,level) ) {
    if (verbose) {
      std::cerr << "combinatorically invalid cell" ;
      for (int i=0; i <= dimension(); i++ )
	std::cerr << c->vertex(i)->point() << ", " ;
      std::cerr << std::endl;
    }
    CGAL_triangulation_assertion(false);
    return false;
  }
  switch ( dimension() ) {
  case 3:
    {
      if ( ! is_infinite(c) ) {
	is_valid_finite(c,verbose,level);
	for (int i=0; i<4; i++ ) {
	  if (side_of_sphere(c, c->vertex((c->neighbor(i))->index(c))->point())
	      == ON_BOUNDED_SIDE ) {
	    if (verbose)
		std::cerr << "non-empty sphere " << std::endl;
	    CGAL_triangulation_assertion(false);
	    return false;
	  }
	}
      }
      break;
    }
  case 2:
    {
      if ( ! is_infinite(c,3) ) {
	for (int i=0; i<2; i++ ) {
	  if (side_of_circle(c, 3, c->vertex(c->neighbor(i)->index(c))->point())
	       == ON_BOUNDED_SIDE ) {
	    if (verbose)
		std::cerr << "non-empty circle " << std::endl;
	    CGAL_triangulation_assertion(false);
	    return false;
	  }
	}
      }
      break;
    }
  }
  if (verbose)
      std::cerr << "Delaunay valid cell" << std::endl;
  return true;
}

} //namespace CGAL

#include <CGAL/internal/Delaunay_triangulation_hierarchy_3.h>

#endif // CGAL_DELAUNAY_TRIANGULATION_3_H