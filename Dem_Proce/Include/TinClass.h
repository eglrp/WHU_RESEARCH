#if !defined(AFX_TINCLASS_H__5ABA2884_CE44_11D3_B875_0050BAAF35F4__INCLUDED_)
#define AFX_TINCLASS_H__5ABA2884_CE44_11D3_B875_0050BAAF35F4__INCLUDED_

#include "MemoryPool.h"

#if !defined( REAL )

#define REAL double

struct realPOINT {
	REAL x,y;
}	;

#endif


///////////////////////////////////////////////////
/* Labels that signify the result of triPOINT location.  The result of a        */
/*   search indicates that the triPOINT falls in the interior of a TRIANGLE, on */
/*   an EDGE, on a vertex, or outside the mesh.                              */
enum locateresult {INTRIANGLE, ONEDGE, ONVERTEX, OUTSIDE};
/* Labels that signify the result of site insertion.  The result indicates   */
/*   that the triPOINT was inserted with complete success, was inserted but     */
/*   encroaches on a segment, was not inserted because it lies on a segment, */
/*   or was not inserted because another triPOINT occupies the same location.   */
enum insertsiteresult {SUCCESSFULPOINT, ENCROACHINGPOINT, VIOLATINGPOINT,DUPLICATEPOINT};
/* Labels that signify the result of direction finding.  The result          */
/*   indicates that a segment connecting the two query points falls within   */
/*   the direction TRIANGLE, along the left EDGE of the direction TRIANGLE,  */
/*   or along the right EDGE of the direction TRIANGLE.                      */
enum finddirectionresult {WITHIN, LEFTCOLLINEAR, RIGHTCOLLINEAR};

/* Labels that signify the result of the circumcenter computation routine.   */
/*   The return value indicates which EDGE of the TRIANGLE is shortest.      */
//circumcenter computation routine
enum circumcenterresult {OPPOSITEORG, OPPOSITEDEST, OPPOSITEAPEX};

/*****************************************************************************/
/*						                                                     */
/*  The basic mesh data structures                                           */
/*						                                                     */
/*  There are three:  points, triangles, and shell edges (abbreviated        */
/*  `SHELLE').  These three data structures, linked by pointers, comprise    */
/*  the mesh.  A triPOINT simply represents a triPOINT in space and its properties.*/
/*  A TRIANGLE is a TRIANGLE.  A shell EDGE is a special data structure used */
/*  to represent impenetrable segments in the mesh (including the outer      */
/*  boundary, boundaries of holes, and internal boundaries separating two    */
/*  triangulated regions).  Shell edges represent boundaries defined by the  */
/*  user that triangles may not lie across.                                  */
/*				                                                             */
/*  A TRIANGLE consists of a list of three vertices, a list of three         */
/*  adjoining triangles, a list of three adjoining shell edges (when shell   */
/*  edges are used), an arbitrary number of optional user-defined floating-  */
/*  triPOINT attributes, and an optional area constraint.  The latter is an     */
/*  upper bound on the permissible area of each TRIANGLE in a region, used   */
/*  for mesh refinement.                                                     */
/*  ------dummytri  -----dummysh                                                                      */
/*  For a TRIANGLE on a boundary of the mesh, some or all of the neighboring */
/*  triangles may not be present.  For a TRIANGLE in the interior of the     */
/*  mesh, often no neighboring shell edges are present.  Such absent         */
/*  triangles and shell edges are never represented by NULL pointers; they   */
/*  are represented by two special records:  `dummytri', the TRIANGLE that   */
/*  fills "outer space", and `dummysh', the omnipresent shell EDGE.          */
/*  `dummytri' and `dummysh' are used for several reasons; for instance,     */
/*  they can be dereferenced and their contents examined without causing the */
/*  memory protection exception that would occur if NULL were dereferenced.  */
/*                                                                           */
/*  However, it is important to understand that a TRIANGLE includes other    */
/*  information as well.  The pointers to adjoining vertices, triangles, and */
/*  shell edges are ordered in a way that indicates their geometric relation */
/*  to each other.  Furthermore, each of these pointers contains orientation */
/*  information.  Each pointer to an adjoining TRIANGLE indicates which face */
/*  of that TRIANGLE is contacted.  Similarly, each pointer to an adjoining  */
/*  shell EDGE indicates which side of that shell EDGE is contacted, and how */
/*  the shell EDGE is oriented relative to the TRIANGLE.                     */
/*				                                                              */
/*  Shell edges are found abutting edges of triangles; either sandwiched     */
/*  between two triangles, or resting against one TRIANGLE on an exterior    */
/*  boundary or hole boundary.                                               */
/*                                                                           */
/*  A shell EDGE consists of a list of two vertices, a list of two           */
/*  adjoining shell edges, and a list of two adjoining triangles.  One of    */
/*  the two adjoining triangles may not be present (though there should      */
/*  always be one), and neighboring shell edges might not be present.        */
/*  Shell edges also store a user-defined integer "boundary marker".         */
/*  Typically, this integer is used to indicate what sort of boundary        */
/*  conditions are to be applied at that location in a finite element        */
/*  simulation.                                                              */
/*                                                                           */
/*  Like triangles, shell edges maintain information about the relative      */
/*  orientation of neighboring objects.                                      */
/*                                                                          */
/*  Points are relatively simple.  A triPOINT is a list of floating triPOINT       */
/*  numbers, starting with the x, and y coordinates, followed by an          */
/*  arbitrary number of optional user-defined floating-triPOINT attributes,     */
/*  followed by an integer boundary marker.  During the segment insertion    */
/*  phase, there is also a pointer from each triPOINT to a TRIANGLE that may    */
/*  contain it.  Each pointer is not always correct, but when one is, it     */
/*  speeds up segment insertion.  These pointers are assigned values once    */
/*  at the beginning of the segment insertion phase, and are not used or     */
/*  updated at any other time.  Edge swapping during segment insertion will  */
/*  render some of them incorrect.  Hence, don't rely upon them for          */
/*  anything.  For the most part, points do not have any information about   */
/*  what triangles or shell edges they are linked to.                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  Handles                                                                  */
/*                                                                           */
/*  The oriented TRIANGLE (`triEDGE') and oriented shell EDGE (`EDGE') data  */
/*  structures defined below do not themselves store any part of the mesh.   */
/*  The mesh itself is made of `TRIANGLE's, `SHELLE's, and `triPOINT's.      */
/*														                     */
/*  Oriented triangles and oriented shell edges will usually be referred to  */
/*  as "handles".  A handle is essentially a pointer into the mesh; it       */
/*  allows you to "hold" one particular part of the mesh.  Handles are used  */
/*  to specify the regions in which one is traversing and modifying the mesh.*/
/*  A single `TRIANGLE' may be held by many handles, or none at all.  (The   */
/*  latter case is not a memory leak, because the TRIANGLE is still          */
/*  connected to other triangles in the mesh.)                               */
/*                                                                           */
/*                                                                           */
/*  A `triEDGE' is a handle that holds a TRIANGLE.  It holds a specific side */
/*  of the TRIANGLE.  An `EDGE' is a handle that holds a shell EDGE.  It     */
/*  holds either the left or right side of the EDGE.                         */
/*  --------------manipulation primitives            ---------------         */
/*  Navigation about the mesh is accomplished through a set of mesh          */
/*  manipulation primitives, further below.  Many of these primitives take   */
/*  a handle and produce a new handle that holds the mesh near the first     */
/*  handle.  Other primitives take two handles and glue the corresponding    */
/*  parts of the mesh together.                                              */
/*                                                                           */
/*  The exact position of the handles is                                     */
/*  important.  For instance, when two triangles are glued together by the   */
/*  bond() primitive, they are glued by the sides on which the handles lie.  */
/*                                                                           */
/*  Because points have no information about which triangles they are        */
/*  attached to, I commonly represent a triPOINT by use of a handle whose    */
/*  origin is the triPOINT.  A single handle can simultaneously represent a  */
/*  TRIANGLE, an EDGE, and a triPOINT.                                       */
/*                                                                           */
/*****************************************************************************/
   
/* The TRIANGLE data structure.  
/********Each TRIANGLE contains three pointers to adjoining triangles,	     */
/******** plus three pointers to vertex points,								 */
/********plus three pointers to shell edges									 */
/*   (defined below; these pointers are usually								 */
/*   `dummysh').  It may or may not also contain user-defined attributes     */
/*   and/or a floating-triPOINT "area constraint".  It may also contain extra   */
/*   pointers for nodes, when the user asks for high-order elements.         */
/*   Because the size and structure of a `TRIANGLE' is not decided until     */
/*   runtime, I haven't simply defined the type `TRIANGLE' to be a struct.   */

