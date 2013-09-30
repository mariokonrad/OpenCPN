//    This code is in the public domain. Specifically, we give to the public
//    domain all rights for future licensing of the source code, all resale
//    rights, and all publishing rights.
//
//    UNC-CH GIVES NO WARRANTY, EXPRESSED OR IMPLIED, FOR THE SOFTWARE
//    AND/OR DOCUMENTATION PROVIDED, INCLUDING, WITHOUT LIMITATION, WARRANTY
//    OF MERCHANTABILITY AND WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE.
//
//
//                                    - Atul Narkhede (narkhede@cs.unc.edu)

#ifndef __GEO__TRIANGULATE__H__
#define __GEO__TRIANGULATE__H__

#include <sys/types.h>
#include <cstdlib>
#include <cstdio>

#define ST_VALID 1    // for trapezium state
#define ST_INVALID 2


// Segment attributes

typedef struct {
      int is_valid;
      int id_poly;
      int nvert;
      int *vertex_index_list;
      void *poly_next;
} polyout;

// Integer types
typedef struct {
      int x, y;
} ipoint_t;

/* Trapezoid attributes */
typedef struct {
      int lseg, rseg;   /* two adjoining segments */
      ipoint_t hi, lo;  /* max/min y-values */
      int u0, u1;
      int d0, d1;
      int sink;         /* pointer to corresponding in Q */
      int usave, uside; /* I forgot what this means */
      int state;
      int inside;
      int ase;          // TODO remove debug
} itrap_t;

/* Segment attributes */
typedef struct {
      ipoint_t v0, v1;  /* two endpoints */
      int is_inserted;  /* inserted in trapezoidation yet ? */
      int root0, root1; /* root nodes in Q */
      int next;         /* Next logical segment */
      int prev;         /* Previous segment */
} isegment_t;

polyout *triangulate_polygon(int, int[], double (*)[2]);
int int_trapezate_polygon(int, int[], double (*)[2], itrap_t **, isegment_t **, int *);
int is_point_inside_polygon(double *);

#endif