typedef struct tagSHELLE	SHELLE;  
typedef struct tagTriEDGE	triEDGE; 
typedef struct tagEDGE		EDGE;    
typedef struct tag_triPOINT triPOINT;

typedef struct tagTRIANGLE2 TRIANGLE;

//!!!
// because there are three version of struct TRIANGLE
// the pointer TRIANGLE * can not be used for browsing directly  ??
//

/******* when order == 1  *******/
struct  tagTRIANGLE1 {
	TRIANGLE	*adjoin[3];
	triPOINT	*vertex[3];
	SHELLE		*sh[3];    
	float		attr;      
	float		area;      
	long		node;		/* use area to replace */
	long		cntMark;   
}	;	


/******* when order == 2  *******/
struct  tagTRIANGLE2 {
	TRIANGLE	*adjoin[3];
	triPOINT	*vertex[3];
	SHELLE		*sh[3];
	float		attr;
	float		area;
	long		node;		/* use area to replace */
	long		cntMark;
	triPOINT	*highorder[3]; 
	int sign;
}	;


/******* when order == 3  *******/
struct  tagTRIANGLE3 {
	TRIANGLE	*adjoin[3];
	triPOINT	*vertex[3];
	SHELLE		*sh[3];
	float		attr;
	float		area;
	long		node;		/* use area to replace */
	long		cntMark;
	triPOINT	*highorder[7];
}	;

/* An oriented TRIANGLE:  includes a pointer to a TRIANGLE and orientation.  */
/*   The orientation denotes an EDGE of the TRIANGLE.  Hence, there are      */
/*   three possible orientations.  By convention, each EDGE is always        */
/*   directed to triPOINT counterclockwise about the corresponding TRIANGLE.    */

struct tagTriEDGE	{
	TRIANGLE *tri;
	int orient;                                         /* Ranges from 0 to 2. */
	int m_BoundarySign;
}	;
//boundary sign of cave triangles
#define  BOUNDARY_SIGN  99999

//typedef triEDGE triEDGE;

/* The shell data structure.  Each shell EDGE contains two pointers to       */
/*   adjoining shell edges, plus two pointers to vertex points, plus two     */
/*   pointers to adjoining triangles, plus one shell marker.                 */
//
struct tagSHELLE {
	SHELLE *adjoin[2];
	triPOINT  *vertex[2];
	TRIANGLE *tri[2];    
	long marker;         
};

//typedef SHELLE SHELLE;                  // old:  typedef SHELLE *SHELLE 

/* An oriented shell EDGE:  includes a pointer to a shell EDGE and an        */
/*   orientation.  The orientation denotes a side of the EDGE.  Hence, there */
/*   are two possible orientations.  By convention, the EDGE is always       */
/*   directed so that the "side" denoted is the right side of the EDGE.      */

struct tagEDGE {
	SHELLE *sh;
	int shorient;                                       /* Ranges from 0 to 1. */
};

// typedef EDGE EDGE;

/* The triPOINT data structure.  Each triPOINT is actually an array of REALs.      */
/*   The number of REALs is unknown until runtime.  An integer boundary      */
/*   marker, and sometimes a pointer to a TRIANGLE, is appended after the    */
/*   REALs.                                                                  */

struct tag_triPOINT	{
	REAL x,y;            
	float attr;          
	long marker;         
	union 	
	{ 
		triPOINT *pt;    
		int	freeCount;	
	}	dup;
	
	TRIANGLE *tri;      
	void *extraAttr;	
}	;

//typedef triPOINT triPOINT;

/* A queue used to store encroached segments.  Each segment's vertices are   */
/*   stored so that one can check whether a segment is still the same.       */

struct badSEGMENt {
	EDGE encsegment;                          /* An encroached segment. */
	triPOINT segorg, segdest;                                /* The two vertices. */
	badSEGMENt  *nextsegment;				/* Pointer to next encroached segment. */
};

/* A queue used to store bad triangles.  The key is the square of the cosine */
/*   of the smallest angle of the TRIANGLE.  Each TRIANGLE's vertices are    */
/*   stored so that one can check whether a TRIANGLE is still the same.      */

struct badFACE {
	triEDGE badfacetri;                              /* A bad TRIANGLE. */
	REAL key;                             /* cos^2 of smallest (apical) angle. */
	triPOINT *faceorg, *facedest, *faceapex;                  /* The three vertices. */
	badFACE *nextface;                 /* Pointer to next bad TRIANGLE. */
};

/* A node in a heap used to store events for the sweepline Delaunay          */
/*   algorithm.  Nodes do not point directly to their parents or children in */
/*   the heap.  Instead, each node knows its position in the heap, and can   */
/*   look up its parent and children in a separate array.  The `eventptr'    */
/*   points either to a `triPOINT' or to a TRIANGLE (in encoded format, so that */
/*   an orientation is included).  In the latter case, the origin of the     */
/*   oriented TRIANGLE is the apex of a "circle event" of the sweepline      */
/*   algorithm.  To distinguish site events from circle events, all circle   */
/*   events are given an invalid (smaller than `xmin') x-coordinate `xkey'.  */

struct sweepEVENT {
	REAL xkey, ykey;                              /* Coordinates of the event. */
	union	{
		TRIANGLE *tri;
		triPOINT *pt;
		sweepEVENT *event;
	}	*eventPtr;       /* Can be a triPOINT or the location of a circle event. */
	int heapposition;              /* Marks this event's position in the heap. */
};

/* A node in the splay tree.  Each node holds an oriented ghost TRIANGLE     */
/*   that represents a boundary EDGE of the growing triangulation.  When a   */
/*   circle event covers two boundary edges with a TRIANGLE, so that they    */
/*   are no longer boundary edges, those edges are not immediately deleted   */
/*   from the tree; rather, they are lazily deleted when they are next       */
/*   encountered.  (Since only a random sample of boundary edges are kept    */
/*   in the tree, lazy deletion is faster.)  `keydest' is used to verify     */
/*   that a TRIANGLE is still the same as when it entered the splay tree; if */
/*   it has been rotated (due to a circle event), it no longer represents a  */
/*   boundary EDGE and should be deleted.                                    */

struct splayNODE {
	triEDGE keyedge;                  /* Lprev of an EDGE on the front. */
	triPOINT *keydest;            /* Used to verify that splay node is still live. */
	splayNODE *lchild, *rchild;              /* Children in splay tree. */
};

///////////////////////////////////////////////////////////////////



/*****************************************************************************/
/*                                                                           */
/*  (triangle.h)                                                             */
/*                                                                           */
/*  Include file for programs that call Triangle.                            */
/*                                                                           */
/*  Accompanies Triangle Version 1.3                                         */
/*  July 19, 1996                                                            */
/*                                                                           */
/*  Copyright 1996                                                           */
/*  Jonathan Richard Shewchuk                                                */
/*  School of Computer Science                                               */
/*  Carnegie Mellon University                                               */
/*  5000 Forbes Avenue                                                       */
/*  Pittsburgh, Pennsylvania  15213-3891                                     */
/*  jrs@cs.cmu.edu                                                           */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  How to call Triangle from another program                                */
/*                                                                           */
/*                                                                           */
/*  If you haven't read Triangle's instructions (run "triangle -h" to read   */
/*  them), you won't understand what follows.                                */
/*                                                                           */
/*  Triangle must be compiled into an object file (triangle.o) with the      */
/*  TRILIBRARY symbol defined (preferably by using the -DTRILIBRARY compiler */
/*  switch).  The makefile included with Triangle will do this for you if    */
/*  you run "make trilibrary".  The resulting object file can be called via  */
/*  the procedure triangulate().                                             */
/*                                                                           */
/*  If the size of the object file is important to you, you may wish to      */
/*  generate a reduced version of triangle.o.  The REDUCED symbol gets rid   */
/*  of all features that are primarily of research interest.  Specifically,  */
/*  the -DREDUCED switch eliminates Triangle's -i, -F, -s, and -C switches.  */
/*  The CDT_ONLY symbol gets rid of all meshing algorithms above and beyond  */
/*  constrained Delaunay triangulation.  Specifically, the -DCDT_ONLY switch */
/*  eliminates Triangle's -r, -q, -a, -S, and -s switches.                   */
/*                                                                           */
/*  IMPORTANT:  These definitions (TRILIBRARY, REDUCED, CDT_ONLY) must be    */
/*  made in the makefile or in triangle.c itself.  Putting these definitions */
/*  in this file will not create the desired effect.                         */
/*                                                                           */
/*                                                                           */
/*  The calling convention for triangulate() follows.                        */
/*                                                                           */
/*      void triangulate(triswitches, in, out, vorout)                       */
/*      char *triswitches;                                                   */
/*      TinIO *in;                                            */
/*      TinIO *out;                                           */
/*      TinIO *vorout;                                        */
/*                                                                           */
/*  `triswitches' is a string containing the command line switches you wish  */
/*  to invoke.  No initial dash is required.  Some suggestions:              */
/*                                                                           */
/*  - You'll probably find it convenient to use the `z' switch so that       */
/*    points (and other items) are numbered from zero.  This simplifies      */
/*    indexing, because the first item of any type always starts at index    */
/*    [0] of the corresponding array, whether that item's number is zero or  */
/*    one.                                                                   */
/*  - You'll probably want to use the `Q' (quiet) switch in your final code, */
/*    but you can take advantage of Triangle's printed output (including the */
/*    `V' switch) while debugging.                                           */
/*  - If you are not using the `q' or `a' switches, then the output points   */
/*    will be identical to the input points, except possibly for the         */
/*    boundary markers.  If you don't need the boundary markers, you should  */
/*    use the `N' (no nodes output) switch to save memory.  (If you do need  */
/*    boundary markers, but need to save memory, a good nasty trick is to    */
/*    set out->pointList equal to in->pointList before calling triangulate(),*/
/*    so that Triangle overwrites the input points with identical copies.)   */
/*  - The `I' (no iteration numbers) and `g' (.off file output) switches     */
/*    have no effect when Triangle is compiled with TRILIBRARY defined.      */
/*                                                                           */
/*  `in', `out', and `vorout' are descriptions of the input, the output,     */
/*  and the Voronoi output.  If the `v' (Voronoi output) switch is not used, */
/*  `vorout' may be NULL.  `in' and `out' may never be NULL.                 */
/*                                                                           */
/*  Certain fields of the input and output structures must be initialized,   */
/*  as described below.                                                      */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  The `triangulateIO' structure.                                           */
/*                                                                           */
/*  Used to pass data into and out of the triangulate() procedure.           */
/*                                                                           */
/*                                                                           */
/*  Arrays are used to store points, triangles, markers, and so forth.  In   */
/*  all cases, the first item in any array is stored starting at index [0].  */
/*  However, that item is item number `1' unless the `z' switch is used, in  */
/*  which case it is item number `0'.  Hence, you may find it easier to      */
/*  index points (and triangles in the neighbor list) if you use the `z'     */
/*  switch.  Unless, of course, you're calling Triangle from a Fortran       */
/*  program.                                                                 */
/*                                                                           */
/*  Description of fields (except the `numberof' fields, which are obvious): */
/*                                                                           */
/*  `pointList':  An array of point coordinates.  The first point's x        */
/*    coordinate is at index [0] and its y coordinate at index [1], followed */
/*    by the coordinates of the remaining points.  Each point occupies two   */
/*    REALs.                                                                 */
/*  `pointAttrList':  An array of point attributes.  Each point's       */
/*    attributes occupy `numOfPointAttrs' REALs.                     */
/*  `pointMarkList':  An array of point markers; one int per point.        */
/*                                                                           */
/*  `triList':  An array of triangle corners.  The first triangle's     */
/*    first corner is at index [0], followed by its other two corners in     */
/*    counterclockwise order, followed by any other nodes if the triangle    */
/*    represents a nonlinear element.  Each triangle occupies                */
/*    `numOfCorners' ints.                                                */
/*  `triAttrList':  An array of triangle attributes.  Each         */
/*    triangle's attributes occupy `numOfTriAttributes' REALs.       */
/*  `triAreaList':  An array of triangle area constraints; one REAL per */
/*    triangle.  Input only.                                                 */
/*  `neighborList':  An array of triangle neighbors; three ints per          */
/*    triangle.  Output only.                                                */
/*                                                                           */
/*  `segmentList':  An array of segment endpoints.  The first segment's      */
/*    endpoints are at indices [0] and [1], followed by the remaining        */
/*    segments.  Two ints per segment.                                       */
/*  `segMarkList':  An array of segment markers; one int per segment.  */
/*                                                                           */
/*  `holeList':  An array of holes.  The first hole's x and y coordinates    */
/*    are at indices [0] and [1], followed by the remaining holes.  Two      */
/*    REALs per hole.  Input only, although the pointer is copied to the     */
/*    output structure for your convenience.                                 */
/*                                                                           */
/*  `regionList':  An array of regional attributes and area constraints.     */
/*    The first constraint's x and y coordinates are at indices [0] and [1], */
/*    followed by the regional attribute and index [2], followed by the      */
/*    maximum area at index [3], followed by the remaining area constraints. */
/*    Four REALs per area constraint.  Note that each regional attribute is  */
/*    used only if you select the `A' switch, and each area constraint is    */
/*    used only if you select the `a' switch (with no number following), but */
/*    omitting one of these switches does not change the memory layout.      */
/*    Input only, although the pointer is copied to the output structure for */
/*    your convenience.                                                      */
/*                                                                           */
/*  `edgeList':  An array of edge endpoints.  The first edge's endpoints are */
/*    at indices [0] and [1], followed by the remaining edges.  Two ints per */
/*    edge.  Output only.                                                    */
/*  `edgeMarkList':  An array of edge markers; one int per edge.  Output   */
/*    only.                                                                  */
/*  `normList':  An array of normal vectors, used for infinite rays in       */
/*    Voronoi diagrams.  The first normal vector's x and y magnitudes are    */
/*    at indices [0] and [1], followed by the remaining vectors.  For each   */
/*    finite edge in a Voronoi diagram, the normal vector written is the     */
/*    zero vector.  Two REALs per edge.  Output only.                        */
/*                                                                           */
/*                                                                           */
/*  Any input fields that Triangle will examine must be initialized.         */
/*  Furthermore, for each output array that Triangle will write to, you      */
/*  must either provide space by setting the appropriate pointer to point    */
/*  to the space you want the data written to, or you must initialize the    */
/*  pointer to NULL, which tells Triangle to allocate space for the results. */
/*  The latter option is preferable, because Triangle always knows exactly   */
/*  how much space to allocate.  The former option is provided mainly for    */
/*  people who need to call Triangle from Fortran code, though it also makes */
/*  possible some nasty space-saving tricks, like writing the output to the  */
/*  same arrays as the input.                                                */
/*                                                                           */
/*  Triangle will not free() any input or output arrays, including those it  */
/*  allocates itself; that's up to you.                                      */
/*                                                                           */
/*  Here's a guide to help you decide which fields you must initialize       */
/*  before you call triangulate().                                           */
/*                                                                           */
/*  `in':                                                                    */
/*                                                                           */
/*    - `pointList' must always point to a list of points; `numOfPoints'  */
/*      and `numOfPointAttrs' must be properly set.                  */
/*      `pointMarkList' must either be set to NULL (in which case all      */
/*      markers default to zero), or must point to a list of markers.  If    */
/*      `numOfPointAttrs' is not zero, `pointAttrList' must     */
/*      point to a list of point attributes.                                 */
/*    - If the `r' switch is used, `triList' must point to a list of    */
/*      triangles, and `numOfTriangles', `numOfCorners', and           */
/*      `numOfTriAttributes' must be properly set.  If               */
/*      `numOfTriAttributes' is not zero, `triAttrList'    */
/*      must point to a list of triangle attributes.  If the `a' switch is   */
/*      used (with no number following), `triAreaList' must point to a  */
/*      list of triangle area constraints.  `neighborList' may be ignored.   */
/*    - If the `p' switch is used, `segmentList' must point to a list of     */
/*      segments, `numOfSegments' must be properly set, and               */
/*      `segMarkList' must either be set to NULL (in which case all    */
/*      markers default to zero), or must point to a list of markers.        */
/*    - If the `p' switch is used without the `r' switch, then               */
/*      `numOfHoles' and `numOfRegions' must be properly set.  If      */
/*      `numOfHoles' is not zero, `holeList' must point to a list of      */
/*      holes.  If `numOfRegions' is not zero, `regionList' must point to */
/*      a list of region constraints.                                        */
/*    - If the `p' switch is used, `holeList', `numOfHoles',              */
/*      `regionList', and `numOfRegions' is copied to `out'.  (You can    */
/*      nonetheless get away with not initializing them if the `r' switch is */
/*      used.)                                                               */
/*    - `edgeList', `edgeMarkList', `normList', and `numberofedges' may be */
/*      ignored.                                                             */
/*                                                                           */
/*  `out':                                                                   */
/*                                                                           */
/*    - `pointList' must be initialized (NULL or pointing to memory) unless  */
/*      the `N' switch is used.  `pointMarkList' must be initialized       */
/*      unless the `N' or `B' switch is used.  If `N' is not used and        */
/*      `in->numOfPointAttrs' is not zero, `pointAttrList' must */
/*      be initialized.                                                      */
/*    - `triList' must be initialized unless the `E' switch is used.    */
/*      `neighborList' must be initialized if the `n' switch is used.  If    */
/*      the `E' switch is not used and (`in->numberofelementattributes' is   */
/*      not zero or the `A' switch is used), `elementattributelist' must be  */
/*      initialized.  `triAreaList' may be ignored.                     */
/*    - `segmentList' must be initialized if the `p' or `c' switch is used,  */
/*      and the `P' switch is not used.  `segMarkList' must also be    */
/*      initialized under these circumstances unless the `B' switch is used. */
/*    - `edgeList' must be initialized if the `e' switch is used.            */
/*      `edgeMarkList' must be initialized if the `e' switch is used and   */
/*      the `B' switch is not.                                               */
/*    - `holeList', `regionList', `normList', and all scalars may be ignored.*/
/*                                                                           */
/*  `vorout' (only needed if `v' switch is used):                            */
/*                                                                           */
/*    - `pointList' must be initialized.  If `in->numOfPointAttrs'   */
/*      is not zero, `pointAttrList' must be initialized.               */
/*      `pointMarkList' may be ignored.                                    */
/*    - `edgeList' and `normList' must both be initialized.                  */
/*      `edgeMarkList' may be ignored.                                     */
/*    - Everything else may be ignored.                                      */
/*                                                                           */
/*  After a call to triangulate(), the valid fields of `out' and `vorout'    */
/*  will depend, in an obvious way, on the choice of switches used.  Note    */
/*  that when the `p' switch is used, the pointers `holeList' and            */
/*  `regionList' are copied from `in' to `out', but no new space is          */
/*  allocated; be careful that you don't free() the same array twice.  On    */
/*  the other hand, Triangle will never copy the `pointList' pointer (or any */
/*  others); new space is allocated for `out->pointList', or if the `N'      */
/*  switch is used, `out->pointList' remains uninitialized.                  */
/*                                                                           */
/*  All of the meaningful `numberof' fields will be properly set; for        */
/*  instance, `numberofedges' will represent the number of edges in the      */
/*  triangulation whether or not the edges were written.  If segments are    */
/*  not used, `numOfSegments' will indicate the number of boundary edges. */
/*                                                                           */
/*****************************************************************************/


struct triangulateIO {
	REAL *pointList;        	/* In / out */
	float *pointAttrList; 		/* In / out */
	long *pointMarkList;		/* In / out */
	long   numOfPoints;			/* In / out */
	long   numOfPointAttrs;		/* In / out */

	long *triList; 				/* In / out */
	float *triAttrList;			/* In / out */
	float *triAreaList; 		/* In only */
	long *neighborList;			/* Out only */
	long numOfTriangles;			/* In / out */
	long numOfCorners; 			/* In / out */
	long numOfTriAttrs; 			/* In / out */

	long *segmentList;		/* In / out */
	long *segMarkList;		/* In / out */
	long numOfSegments;		/* In / out */

	REAL *holeList;			/* In / pointer to array copied out */
	long numOfHoles;		/* In / copied out */

	REAL *regionList;		/* In / pointer to array copied out */
	long numOfRegions;		/* In / copied out */

	long *edgeList;    		/* Out only */
	long *edgeMarkList;	/* Not used with Voronoi diagram; out only */
	REAL *normList;			/* Used only with Voronoi diagram; out only */
	long numOfEdges;		/* Out only */
};

typedef triangulateIO TinIO;

////////////////////////////////////////////////////////////////////

#ifdef TINDLL_EXPORTS
#define TINDLL_API __declspec(dllexport)
#else
#define TINDLL_API __declspec(dllimport)
#endif


//extern int plus1mod3[3];
//extern int minus1mod3[3];

class AFX_EXT_CLASS CTINClass	{
public:

	int plus1mod3[3];
	int minus1mod3[3];
	/********* Primitives for m_triangles                                  *********/
	/* decode() converts a pointer to an oriented TRIANGLE.  The orientation is  */
	/*   extracted from the two least significant bits of the pointer.           */
	
	void decode(TRIANGLE *ptr, triEDGE &triEdge) 
	{
		triEdge.orient = (int) ((unsigned long) (ptr) & (unsigned long) 3l);  
		triEdge.tri = (TRIANGLE *) ((unsigned long) (ptr) ^ (unsigned long) triEdge.orient);
	};
	
	/* encode() compresses an oriented TRIANGLE into a single pointer.  It       */
	/*   relies on the assumption that all m_triangles are aligned to four-byte    */
	/*   boundaries, so the two least significant bits of triEdge.tri are zero.*/
	
	TRIANGLE *encode(triEDGE &triEdge) 
	{
		return (TRIANGLE *) ((unsigned long) triEdge.tri | (unsigned long) triEdge.orient);
	};
	
	/* The following EDGE manipulation primitives are all described by Guibas    */
	/*   and Stolfi.  However, they use an EDGE-based data structure, whereas I  */
	/*   am using a TRIANGLE-based data structure.                               */
	
	/* SymmTri() finds the abutting TRIANGLE, on the same EDGE.  Note that the       */
	/*   EDGE direction is necessarily reversed, because TRIANGLE/EDGE handles   */
	/*   are always directed counterclockwise around the TRIANGLE.               */
	//寻找相邻三角形---对称三角形--相邻边在不同三角形中的方向是不相同的，编号是按逆时针的
	//下面两个函数唯一不同的是：返回值存储于不同的空间
	void SymmTri(triEDGE &triEdge1, triEDGE &triEdge2)//存在triEdge2 
	{
		decode( triEdge1.tri->adjoin[triEdge1.orient], triEdge2 );
	};
	
	void SymmTriSelf(triEDGE &triEdge)  
	{
		decode(triEdge.tri->adjoin[triEdge.orient], triEdge);
	};
	
	/* NextEdge() finds the next EDGE (counterclockwise) of a TRIANGLE.             */
	//在三角形三边中找当前边的下一条有向边
	//返回有向边，不覆盖当前边
	void NextEdge(triEDGE &triEdge1, triEDGE &triEdge2) 
	{
		triEdge2.tri = triEdge1.tri;
		triEdge2.orient = plus1mod3[triEdge1.orient];
	};
	//返回的当前边的下一条边，覆盖当前边
	void NextEdgeSelf(triEDGE &triEdge)  
	{
		triEdge.orient = plus1mod3[triEdge.orient];
	};
	
	/* PrevEdge() finds the previous EDGE (clockwise) of a TRIANGLE.                */
	//在三角形三边中找前一条有向边
	void PrevEdge(triEDGE &triEdge1, triEDGE &triEdge2) 
	{
		triEdge2.tri = triEdge1.tri;
		triEdge2.orient = minus1mod3[triEdge1.orient];
	};
	
	void PrevEdgeSelf(triEDGE &triEdge)
	{
		triEdge.orient = minus1mod3[triEdge.orient];
	};
	
	/* oNextSpinEdge() spins counterclockwise around a triPOINT; that is, it finds the next */
	/*   EDGE with the same origin in the counterclockwise direction.  This EDGE */
	/*   will be part of a different TRIANGLE.                                   */
	//寻找具有同一个起点的下一条有向边
	//逆时针
	void oNextSpinEdge(triEDGE &triEdge1, triEDGE &triEdge2) 
	{
		PrevEdge(triEdge1, triEdge2);   
		SymmTriSelf(triEdge2);
	};
	
	void oNextSpinEdgeSelf(triEDGE &triEdge) 
	{
		PrevEdgeSelf(triEdge);
		SymmTriSelf(triEdge);
	};
	/* oPrevSpinEdge() spins clockwise around a triPOINT; that is, it finds the next EDGE   */
	/*   with the same origin in the clockwise direction.  This EDGE will be     */
	/*   part of a different TRIANGLE.                                           */
	//寻找具有同一个起点的前一条有向边
	//顺时针
	void oPrevSpinEdge(triEDGE &triEdge1, triEDGE &triEdge2) 
	{
		SymmTri(triEdge1, triEdge2);
		NextEdgeSelf(triEdge2);
	};
	
	void oPrevSpinEdgeSelf(triEDGE &triEdge)
	{
		SymmTriSelf(triEdge); 
		NextEdgeSelf(triEdge);
	};
	/* dNextSpinEdge() spins counterclockwise around a triPOINT; that is, it finds the next */
	/*   EDGE with the same destination in the counterclockwise direction.  This */
	/*   EDGE will be part of a different TRIANGLE.                              */
	//逆时针
	void dNextSpinEdge(triEDGE &triEdge1, triEDGE &triEdge2) 
	{
		SymmTri(triEdge1, triEdge2);
		PrevEdgeSelf(triEdge2);
	};
	
	void dNextSpinEdgeself(triEDGE &triEdge)
	{
		SymmTriSelf(triEdge);  
		PrevEdgeSelf(triEdge);
	};
	/* dPrevSpinEdge() spins clockwise around a triPOINT; that is, it finds the next EDGE   */
	/*   with the same destination in the clockwise direction.  This EDGE will   */
	/*   be part of a different TRIANGLE.                                        */
	//顺时针
	void dPrevSpinEdge(triEDGE &triEdge1, triEDGE &triEdge2) 
	{
		NextEdge(triEdge1, triEdge2);
		SymmTriSelf(triEdge2);
	};
	void dPrevSpinEdgeSelf(triEDGE &triEdge)
	{
		NextEdgeSelf(triEdge); 
		SymmTriSelf(triEdge);
	};
	
	/* rnext() moves one EDGE counterclockwise about the adjacent TRIANGLE.      */
	/*   (It's best understood by reading Guibas and Stolfi.  It involves        */
	/*   changing m_triangles twice.)                                              */
	//逆时针
	void rnext(triEDGE &triEdge1, triEDGE &triEdge2) 
	{
		SymmTri(triEdge1, triEdge2); 
		NextEdgeSelf(triEdge2); 
		SymmTriSelf(triEdge2);
	};
	void rnextself(triEDGE &triEdge)
	{
		SymmTriSelf(triEdge); 
		NextEdgeSelf(triEdge);
		SymmTriSelf(triEdge);
	};
	/* rnext() moves one EDGE clockwise about the adjacent TRIANGLE.             */
	/*   (It's best understood by reading Guibas and Stolfi.  It involves        */
	/*   changing m_triangles twice.)                                              */
	//顺时针
	void rprev(triEDGE &triEdge1, triEDGE &triEdge2) 
	{
		SymmTri(triEdge1, triEdge2); 
		PrevEdgeSelf(triEdge2);  
		SymmTriSelf(triEdge2);
	};
	
	void rprevself(triEDGE &triEdge)  
	{
		SymmTriSelf(triEdge);     
		PrevEdgeSelf(triEdge);
		SymmTriSelf(triEdge);
	};
	
	/* These primitives determine or set the origin, destination, or apex of a   */
	/* TRIANGLE.                                                                 */
	//返回有向边的起点
	triPOINT *org(triEDGE &triEdge) 
	{                                               
		return triEdge.tri->vertex[ plus1mod3[triEdge.orient] ];
	};
	//返回有向边的终点
	triPOINT *dest(triEDGE &triEdge)
	{                                            
		return triEdge.tri->vertex[minus1mod3[triEdge.orient]];
	};
	//返回三角形的顶点
	triPOINT *apex(triEDGE &triEdge)
	{                                      
		return triEdge.tri->vertex[triEdge.orient];
	};
	//设置有向边的起点
	void SetOrg(triEDGE &triEdge, triPOINT *pt)
	{                                           
		triEdge.tri->vertex[plus1mod3[triEdge.orient]] = pt;
	};
    //设置有向边的终点
	void SetDest(triEDGE &triEdge, triPOINT *pt)  
	{                                         
		triEdge.tri->vertex[minus1mod3[triEdge.orient]] = pt;
	};
	//设置三角形的顶点
	void SetApex(triEDGE &triEdge, triPOINT *pt)  
	{                           
		triEdge.tri->vertex[triEdge.orient] =  pt;
	};

	void SetVertices2Null(triEDGE &triEdge) 
	{                                 
		triEdge.tri->vertex[0] = (triPOINT *) NULL;  
		triEdge.tri->vertex[1] = (triPOINT *) NULL; 
		triEdge.tri->vertex[2] = (triPOINT *) NULL;
	};
	/* Bond two m_triangles together.                                              */
	//捆绑两个三角形
	void bond(triEDGE &triEdge1, triEDGE &triEdge2)  
	{
		(triEdge1).tri->adjoin[(triEdge1).orient] = encode(triEdge2);
		(triEdge2).tri->adjoin[(triEdge2).orient] = encode(triEdge1);
	};
	
	/* Dissolve a bond (from one side).  Note that the other TRIANGLE will still */
	/*   think it's connected to this TRIANGLE.  Usually, however, the other     */
	/*   TRIANGLE is being deleted entirely, or bonded to another TRIANGLE, so   */
	/*   it doesn't matter.                                                      */
	//解绑两个三角形
	void dissolve(triEDGE &triEdge )
	{                                                   
		triEdge.tri->adjoin[triEdge.orient] = m_dummytri;
	};
	
	/* Copy a TRIANGLE/EDGE handle.                                              */
	//拷贝三角形或有向边
	void triedgecopy(triEDGE &triEdge1, triEDGE &triEdge2)
	{
		(triEdge2).tri = (triEdge1).tri;
		(triEdge2).orient = (triEdge1).orient;
	};
	
	/* Test for equality of TRIANGLE/EDGE handles.                               */
	//判断两个三角形或有向边是否一致
	bool TriEdgeEqual(triEDGE &triEdge1, triEDGE &triEdge2)
	{
		return (((triEdge1).tri == (triEdge2).tri) && ((triEdge1).orient == (triEdge2).orient));
	};
	
	/* Primitives to infect or cure a TRIANGLE with the virus.  These rely on    */
	/*   the assumption that all shell edges are aligned to four-byte boundaries.*/
	
	void infect(triEDGE &triEdge )
	{
		triEdge.tri->sh[0] = (SHELLE *)((unsigned long) triEdge.tri->sh[0] | (unsigned long) 2l);
	};
	
	void uninfect(triEDGE &triEdge ) 
	{
		triEdge.tri->sh[0] = (SHELLE *) ((unsigned long) triEdge.tri->sh[0] & ~ (unsigned long) 2l);
	};
	
	/* Test a TRIANGLE for viral infection.                                      */
	
	bool infected(triEDGE &triEdge) 
	{
		return (((unsigned long) triEdge.tri->sh[0] & (unsigned long) 2l) != 0);
	};
	
	float elemattribute(triEDGE &triEdge , int attnum)
	{
		return triEdge.tri->attr;
	};
	
	void setelemattribute(triEDGE &triEdge, int attnum, float value)
	{
		triEdge.tri->attr = value;
	};
	
	/* Check or set a TRIANGLE's maximum area bound.                             */
	//检查三角形的面积最大跨度（最大范围）
	double areabound(triEDGE  &triEdge)	{ return triEdge.tri->area; };
	//设置三角形的面积最大跨度（最大范围）
	void setareabound(triEDGE &triEdge, float value) { triEdge.tri->area = value; };
	
	/********* Primitives for shell edges                                *********/
	/*                                                                           */
	/*                                                                           */
	
	/* shDecode() converts a pointer to an oriented shell EDGE.  The orientation  */
	/*   is extracted from the least significant bit of the pointer.  The two    */
	/*   least significant bits (one for orientation, one for viral infection)   */
	/*   are masked out to produce the real pointer.                             */
	
	void shDecode(SHELLE *sptr, EDGE &edge) 
	{
		edge.shorient = (int) ((unsigned long) (sptr) & (unsigned long) 1l); 
		edge.sh = (SHELLE *)((unsigned long) (sptr) & ~ (unsigned long) 3l);
	};
	
	/* shEncode() compresses an oriented shell EDGE into a single pointer.  It    */
	/*   relies on the assumption that all shell edges are aligned to two-byte   */
	/*   boundaries, so the least significant bit of edge.sh is zero.          */
	
	SHELLE * shEncode(EDGE &edge)  
	{
		return (SHELLE *) ((unsigned long) edge.sh | (unsigned long) edge.shorient);
	};
	
	/* shSymmTri() toggles the orientation of a shell EDGE.                           */
	void shSymmTri(EDGE &edge1, EDGE &edge2) 
	{
		(edge2).sh = (edge1).sh;
		(edge2).shorient = 1 - (edge1).shorient;
	};
	
	void shSymmTriSelf(EDGE &edge) {edge.shorient = 1 - edge.shorient; };
	
	/* spivot() finds the other shell EDGE (from the same segment) that shares   */
	/*   the same origin.                                                        */
	
	void spivot(EDGE &edge1, EDGE &edge2) 
	{                                            
		shDecode( (edge1).sh->adjoin[(edge1).shorient], edge2);
	};
	
	void spivotself(EDGE &edge)  { shDecode(edge.sh->adjoin[edge.shorient], edge); };
	
	/* shNext() finds the next shell EDGE (from the same segment) in sequence;    */
	/*   one whose origin is the input shell EDGE's destination.                 */
	
	void shNext(EDGE &edge1, EDGE &edge2)
	{
		shDecode((edge1).sh->adjoin[1 - (edge1).shorient], edge2);
	};
	
	void shNextself(EDGE &edge) { shDecode(edge.sh->adjoin[1 - edge.shorient], edge); };
	
	/* These primitives determine or set the origin or destination of a shell    */
	/*   EDGE.                                                                   */
	
	triPOINT *shOrg(EDGE &edge)  { return edge.sh->vertex[edge.shorient]; };
	
	triPOINT *shDest(EDGE &edge) { return edge.sh->vertex[1 - edge.shorient]; };
	
	void setshOrg(EDGE &edge, triPOINT *pointptr) { edge.sh->vertex[edge.shorient] = pointptr; };
	
	void setshDest(EDGE &edge, triPOINT *pointptr) { edge.sh->vertex[1-edge.shorient] = pointptr; };
	
	/* These primitives read or set a shell marker.  Shell markers are used to   */
	/*   hold user boundary information.                                         */
	
	long shMark(EDGE &edge) { return edge.sh->marker; };
	
	void SetShellMark(EDGE &edge, long value) { edge.sh->marker = value; };
	
	/* Bond two shell edges together.                                            */
	void shBond(EDGE &edge1, EDGE &edge2) 
	{
		(edge1).sh->adjoin[(edge1).shorient] = shEncode(edge2);
		(edge2).sh->adjoin[(edge2).shorient] = shEncode(edge1);
	};
	
	/* Dissolve a shell EDGE bond (from one side).  Note that the other shell    */
	/*   EDGE will still think it's connected to this shell EDGE.                */
	
	void shDissolve(EDGE &edge)
	{
		edge.sh->adjoin[edge.shorient] = (SHELLE *) m_dummysh;
	};
	
	/* Copy a shell EDGE.                                                        */
	
	void shellecopy(EDGE &edge1, EDGE &edge2)
	{
		(edge2).sh = (edge1).sh;
		(edge2).shorient = (edge1).shorient;
	};
	/* Test for equality of shell edges.                                         */
	
	bool shelleequal(EDGE &edge1, EDGE &edge2) 
	{ 
		return (((edge1).sh == (edge2).sh) && ((edge1).shorient == (edge2).shorient));
	};
	
	/********* Primitives for interacting m_triangles and shell edges      *********/
	/*                   三角形和突壳边的相互操作                                  */
	/*                                                                           */
	
	/* tspivot() finds a shell EDGE abutting a TRIANGLE.                         */
	void tspivot(triEDGE &triEdge, EDGE &edge) 
	{
		shDecode( triEdge.tri->sh[triEdge.orient], edge);
	};
	
	/* stpivot() finds a TRIANGLE abutting a shell EDGE.  It requires that the   */
	/*   variable `ptr' of type `TRIANGLE' be defined.                           */
	
	void stpivot(EDGE &edge, triEDGE &triEdge)
	{
		decode(edge.sh->tri[edge.shorient], triEdge);
	};
	
	/* Bond a TRIANGLE to a shell EDGE.                                          */
	
	void tshBond(triEDGE &triEdge, EDGE &edge)
	{
		triEdge.tri->sh[triEdge.orient] = shEncode(edge);
		edge.sh->tri[edge.shorient] = encode(triEdge);
	};
	
	/* Dissolve a bond (from the TRIANGLE side).                                 */
	
	void tshDissolve(triEDGE &triEdge)
	{
		triEdge.tri->sh[triEdge.orient] =  m_dummysh;
	};

	/* Dissolve a bond (from the shell EDGE side).                               */
	void shtDissolve(EDGE &edge)
	{
		edge.sh->tri[edge.shorient] =  m_dummytri;
	};
	
	/********* Primitives for points      点操作                         *********/
	/*                                                                           */
	/*                                                                          */
	//返回点的标记 
	long PointMark(triPOINT *pt)	{  return pt->marker; };
	//设置点的标记
	void SetPointMark(triPOINT * pt,long value) { pt->marker = value; };
	//返回三角形的指针
	TRIANGLE *Point2Tri(triPOINT *pt)  { return pt->tri; };
	//设置点指向的三角形
	void SetPoint2Tri(triPOINT * pt,TRIANGLE * value) { pt->tri = value; };
	//返回重复点的数目
	int PointDupCount(triPOINT * pt)
	{ 
		if( pt->dup.freeCount < 0 ) //重复次数是负数
			return -pt->dup.freeCount;//返回重复次数
		else return 0; //其它，表示没有重复
	}
    //
	triPOINT *PointDup(triPOINT * pt) 
	{ 
		if( pt->dup.freeCount < 0 ) //pt是重复点
			return NULL;
		else//当freeCount >= 0 
			return	pt->dup.pt; 
	}
    //设置重复次数（增加） 
	void SetPointDup(triPOINT *pt, triPOINT *dupPt) { 
		if( dupPt == NULL )
			pt->dup.freeCount = -1L;
		else	
		{
			pt->dup.pt = dupPt; 
			dupPt->dup.freeCount--;
		}
	};
    //减少重复次数
	int DecreasePointDupCount(triPOINT * pt) { 
		if( pt->dup.freeCount < 0  ) 
			pt->dup.freeCount++; 
		return -pt->dup.freeCount; 
	};
	
	/********* Mesh manipulation primitives end here                     *********/
	
private:
	/* Variables used to allocate memory for triangles, shell edges, points,     */
	/*   viri (triangles being eaten), bad (encroached) segments, bad (skinny    */
	/*   or too large) triangles, and splay tree nodes.                          */
	//存放三角形、突壳边、点 等变量
	CMemoryPool <TRIANGLE, TRIANGLE &>		m_triangles;//三角形
	CMemoryPool <SHELLE, SHELLE &>			m_shelles;//突壳边
	CMemoryPool <triPOINT, triPOINT & >		m_points;//点
	CMemoryPool <TRIANGLE *, TRIANGLE *>	m_viri; //被吞噬的三角形
	CMemoryPool <EDGE, EDGE &>				m_badSegments;//被侵蚀的边
	CMemoryPool <badFACE, badFACE &>		m_badTriangles;//太小或太大的三角形
	CMemoryPool <splayNODE,splayNODE &>		m_splayNodes;//树节点
	
	/* Variables that maintain the bad TRIANGLE queues.  The tails are pointers  */
	/*   to the pointers that have to be filled in to enqueue an item.           */
	badFACE *queuefront[64];
	badFACE **queuetail[64];
	//数据范围
	REAL m_xmin, m_xmax, m_ymin, m_ymax;                    /* x and y bounds. */
	REAL m_xminextreme;        /* Nonexistent x value used as a flag in sweepline. */
	
	int inelements;       //输入的三角形数目       /* Number of input triangles. */
	int insegments;       //。。。线段。。。        /* Number of input segments. */
	int holes;            //      空洞                 /* Number of input holes. */
	int regions;          //。。。区域。。。          /* Number of input regions. */
	long edges;           //。。。输出边。。           /* Number of output edges. */
	int mesh_dim;                                  /* Dimension (ought to be 2). */
	int nextras;           //每个点的属性数   /* Number of attributes per triPOINT. */
	int eextras;           //每个三角形的属性数   /* Number of attributes per TRIANGLE. */
	long hullsize;               //凸壳的边数       /* Number of edges of convex hull. */
	int triwords;                                   /* Total words per TRIANGLE. */
	int shwords;                                  /* Total words per shell EDGE. */
	int readnodefile;                             /* Has a .node file been read? */
	long samples;                /* Number of random samples for triPOINT location. */
    
	bool m_checksegments;           /* Are there segments in the triangulation yet? */
	
	/* Switches for the triangulator.                                            */
	/*   poly: -p switch.  refine: -r switch.                                    */
	/*   quality: -q switch.                                                     */
	/*     minangle: minimum angle bound, specified after -q switch.             */
	/*     goodangle: cosine squared of minangle.                                */
	/*   vararea: -a switch without number.                                      */
	/*   fixedarea: -a switch with number.                                       */
	/*     maxarea: maximum area bound, specified after -a switch.               */
	/*   regionattrib: -A switch.  convex: -c switch.                            */
	/*   firstnumber: inverse of -z switch.  All items are numbered starting     */
	/*     from firstnumber.                                                     */
	/*   edgesout: -e switch.  voronoi: -v switch.                               */
	/*   neighbors: -n switch.  geomview: -g switch.                             */
	/*   nobound: -B switch.  nopolywritten: -P switch.                          */
	/*   nonodewritten: -N switch.  noelewritten: -E switch.                     */
	/*   noiterationnum: -I switch.  noholes: -O switch.                         */
	/*   noexact: -X switch.                                                     */
	/*   order: element order, specified after -o switch.                        */
	/*   nobisect: count of how often -Y switch is selected.                     */
	/*   steiner: maximum number of Steiner points, specified after -S switch.   */
	/*     steinerleft: number of Steiner points not yet used.                   */
	/*   incremental: -i switch.  sweepline: -F switch.                          */
	/*   dwyer: inverse of -l switch.                                            */
	/*   splitseg: -s switch.                                                    */
	/*   docheck: -C switch.                                                     */
	/*   quiet: -Q switch.  verbose: count of how often -V switch is selected.   */
	/*   useshelles: -p, -r, -q, or -c switch; determines whether shell edges    */
	/*     are used at all.                                                      */
	/*                                                                           */
	/* Read the instructions to find out the meaning of these switches.          */
	
	bool m_poly, refine, quality, vararea, fixedarea, regionattrib, m_convex;
	bool edgesout, voronoi, neighbors, geomview;
	bool nobound, nopolywritten, nonodewritten, noelewritten, noiterationnum;
	bool noholes, noexact;
	bool incremental, sweepline, dwyer;
	bool m_splitSeg;
	bool docheck;
	bool quiet ;
	bool m_useShelles;
	bool nobisect;

	int  m_verbose;
	int  steiner, steinerleft;
	
	int  m_firstnumber;
	int  m_order;
	REAL m_minangle, m_goodangle;
	REAL m_maxarea;
	
	
	/* Triangular bounding box points.                                           */
	//
	triPOINT *m_infpoint1, *m_infpoint2, *m_infpoint3;
	
	/* Pointer to the `TRIANGLE' that occupies all of "outer space".             */
	//虚拟三角形
	TRIANGLE *m_dummytri;
	TRIANGLE *m_dummytribase;      /* Keep base address so we can free() it later. */
	//////////////////
	//added by zzq 2004-04-15
	TRIANGLE *m_Boundarytri;
	
	/* Pointer to the omnipresent shell EDGE.  Referenced by any TRIANGLE or     */
	/*   shell EDGE that isn't really connected to a shell EDGE at that          */
	/*   location.                                                               */
	
	SHELLE *m_dummysh;
	SHELLE *m_dummyshbase;         /* Keep base address so we can free() it later. */
	
	/* Pointer to a recently visited TRIANGLE.  Improves triPOINT location if       */
	/*   proximate points are inserted sequentially.                             */
	triEDGE m_recentTri;
	
private:
	REAL splitter;       /* Used to split REAL factors for exact multiplication. */
	REAL epsilon;                             /* Floating-triPOINT machine epsilon. */
	REAL resulterrbound;
	REAL ccwerrboundA, ccwerrboundB, ccwerrboundC;
	REAL iccerrboundA, iccerrboundB, iccerrboundC;
	
	long incirclecount;                   /* Number of incircle tests performed. */
	long counterclockcount;       /* Number of counterclockwise tests performed. */
	long hyperbolacount;        /* Number of right-of-hyperbola tests performed. */
	long circumcentercount;    /* Number of circumcenter calculations performed. */
	long circletopcount;         /* Number of circle top calculations performed. */
	
private:	
	/*****************************************************************************/
	/*  Mesh manipulation primitives.  Each TRIANGLE contains three pointers to  */
	/*  other triangles, with orientations.  Each pointer points not to the      */
	/*  first byte of a TRIANGLE, but to one of the first three bytes of a       */
	/*  TRIANGLE.  It is necessary to extract both the TRIANGLE itself and the   */
	/*  orientation.  To save memory, I keep both pieces of information in one   */
	/*  pointer.  To make this possible, I assume that all triangles are aligned */
	/*  to four-byte boundaries.  The `decode' routine below decodes a pointer,  */
	/*  extracting an orientation (in the range 0 to 2) and a pointer to the     */
	/*  beginning of a TRIANGLE.  The `encode' routine compresses a pointer to a */
	/*  TRIANGLE and an orientation into a single pointer.  My assumptions that  */
	/*  triangles are four-byte-aligned and that the `unsigned long' type is     */
	/*  long enough to hold a pointer are two of the few kludges in this program.*/
	/*                                                                           */
	/*  Shell edges are manipulated similarly.  A pointer to a shell EDGE        */
	/*  carries both an address and an orientation in the range 0 to 1.          */
	/*                                                                           */
	/*  The other primitives take an oriented TRIANGLE or oriented shell EDGE,   */
	/*  and return an oriented TRIANGLE or oriented shell EDGE or triPOINT; or they */
	/*  change the connections in the data structure.                            */
	/*                                                                           */
	/*****************************************************************************/
	/* prototypes *///原理
	void alternateaxes(triPOINT **sortarray,int arraysize,int axis);//交换轴
	
	void badsegmentdealloc(EDGE *dyingseg);
	EDGE *badsegmenttraverse(void);
	
	void conformingedge(triPOINT * endpoint1,triPOINT * endpoint2,int newmark);
	
	void boundingbox(void);
	
	
	void checkmesh(void);
	void checkdelaunay(void);
	void Check4DeadEvent(triEDGE*checktri,sweepEVENT **freeevents,sweepEVENT **eventheap,long *heapsize);
	
	int checkedge4encroach(EDGE *testedge);
	
	REAL circletop(triPOINT *pa,triPOINT *pb,triPOINT *pc,REAL ccwabc);
	
	void CreateEventHeap(sweepEVENT ***eventheap,sweepEVENT **events,sweepEVENT **freeevents);
	
	long delaunay(void);
	
	void delaunayfixup(triEDGE*fixuptri,int  leftside);
	
	badFACE *dequeuebadtri(void);
	
	long divconqdelaunay(void);
	
	void divconqrecurse(triPOINT **sortarray,int vertices,int axis,triEDGE*farleft,triEDGE*farright);
	void dummyinit(int trianglewords,int shellewords);
	
	void enforcequality(void);
	void enqueuebadtri(triEDGE *instri,REAL angle,triPOINT *insapex,triPOINT *insorg,triPOINT *insdest);
	
	REAL estimate(int elen,REAL *e);
	void eventheapdelete(sweepEVENT **heap,int heapsize,int eventnum);
	void eventHeapInsert(sweepEVENT **heap,int heapsize,sweepEVENT *newevent);
	void eventHeapify(sweepEVENT **heap,int heapsize,int eventnum);
	
	void exactinit(void);
	
	int fast_expansion_sum_zeroelim(int elen,REAL *e,int flen,REAL *f,REAL *h);
	
	enum circumcenterresult findcircumcenter(triPOINT * torg,triPOINT * tdest,triPOINT * tapex,REAL * circumcenter,REAL *xi,REAL *eta);
	
	void constrainededge(triEDGE*starttri,triPOINT *endpoint2,int newmark);
	enum finddirectionresult finddirection(triEDGE*searchtri,triPOINT *endpoint);
	
	void flip(triEDGE*flipedge);
	int formskeleton(long *segmentList,long *segMarkList,int numOfSegments);
	
	triPOINT *GetPoint(int number);
	
	void highorder(void);
	
	REAL incircle(triPOINT * pa,triPOINT * pb,triPOINT * pc,triPOINT * pd);
	REAL incircleadapt(triPOINT * pa,triPOINT * pb,triPOINT * pc,triPOINT * pd,REAL permanent);
	long incrementaldelaunay(void);
	void initializepointpool(void);
	void InitializeTriSegPools(void);
	
	////////////////////////////////////////////
	enum insertsiteresult InsertSite(triPOINT * insertpoint,triEDGE*searchtri,
		EDGE *splitedge,int segmentflaws,int triflaws);
	void DeletesSite(triEDGE*deltri);void insertsegment(triPOINT *endpoint1,triPOINT * endpoint2,int newmark);
	
	void internalerror(void);
	
	enum locateresult locate(triPOINT *searchpoint,triEDGE*searchtri);
	
	void MakePointMap(void);
	void makeshelle(EDGE *newedge);
	void markhull(void);
	void mergehulls(triEDGE*farleft,triEDGE*innerleft,triEDGE*innerright,triEDGE*farright,int axis);
	
	void numbernodes(void);
	void parsecommandline(int argc,char **argv);
	
	void pointmedian(REAL **sortarray,int arraysize,int median,int axis);
	void pointsort(triPOINT **sortarray,int arraysize);
	
	void precisionerror(void);
	enum locateresult PreciseLocate(triPOINT *searchpoint,triEDGE*searchtri);
	void printshelle(EDGE *s);
	
	void regionplague(REAL attribute,REAL area);
	long removeghosts(triEDGE*startghost);
	int rightofhyperbola(triEDGE*fronttri,triPOINT *newsite);
	void quality_statistics(void);
	
	int reconstruct(long *triList,float *triangleattriblist,float *triAreaList,int elements,
		int corners,int attribs,long *segmentList,long *segMarkList,int numOfSegments);
	
	long removebox(void);
	void repairencs(int flaws);
	
	int scale_expansion_zeroelim(int elen,REAL *e,REAL b,REAL *h);
	int scoutsegment(triEDGE*searchtri,triPOINT *endpoint2,int newmark);
	void segmentintersection(triEDGE*splittri,EDGE * splitshelle,triPOINT *endpoint2);
	
	splayNODE *circletopinsert(splayNODE *splayroot,triEDGE*newkey,
		triPOINT *pa,triPOINT *pb,triPOINT *pc,REAL topy);
	
	void shelleDealloc(SHELLE *dyingshelle);
	
	void insertshelle(triEDGE*tri,int shellemark);
	
	SHELLE *shelleTraverse(void);
	splayNODE *splay(splayNODE *splaytreem,triPOINT *searchpoint,triEDGE*searchtri);
	splayNODE *splayinsert(splayNODE *splayroot,triEDGE*newkey,triPOINT *searchpoint);
	void splittriangle(badFACE *badtri);
	
	void statistics(void);
	long SweepLineDelaunay(void);
	
	void triangulatepolygon(triEDGE*firstedge,triEDGE*lastedge,int edgecount,int doflip,int triflaws);
	
	void tallyfaces(void);
	
	void tallyencs(void);
	void testtriangle(triEDGE*testtri);
	
	void triangledeinit(void);
	void triangleinit(void);
	void transfernodes(REAL *pointList,float *pointattriblist,long *pointMarkList,int numOfPoints,int numberofpointattribs);
	void triangleDealloc(TRIANGLE *dyingtriangle);
	void printtriangle(triEDGE *t);
	
	void maketriangle(triEDGE *newtriedge);
	
	/////////////////////////////////////////////////////
	void writenodes(REAL **pointList,float **pointattriblist,long **pointMarkList);
	void writepoly(long **segmentList,long **segMarkList);
	void writeelements(long **triList,float **triangleattriblist);
	void writeneighbors(long **neighborList);
	void writeedges(long **edgeList,long **edgeMarkList);
	void writevoronoi(REAL **vpointList,float **vpointattriblist,long **vpointMarkList,
		long ** vedgeList,long **vedgeMarkList, REAL **vnormList);
	
	////////////////////////////////////////////////////////
	REAL counterclockwiseadapt(triPOINT *pa,triPOINT *pb,triPOINT *pc,REAL detsum);
	REAL counterclockwise(triPOINT * pa,triPOINT * pb,triPOINT * pc);
	
	splayNODE *frontlocate(splayNODE *splayroot,triEDGE*bottommost,
		triPOINT *searchpoint,triEDGE*searchtri,long *farright);
	
	void InfectHull();
	void Plague();

private:
	int  DeScoutSegment(triEDGE*searchtri, triPOINT *endpoint2);
	void RemoveShelle(triEDGE*tri);
	void LocatePoint( triPOINT *searchPoint, triEDGE &searchtri );

public:	// old function for input/construct/output, key for black box of everything
    //黑盒子的接口----------关键函数
	void triangulate(char *triswitches,TinIO *in,TinIO *out,TinIO *vorout);

	///////////////////////// new functions & members //////////////////////////////////////
private:
	bool	m_segmentIntersectionEnabled;

	bool	m_selectedChanged;
	REAL	m_xMinSel, m_yMinSel, m_xMaxSel, m_yMaxSel;

	long	m_maxNumberOfSelectedTriangles;
	long	m_numberOfSelectedTriangles;
	TRIANGLE  **m_selectedTriangles;

public:
	//容许交叉
	void EnableIntersection( bool enable=true )	{ m_segmentIntersectionEnabled = enable; };
	//取出一定范围内的所有三角形
	TRIANGLE  **SelectTriangles(long *numberOfSelectedTriangles, REAL m_xMinSel=0, REAL m_yMinSel=0,REAL m_xMaxSel=0,REAL m_yMaxSel=0);
    //获取数据范围
	void GetRange( REAL *xmin, REAL *ymin, REAL *xmax, REAL *ymax)
	{ 
		*xmin = m_xmin;	*ymin = m_ymin;	*xmax = m_xmax;	*ymax = m_ymax;
	}

public:
	//三角形遍历函数
	void TriangleTraversalInit() { m_triangles.TraversalInit(); };
	TRIANGLE *TriangleTraverse();
	//返回三角形个数
	long GetNumberOfTriangles()	{ return m_triangles.GetNumberOfItems(); };
	//返回虚拟三角形
	TRIANGLE *GetDummyTri()	{ return m_dummytri; };
    //返回虚拟壳
	SHELLE *GetDummySh()	{ return m_dummysh; };
    //点遍历函数
	void PointTraversalInit() { m_points.TraversalInit(); };
	triPOINT *PointTraverse(void);
    //返回点数
	long GetNumberOfPoints()	{ return m_points.GetNumberOfItems(); };
	

protected:	// point handle

	triPOINT *PointAlloc();
	void PointDealloc(triPOINT *dyingpoint);

	// for changeable attributes, it may be structure realized in derived class
	virtual void *AttributeAlloc();
	virtual void AttributeDealloc( void *dyingAttr );

private:

	//boundary points of tin
	bool bAddBoundary;
	triPOINT* m_TinBoundaryPts;
	int m_TinBdPtsNum;
	int m_MaxBdPtsAllocNum;

	triEDGE *m_TinBoundaryEdge;
	int m_TinBdEdgeNum;
	int m_MaxBdEdgeAllocNum;

public:

	/*the following for tin boundary processing*/
	//added by zzq
	triPOINT* zzqGetTinBoundaryPts(int &n);
	bool zzqBoundaryCorrode(triEDGE &hulltri);
	bool zzqMorbidTri(triEDGE hulltri);
    bool zzqSetTinBoundary();
	void zzqSetTinBoundaryState(bool state){bAddBoundary = state;}
	////////////////////////
	//在边界所有点中搜索最靠近当前点的边界点
	triEDGE* zzqGetNearestBoundaryEdge(triPOINT*pt);
	triEDGE* zzqGetTinBoundaryEdge(int&n);
	//////////////////////////////////////////////
    //类创建函数
	CTINClass(char *triswitches);
	virtual ~CTINClass();
    //寻找某点周围的点--返回指向点数组的指针
	triPOINT **GetNeighborPoints(triPOINT *thePoint, int *pSum);
 	/////////////////////////////////////
	//构建三角网之前的初始化、加点工作、收尾等工作
	void BeginAddPoints();
	triPOINT *AddPoint( double x, double y, float attr, long marker=0);
	void EndAddPoints();   
	//////////////////////////////////////
	//插点和删点操作
	triPOINT *InsertPoint(double x, double y, float attr, long marker=0);
	bool RemovePoint(triPOINT *point);

	//////////////////////////////////////
	//插边操作
	void InsertSegment(triPOINT *end1, triPOINT *end2, int boundmarker = 0);
	void InsertSegments(triPOINT **ends, int numberOfSegments, int boundmarker = 0);
    //边界标记
	void MarkHull();
    //删边操作
	void RemoveSegment(triPOINT *end1, triPOINT *end2);

	///////////////////////////////////////
	//快速构建三角网
	void FastConstruct();

	/////////////////////////////////////
	//突壳标记
	void EnableConvex( bool state = true )	{ m_convex = state;	};
	//挖掉边界外部的三角形
	void CarveHoles(REAL *holeList = NULL, int holes = 0, REAL *regionList = NULL, int regions = 0 );
    //保存TIN
	void SaveTIN(const char *fileName, double xOff, double yOff);

	//得到构建的三角网
	//double *xyz	已分配好内存空间的空间点坐标
	//int &ptnum	分配内存的点数，返回实际点数
	//int *v123		已分配好内存的三角形顶点序号
	//int &tinnum	分配内存的三角形个数
	BOOL GetTinData(double *xyz,int &ptnum,int *v123,int &tinnum);

	//////////////////////////////////////////
	//获取三角点
	triPOINT *zzqGetPoint(int num);
	//获取包含点X Y 的三角形
	bool CTINClass::zzqGetTinContainningPt(double x, double y,triEDGE &searchtri);
	bool CTINClass::zzqPt2Tri(triPOINT *insertpoint, triEDGE *searchtri);
};

#endif