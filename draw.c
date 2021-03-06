#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "ml6.h"
#include "display.h"
#include "draw.h"
#include "matrix.h"
#include "math.h"
#include "gmath.h"


/*======== void scanline_convert() ==========
  Inputs: struct matrix *points
          int i
          screen s
          zbuffer zb
  Returns:

  Fills in polygon i by drawing consecutive horizontal (or vertical) lines.

  Color should be set differently for each polygon.
  ====================*/
void scanline_convert( struct matrix *points, int i, screen s, zbuffer zb, color c) {

  int top, mid, bot, y;
  int distance0, distance1, distance2;
  double x0, x1, y0, y1, y2, dx0, dx1, z0, z1, dz0, dz1;
  int flip = 0;

  z0 = z1 = dz0 = dz1 = 0;

  y0 = points->m[1][i];
  y1 = points->m[1][i+1];
  y2 = points->m[1][i+2];

  /* color c; */
  /* c.red = (23 * (i/3))%255; */
  /* c.green = (109 * (i/3))%255; */
  /* c.blue = (c.blue+(227 * (i/3)))%255; */

  //find bot, mid, top
  if ( y0 <= y1 && y0 <= y2) {
    bot = i;
    if (y1 <= y2) {
      mid = i+1;
      top = i+2;
    }
    else {
      mid = i+2;
      top = i+1;
    }
  }//end y0 bottom
  else if (y1 <= y0 && y1 <= y2) {
    bot = i+1;
    if (y0 <= y2) {
      mid = i;
      top = i+2;
    }
    else {
      mid = i+2;
      top = i;
    }
  }//end y1 bottom
  else {
    bot = i+2;
    if (y0 <= y1) {
      mid = i;
      top = i+1;
    }
    else {
      mid = i+1;
      top = i;
    }
  }//end y2 bottom
  //printf("ybot: %0.2f, ymid: %0.2f, ytop: %0.2f\n", (points->m[1][bot]),(points->m[1][mid]), (points->m[1][top]));
  /* printf("bot: (%0.2f, %0.2f, %0.2f) mid: (%0.2f, %0.2f, %0.2f) top: (%0.2f, %0.2f, %0.2f)\n", */

  x0 = points->m[0][bot];
  x1 = points->m[0][bot];
  z0 = points->m[2][bot];
  z1 = points->m[2][bot];
  y = (int)(points->m[1][bot]);

  distance0 = (int)(points->m[1][top]) - y;
  distance1 = (int)(points->m[1][mid]) - y;
  distance2 = (int)(points->m[1][top]) - (int)(points->m[1][mid]);

  //printf("distance0: %d distance1: %d distance2: %d\n", distance0, distance1, distance2);
  dx0 = distance0 > 0 ? (points->m[0][top]-points->m[0][bot])/distance0 : 0;
  dx1 = distance1 > 0 ? (points->m[0][mid]-points->m[0][bot])/distance1 : 0;
  dz0 = distance0 > 0 ? (points->m[2][top]-points->m[2][bot])/distance0 : 0;
  dz1 = distance1 > 0 ? (points->m[2][mid]-points->m[2][bot])/distance1 : 0;

  while ( y <= (int)points->m[1][top] ) {
    //printf("\tx0: %0.2f x1: %0.2f y: %d\n", x0, x1, y);
    draw_line(x0, y, z0, x1, y, z1, s, zb, c);

    x0+= dx0;
    x1+= dx1;
    z0+= dz0;
    z1+= dz1;
    y++;

    if ( !flip && y >= (int)(points->m[1][mid]) ) {
      flip = 1;
      dx1 = distance2 > 0 ? (points->m[0][top]-points->m[0][mid])/distance2 : 0;
      dz1 = distance2 > 0 ? (points->m[2][top]-points->m[2][mid])/distance2 : 0;
      x1 = points->m[0][mid];
      z1 = points->m[2][mid];
    }//end flip code
  }//end scanline loop
}

/*======== void add_polygon() ==========
  Inputs:   struct matrix *surfaces
  double x0
  double y0
  double z0
  double x1
  double y1
  double z1
  double x2
  double y2
  double z2
  Returns:
  Adds the vertices (x0, y0, z0), (x1, y1, z1)
  and (x2, y2, z2) to the polygon matrix. They
  define a single triangle surface.
  ====================*/
void add_polygon( struct matrix *polygons,
                  double x0, double y0, double z0,
                  double x1, double y1, double z1,
                  double x2, double y2, double z2 ) {

  add_point(polygons, x0, y0, z0);
  add_point(polygons, x1, y1, z1);
  add_point(polygons, x2, y2, z2);
}

/*======== void draw_polygons() ==========
  Inputs:   struct matrix *polygons
  screen s
  color c
  Returns:
  Goes through polygons 3 points at a time, drawing
  lines connecting each points to create bounding
  triangles
  ====================*/
void draw_polygons(struct matrix *polygons, screen s, zbuffer zb,
                   double *view, double light[2][3], color ambient,
                   double *areflect,
                   double *dreflect,
                   double *sreflect) {
  if ( polygons->lastcol < 3 ) {
    printf("Need at least 3 points to draw a polygon!\n");
    return;
  }

  int point;
  double *normal;

  for (point=0; point < polygons->lastcol-2; point+=3) {

    normal = calculate_normal(polygons, point);

    if ( dot_product(normal, view) > 0 ) {

      color c = get_lighting(normal, view, ambient, light, areflect, dreflect, sreflect);

      scanline_convert(polygons, point, s, zb, c);

      draw_line( polygons->m[0][point],
                 polygons->m[1][point],
                 polygons->m[2][point],
                 polygons->m[0][point+1],
                 polygons->m[1][point+1],
                 polygons->m[2][point+1],
                 s, zb, c);
      draw_line( polygons->m[0][point+2],
                 polygons->m[1][point+2],
                 polygons->m[2][point+2],
                 polygons->m[0][point+1],
                 polygons->m[1][point+1],
                 polygons->m[2][point+1],
                 s, zb, c);
      draw_line( polygons->m[0][point],
                 polygons->m[1][point],
                 polygons->m[2][point],
                 polygons->m[0][point+2],
                 polygons->m[1][point+2],
                 polygons->m[2][point+2],
                 s, zb, c);
    }
  }
}
/* HELPER FUNCTIONS START */
int get_id(double points[]){
    int id = 0;
    for (int i=0; i<3;++i)
        id += floor(points[i]) * (int)pow(10, 3 * i);
	return id;
}
void set_intensities(struct vertex_normal ** vn, double *view, 
                    double light[2][3], color ambient, 
                    double *areflect, double *dreflect, 
                    double *sreflect)
{
	struct vertex_normal* v;
	 for (v=*vn; v!=NULL; v=v->hh.next){
		v->c = get_lighting(v->norm, view, ambient, light, 
                            areflect, dreflect, sreflect);
      // printf("%f %f %f\n", v->norm[0], v->norm[1], v->norm[2]);
        if ( dot_product(v->norm, view) > 0 ) {
     //       printf("%d %d %d\n", v->c.red, v->c.blue, v->c.green);
        }
    }	
}
void append(struct vertex_normal **vn, struct matrix **points, int index, int vertex)
{
	struct vertex_normal *v = (struct vertex_normal *)
            malloc(sizeof(struct vertex_normal));
    v->id = vertex;
    v->norm = calculate_normal(*points, index);
    normalize(v->norm);
	HASH_ADD_INT(*vn, id, v);
 }
void modify(struct vertex_normal **vn, struct matrix ** points, int index)
{
    //printf("%f\n", v->norm[0]);
    double * addend = calculate_normal(*points, index);
    normalize(addend);
	for (int i=0;i<3;++i)        
        (*vn)->norm[i] += addend[i];
}
void find_positions(double y[], int pos[], int i )
{//y[i] = y(i), pos[0] = bot, pos[2] = top
	if (y[0] < y[1] && y[0] <= y[2]){
		pos[0] = i;
		if (y[1] <= y[2]){
			pos[1] = i+1;
			pos[2] = i+2;
		}
		else{
			pos[1] = i+2;
			pos[2] = i+1;
		}
	}
	else if (y[1] <= y[0] && y[1] <= y[2]){
		pos[0] = i+1;
		if (y[0] <= y[2]) {
			pos[1] = i;
		    pos[2] = i+2;
		}
		else {
			pos[1] = i+2;
			pos[2] = i;
		}
	}
	else {
		pos[0] = i+2;
		if (y[0] <= y[1]) {
			pos[1] = i;
			pos[2] = i+1;
		}
	    else {
			pos[1] = i+1;
			pos[2] = i;
		}
	}
    //if (!(y[pos[0]] <= y[pos[1]] && y[pos[1]] <= y[pos[2]]))
    //  printf("find_position failed");
}
void find_distances(double distance[], struct matrix **mat, int pos[]) 
{
	struct matrix *points = *mat;
	distance[0] = (int)(points->m[1][pos[2]]) - (int)points->m[1][pos[0]];
	distance[1] = (int)(points->m[1][pos[1]]) - (int)points->m[1][pos[0]];
	distance[2] = (int)(points->m[1][pos[2]]) - (int)(points->m[1][pos[1]]);
   // printf("%f %f %f\n", distance[0], distance[1], distance[2]);
}
void find_deltas(double distance[], struct matrix **mat, int pos[], color intensity[], double *dx0, double *dx1, double *dz0, double *dz1, double di0[], double di1[]) 
{
  struct matrix *points = *mat;
  *dx0 = distance[0] > 0 ? (points->m[0][pos[2]]-points->m[0][pos[0]])/distance[0] : 0;
  *dx1 = distance[1] > 0 ? (points->m[0][pos[1]]-points->m[0][pos[0]])/distance[1] : 0;
  *dz0 = distance[0] > 0 ? (points->m[2][pos[2]]-points->m[2][pos[0]])/distance[0] : 0;
  *dz1 = distance[1] > 0 ? (points->m[2][pos[1]]-points->m[2][pos[0]])/distance[1] : 0;
 // for (int i = 0; i < 3 ; ++i)
//      printf("%d : %d %d %d\n", i, intensity[i].red, intensity[i].green, intensity[i].blue);
  (di0)[0] = distance[0] > 0 ? ((intensity[2]).red - (intensity[0]).red) / distance[0] : 0;  
  (di1)[0] = distance[1] > 0 ? ((intensity[1]).red - (intensity[0]).red) / distance[1] : 0;
  (di0)[1] = distance[0] > 0 ? ((intensity[2]).green - (intensity[0]).green) / distance[0] : 0;  
  (di1)[1] = distance[1] > 0 ? ((intensity[1]).green - (intensity[0]).green) / distance[1] : 0;
  (di0)[2] = distance[0] > 0 ? ((intensity[2]).blue - (intensity[0]).blue) / distance[0] : 0;  
  (di1)[2] = distance[1] > 0 ? ((intensity[1]).blue - (intensity[0]).blue) / distance[1] : 0;
//  printf("%f %f %f \n", di1[0], di1[1], di1[2]);
  //printf("%f %f %f \n", di0[0], di0[1], di0[2]);
}
void find_intensities(struct vertex_normal **vn, struct matrix **points, int pos[], int point, color intensity[])
{
    for (int p=0; p<3; ++p){
        double tmp[3] = {(*points)->m[0][pos[p]], 
        (*points)->m[1][pos[p]], (*points)->m[2][pos[p]]};
        int id = get_id(tmp);
        struct vertex_normal * v;
        HASH_FIND_INT(*vn, &id, v); 
        intensity[p] = v->c;
        // printf("%d %d %d\n", v->c.red, v->c.blue, v->c.green);
   } 
}
void draw_gouraud_lines(int x0, int y, double z0,
               int x1, double z1,
               color i0, color i1,
               screen s, zbuffer zb)
{
    if (x0 > x1){
    //swap x0, x1, z0, z1, and i1 and i2 
        int xt;
        double z;
        color it;
        xt = x0;
        it = i0;
        z = z0;
        x0 = x1;
        i0 = i1;
        z0 = z1;
        x1 = xt;
        i1 = it;
        z1 = z;
    }
    //gotta change colors
    double dx = x1 - x0;
    double dz = (z1 - z0) / dx; 
    double di[3];
    //printf("%d\n", i1.red - i0.red);
    di[0] = (double)(i1.red - i0.red) / dx;
    di[1] = (double)(i1.green - i0.green) / dx;
    di[2] = (double)(i1.blue - i0.blue) /dx;
//    printf("%f %f %f \n", di[0], di[1], di[2]);
    for (int i = x0; i < x1; ++i){
        double z = z0 + dz * (i - x0); 
        color c;
        c.red = i0.red + (int)(di[0] * (i - x0));
        c.green = i0.green + (int)(di[1] * (i - x0));
        c.blue = i0.blue + (int)(di[2] * (i - x0));
        plot( s, zb, c, i, y, z ); //i in this case is x val
    }
}
void change_deltas(int *flip, double distance[], color intensity[], int pos[], struct matrix **points, 
                    double *dx1, double *dz1, double di1[], double *x1, double *z1, color *c1 )
{
    *flip = 1;
    *dx1 = distance[2] > 0 ? ((*points)->m[0][pos[2]]-(*points)->m[0][pos[1]])/distance[2] : 0;
    *dz1 = distance[2] > 0 ? ((*points)->m[2][pos[2]]-(*points)->m[2][pos[1]])/distance[2] : 0;
    (di1)[0] = distance[2] > 0 ? ((intensity[2]).red - (intensity[1]).red)/distance[2] : 0;
    (di1)[1] = distance[2] > 0 ? ((intensity[2]).green - (intensity[1]).green)/distance[2] : 0;
    (di1)[2] = distance[2] > 0 ? ((intensity[2]).blue - (intensity[1]).blue)/distance[2] : 0;
    *x1 = ((*points))->m[0][pos[1]];
    *z1 = ((*points))->m[2][pos[1]];
    *c1 = intensity[1]; 
}
 /*update_values(double *x0, double *x1, double *z0, double *z1, 
      color *c0, double dx0, double dx1, double dz1,
      double di0[], double di1[])*/

/* HELPER FUNCTIONS END */
void draw_gouraud(struct matrix * points, screen s, zbuffer zb,
		  double *view, double light[2][3], color ambient,
		  double *areflect, double *dreflect, double *sreflect)
{
    //draw_gouraud interpolates normals
	struct vertex_normal *vn = NULL; 	
	for (int point=0; point < points->lastcol; ++point) {
		double pa[3] = {points->m[0][point], 
		points->m[1][point], points->m[2][point]};
	    int vertex = get_id(pa);	
		struct vertex_normal *v;	
		HASH_FIND_INT(vn, &vertex, v); 
		if (v==NULL)
			append(&vn, &points, point - point % 3, vertex);
		else
            modify(&v, &points, point - point % 3);
	}
	set_intensities(&vn, view, light, ambient, areflect, dreflect, sreflect);	

    //plotting the points
	for (int point = 0; point < points->lastcol-2; point+=3){
		double *normal = calculate_normal(points, point);
		if ( dot_product(normal, view) > 0 ) {
			double y[3] = {points->m[1][point],points->m[1][point+1],points->m[1][point+2]};
			int pos[3]; //pos[0] = bot, pos[1] = mid, pos[2] = top
			find_positions(y, pos, point);
            double x0, x1, z0, z1, dx0, dx1, dz0, dz1;
            color c0, c1;
            double di0[3];
            double di1[3];
		    x0 = points->m[0][pos[0]], x1 = points->m[0][pos[0]];
            z0 = points->m[2][pos[0]], z1 = points->m[2][pos[0]];
			int yindex = (int)(points->m[1][pos[0]]);
			double distance[3] = {}; 
            find_distances(distance, &points, pos);
            color intensity[3]; 
            find_intensities(&vn, &points, pos, point, intensity);
            c0 = intensity[0];
            c1 = intensity[0];
            find_deltas(distance, &points, pos, intensity, &dx0, &dx1, &dz0, &dz1, di0, di1); 
            int flip = 0;
            while (yindex <= (int)points->m[1][pos[2]]){
                draw_gouraud_lines(x0, yindex, z0, x1, z1, c0, c1, s, zb);  
                x0+= dx0, x1+= dx1;
                z0+= dz0, z1+= dz1;
                c0.red += (int)di0[0] , c1.red += (int)di1[0];
                c0.green += (int)di0[1], c1.green += (int)di1[1];
                c0.blue += (int)di0[2], c1.blue += (int)di1[2];
                //printf("%d %d %d \n", c0.red, c0.blue, c0.green);
                ++yindex; 
                if ( !flip && yindex >= (int)(points->m[1][pos[1]]) ) { //if its flipped and past the middle
                    change_deltas(&flip, distance, intensity, pos, &points, &dx1, &dz1, di1, &x1, &z1, &c1);
                } 
            }
		}
	}
}
void find_normals(struct vertex_normal **vn, struct matrix **points, int pos[],
        int point, double normals[3][3])
{
    for (int p=0; p<3; ++p){
        double tmp[3] = {(*points)->m[0][pos[p]], 
        (*points)->m[1][pos[p]], (*points)->m[2][pos[p]]};
        int id = get_id(tmp);
        struct vertex_normal * v;
        HASH_FIND_INT(*vn, &id, v); 
        //they are already normalized
        for (int i = 0; i < 3; ++i)
            normals[p][i] = v->norm[i]; 
   } 
}
void find_phong_deltas(double distance[], struct matrix **mat, int pos[], double
        normals[3][3], double *dx0, double *dx1, double *dz0, double *dz1, double
        **dn0, double **dn1) 
{
  struct matrix *points = *mat;
  *dx0 = distance[0] > 0 ? (points->m[0][pos[2]]-points->m[0][pos[0]])/distance[0] : 0;
  *dx1 = distance[1] > 0 ? (points->m[0][pos[1]]-points->m[0][pos[0]])/distance[1] : 0;
  *dz0 = distance[0] > 0 ? (points->m[2][pos[2]]-points->m[2][pos[0]])/distance[0] : 0;
  *dz1 = distance[1] > 0 ? (points->m[2][pos[1]]-points->m[2][pos[0]])/distance[1] : 0;
 // for (int i = 0; i < 3 ; ++i)
//      printf("%d : %d %d %d\n", i, intensity[i].red, intensity[i].green, intensity[i].blue);
  for (int i = 0; i < 3; ++i){
      (*dn0)[i] = distance[0] > 0 ? (normals[2][i] - normals[0][i]) / distance[0] :
          0;
      (*dn1)[i] = distance[1] > 0 ? (normals[1][i] - normals[0][i]) / distance[1] :
          0;
  }
}
void draw_phong_lines(int x0, int y, double z0,
               int x1, double z1,
               double ** n0, double ** n1,
               screen s, zbuffer zb, double *view, 
               double light[2][3], color ambient, 
               double *areflect, double *dreflect, 
               double *sreflect)
{ //maybe its not pointers thats the issue?
  if (x0 > x1){
    //swap x0, x1, z0, z1, and n1 and n2 
        int xt;
        double z;
        double * nt = (double *)malloc(sizeof(double));
        xt = x0;
        nt = *n0;
        z = z0;
        x0 = x1;
        n0 = n1;
        z0 = z1;
        x1 = xt;
        n1 = &nt;
        z1 = z;
    }
    double dx = x1 - x0;
    double dz = (z1 - z0) / dx; 


    double *dn = (double *)malloc(sizeof(double));
    for (int i = 0; i < 3; ++i)
        dn[i] = ((*n1)[i] - (*n0)[i]) / (x1 - x0);
    for (int i = x0; i < x1; ++i){
        double z = z0 + dz * (i - x0); 
        double * n = (double *)malloc(sizeof(double)); 
        for (int j = 0 ; j < 3; ++j)
            n[j] = (*n0)[j] + dn[j] * (i - x0);
        normalize(n);
        //dn = 0! BIG ERROR FIX
        //i think its because the values passed are the same
        color c = get_lighting(n, view, ambient, light, areflect, dreflect, sreflect);
        plot( s, zb, c, i, y, z ); //i in this case is x val
        free(n);
    }
}

void change_phong_deltas(int *flip, double distance[], double normals[3][3],int pos[], struct matrix **points, 
                    double *dx1, double *dz1, double **dn1, double *x1, double
                    *z1, double ** n1 )
{
    *flip = 1;
    *dx1 = distance[2] > 0 ? ((*points)->m[0][pos[2]]-(*points)->m[0][pos[1]])/distance[2] : 0;
    *dz1 = distance[2] > 0 ? ((*points)->m[2][pos[2]]-(*points)->m[2][pos[1]])/distance[2] : 0;
    for (int i = 0; i < 3; ++i)
        (*dn1)[i] = distance[2] > 0 ? ( normals[2][i] - normals[1][i] ) /
            distance[2] : 0;
    *x1 = ((*points))->m[0][pos[1]];
    *z1 = ((*points))->m[2][pos[1]];
    for (int i = 0; i < 3; ++i)
        (*n1)[i] = normals[1][i];
}
void draw_phong(struct matrix * points, screen s, zbuffer zb,
		  double *view, double light[2][3], color ambient,
		  double *areflect, double *dreflect, double *sreflect)
{
    //same start as gouraud, just find the normals from each vertex
    struct vertex_normal *vn = NULL; 	
	for (int point=0; point < points->lastcol; ++point) {
		double pa[3] = {points->m[0][point], 
		points->m[1][point], points->m[2][point]};
	    int vertex = get_id(pa);	
		struct vertex_normal *v;	
		HASH_FIND_INT(vn, &vertex, v); 
		if (v==NULL)
			append(&vn, &points, point - point % 3, vertex);
		else
            modify(&v, &points, point - point % 3);
	}
    struct vertex_normal* v;
    for (v=vn; v!=NULL; v=v->hh.next)
        normalize(v->norm);

    //but it deviates here, because you don't calculate the intensities yet
    //deviates w.r.t. the fact that you don't call set_intensities

    for (int point = 0; point < points->lastcol-2; point+=3){
		double *normal = calculate_normal(points, point);
		if ( dot_product(normal, view) > 0 ) {
			double y[3] = {points->m[1][point],points->m[1][point+1],points->m[1][point+2]};
			int pos[3]; //pos[0] = bot, pos[1] = mid, pos[2] = top
			find_positions(y, pos, point);
            double x0, x1, z0, z1, dx0, dx1, dz0, dz1;
		    x0 = points->m[0][pos[0]], x1 = points->m[0][pos[0]];
            z0 = points->m[2][pos[0]], z1 = points->m[2][pos[0]];
			int yindex = (int)(points->m[1][pos[0]]);
			double distance[3] = {}; 
            find_distances(distance, &points, pos);
            double normals[3][3]; 
            find_normals(&vn, &points, pos, point, normals);
            // normals 2D array does not contain the same normals confirmed!
            double * n0 = (double *)malloc(sizeof(double));
            double * n1 = (double *)malloc(sizeof(double));
            double * dn1 = (double *)malloc(sizeof(double));
            double * dn0 = (double *)malloc(sizeof(double));
            find_phong_deltas(distance, &points, pos, normals, &dx0, &dx1, &dz0,
                    &dz1, &dn0, &dn1);
            // dn0 != dn1 confirmed!
            int flip = 0;
            while (yindex <= (int)points->m[1][pos[2]]){
                draw_phong_lines(x0, yindex, z0, x1, z1, &n0, &n1, s, zb, view, light, ambient, areflect, dreflect, sreflect);  
                x0+= dx0, x1+= dx1;
                z0+= dz0, z1+= dz1;
                for (int i = 0; i < 3; ++i){
                    //normals are now different
                    n0[i] += dn0[i];
                    n1[i] += dn1[i];
                }
                ++yindex; 
                if ( !flip && yindex >= (int)(points->m[1][pos[1]]) ) { //if its flipped and past the middle
                    change_phong_deltas(&flip, distance, normals, pos, &points,
                            &dx1, &dz1, &dn1, &x1, &z1, &n1);
                } 
            }
            free(dn0);
            free(dn1);
        }	
    }
}


/*======== void add_box() ==========
  Inputs:   struct matrix * edges
  double x
  double y
  double z
  double width
  double height
  double depth
  Returns:
  add the points for a rectagular prism whose
  upper-left corner is (x, y, z) with width,
  height and depth dimensions.
  ====================*/
void add_box( struct matrix * polygons,
              double x, double y, double z,
              double width, double height, double depth ) {

  double x1, y1, z1;
  x1 = x+width;
  y1 = y-height;
  z1 = z-depth;

  //front
  add_polygon(polygons, x, y, z, x1, y1, z, x1, y, z);
  add_polygon(polygons, x, y, z, x, y1, z, x1, y1, z);

  //back
  add_polygon(polygons, x1, y, z1, x, y1, z1, x, y, z1);
  add_polygon(polygons, x1, y, z1, x1, y1, z1, x, y1, z1);

  //right side
  add_polygon(polygons, x1, y, z, x1, y1, z1, x1, y, z1);
  add_polygon(polygons, x1, y, z, x1, y1, z, x1, y1, z1);
  //left side
  add_polygon(polygons, x, y, z1, x, y1, z, x, y, z);
  add_polygon(polygons, x, y, z1, x, y1, z1, x, y1, z);

  //top
  add_polygon(polygons, x, y, z1, x1, y, z, x1, y, z1);
  add_polygon(polygons, x, y, z1, x, y, z, x1, y, z);
  //bottom
  add_polygon(polygons, x, y1, z, x1, y1, z1, x1, y1, z);
  add_polygon(polygons, x, y1, z, x, y1, z1, x1, y1, z1);
}//end add_box

/*======== void add_sphere() ==========
  Inputs:   struct matrix * points
  double cx
  double cy
  double cz
  double r
  double step
  Returns:

  adds all the points for a sphere with center
  (cx, cy, cz) and radius r.

  should call generate_sphere to create the
  necessary points
  ====================*/
void add_sphere( struct matrix * edges,
                 double cx, double cy, double cz,
                 double r, int step ) {

  struct matrix *points = generate_sphere(cx, cy, cz, r, step);

  int p0, p1, p2, p3, lat, longt;
  int latStop, longStop, latStart, longStart;
  latStart = 0;
  latStop = step;
  longStart = 0;
  longStop = step;

  step++;
  for ( lat = latStart; lat < latStop; lat++ ) {
    for ( longt = longStart; longt < longStop; longt++ ) {

      p0 = lat * (step) + longt;
      p1 = p0+1;
      p2 = (p1+step) % (step * (step-1));
      p3 = (p0+step) % (step * (step-1));

      //printf("p0: %d\tp1: %d\tp2: %d\tp3: %d\n", p0, p1, p2, p3);
      if (longt < step - 2)
        add_polygon( edges, points->m[0][p0],
                     points->m[1][p0],
                     points->m[2][p0],
                     points->m[0][p1],
                     points->m[1][p1],
                     points->m[2][p1],
                     points->m[0][p2],
                     points->m[1][p2],
                     points->m[2][p2]);
      if (longt > 0 )
        add_polygon( edges, points->m[0][p0],
                     points->m[1][p0],
                     points->m[2][p0],
                     points->m[0][p2],
                     points->m[1][p2],
                     points->m[2][p2],
                     points->m[0][p3],
                     points->m[1][p3],
                     points->m[2][p3]);
    }
  }
  free_matrix(points);
}

/*======== void generate_sphere() ==========
  Inputs:   double cx
  double cy
  double cz
  double r
  int step
  Returns: Generates all the points along the surface
  of a sphere with center (cx, cy, cz) and
  radius r.
  Returns a matrix of those points
  ====================*/
struct matrix * generate_sphere(double cx, double cy, double cz,
                                double r, int step ) {

  struct matrix *points = new_matrix(4, step * step);
  int circle, rotation, rot_start, rot_stop, circ_start, circ_stop;
  double x, y, z, rot, circ;

  rot_start = 0;
  rot_stop = step;
  circ_start = 0;
  circ_stop = step;

  for (rotation = rot_start; rotation < rot_stop; rotation++) {
    rot = (double)rotation / step;

    for(circle = circ_start; circle <= circ_stop; circle++){
      circ = (double)circle / step;

      x = r * cos(M_PI * circ) + cx;
      y = r * sin(M_PI * circ) *
        cos(2*M_PI * rot) + cy;
      z = r * sin(M_PI * circ) *
        sin(2*M_PI * rot) + cz;

      /* printf("rotation: %d\tcircle: %d\n", rotation, circle); */
      /* printf("rot: %lf\tcirc: %lf\n", rot, circ); */
      /* printf("sphere point: (%0.2f, %0.2f, %0.2f)\n\n", x, y, z); */
      add_point(points, x, y, z);
    }
  }

  return points;
}

/*======== void add_torus() ==========
  Inputs:   struct matrix * points
  double cx
  double cy
  double cz
  double r1
  double r2
  double step
  Returns:

  adds all the points required to make a torus
  with center (cx, cy, cz) and radii r1 and r2.

  should call generate_torus to create the
  necessary points
  ====================*/
void add_torus( struct matrix * edges,
                double cx, double cy, double cz,
                double r1, double r2, int step ) {

  struct matrix *points = generate_torus(cx, cy, cz, r1, r2, step);

  int p0, p1, p2, p3, lat, longt;
  int latStop, longStop, latStart, longStart;
  latStart = 0;
  latStop = step;
  longStart = 0;
  longStop = step;

  //printf("points: %d\n", points->lastcol);

  for ( lat = latStart; lat < latStop; lat++ ) {
    for ( longt = longStart; longt < longStop; longt++ ) {
      p0 = lat * step + longt;
      if (longt == step - 1)
        p1 = p0 - longt;
      else
        p1 = p0 + 1;
      p2 = (p1 + step) % (step * step);
      p3 = (p0 + step) % (step * step);

      //printf("p0: %d\tp1: %d\tp2: %d\tp3: %d\n", p0, p1, p2, p3);
      add_polygon( edges, points->m[0][p0],
                   points->m[1][p0],
                   points->m[2][p0],
                   points->m[0][p3],
                   points->m[1][p3],
                   points->m[2][p3],
                   points->m[0][p2],
                   points->m[1][p2],
                   points->m[2][p2]);
      add_polygon( edges, points->m[0][p0],
                   points->m[1][p0],
                   points->m[2][p0],
                   points->m[0][p2],
                   points->m[1][p2],
                   points->m[2][p2],
                   points->m[0][p1],
                   points->m[1][p1],
                   points->m[2][p1]);
    }
  }
  free_matrix(points);
}
/*======== void generate_torus() ==========
  Inputs:   struct matrix * points
  double cx
  double cy
  double cz
  double r
  int step
  Returns: Generates all the points along the surface
  of a torus with center (cx, cy, cz) and
  radii r1 and r2.
  Returns a matrix of those points
  ====================*/
struct matrix * generate_torus( double cx, double cy, double cz,
                                double r1, double r2, int step ) {

  struct matrix *points = new_matrix(4, step * step);
  int circle, rotation, rot_start, rot_stop, circ_start, circ_stop;
  double x, y, z, rot, circ;

  rot_start = 0;
  rot_stop = step;
  circ_start = 0;
  circ_stop = step;

  for (rotation = rot_start; rotation < rot_stop; rotation++) {
    rot = (double)rotation / step;

    for(circle = circ_start; circle < circ_stop; circle++){
      circ = (double)circle / step;

      x = cos(2*M_PI * rot) *
        (r1 * cos(2*M_PI * circ) + r2) + cx;
      y = r1 * sin(2*M_PI * circ) + cy;
      z = -1*sin(2*M_PI * rot) *
        (r1 * cos(2*M_PI * circ) + r2) + cz;

      //printf("rotation: %d\tcircle: %d\n", rotation, circle);
      //printf("torus point: (%0.2f, %0.2f, %0.2f)\n", x, y, z);
      add_point(points, x, y, z);
    }
  }
  return points;
}

/*======== void add_circle() ==========
  Inputs:   struct matrix * points
  double cx
  double cy
  double r
  double step
  Returns:

  Adds the circle at (cx, cy) with radius r to edges
  ====================*/
void add_circle( struct matrix * edges,
                 double cx, double cy, double cz,
                 double r, int step ) {
  double x0, y0, x1, y1, t;
  int i;
  x0 = r + cx;
  y0 = cy;

  for (i=1; i<=step; i++) {
    t = (double)i/step;
    x1 = r * cos(2 * M_PI * t) + cx;
    y1 = r * sin(2 * M_PI * t) + cy;

    add_edge(edges, x0, y0, cz, x1, y1, cz);
    x0 = x1;
    y0 = y1;
  }
}


/*======== void add_curve() ==========
  Inputs:   struct matrix *points
  double x0
  double y0
  double x1
  double y1
  double x2
  double y2
  double x3
  double y3
  double step
  int type
  Returns:

  Adds the curve bounded by the 4 points passsed as parameters
  of type specified in type (see matrix.h for curve type constants)
  to the matrix points
  ====================*/
void add_curve( struct matrix *edges,
                double x0, double y0,
                double x1, double y1,
                double x2, double y2,
                double x3, double y3,
                int step, int type ) {

  double t, x, y;
  struct matrix *xcoefs;
  struct matrix *ycoefs;
  int i;

  xcoefs = generate_curve_coefs(x0, x1, x2, x3, type);
  ycoefs = generate_curve_coefs(y0, y1, y2, y3, type);

  /* print_matrix(xcoefs); */
  /* printf("\n"); */
  /* print_matrix(ycoefs); */

  for (i=1; i<=step; i++) {

    t = (double)i/step;
    x = xcoefs->m[0][0] *t*t*t + xcoefs->m[1][0] *t*t+
      xcoefs->m[2][0] *t + xcoefs->m[3][0];
    y = ycoefs->m[0][0] *t*t*t + ycoefs->m[1][0] *t*t+
      ycoefs->m[2][0] *t + ycoefs->m[3][0];

    add_edge(edges, x0, y0, 0, x, y, 0);
    x0 = x;
    y0 = y;
  }

  free_matrix(xcoefs);
  free_matrix(ycoefs);
}


/*======== void add_point() ==========
  Inputs:   struct matrix * points
  int x
  int y
  int z
  Returns:
  adds point (x, y, z) to points and increment points.lastcol
  if points is full, should call grow on points
  ====================*/
void add_point( struct matrix * points, double x, double y, double z) {

  if ( points->lastcol == points->cols )
    grow_matrix( points, points->lastcol + 100 );

  points->m[0][ points->lastcol ] = x;
  points->m[1][ points->lastcol ] = y;
  points->m[2][ points->lastcol ] = z;
  points->m[3][ points->lastcol ] = 1;
  points->lastcol++;
} //end add_point

/*======== void add_edge() ==========
  Inputs:   struct matrix * points
  int x0, int y0, int z0, int x1, int y1, int z1
  Returns:
  add the line connecting (x0, y0, z0) to (x1, y1, z1) to points
  should use add_point
  ====================*/
void add_edge( struct matrix * points,
               double x0, double y0, double z0,
               double x1, double y1, double z1) {
  add_point( points, x0, y0, z0 );
  add_point( points, x1, y1, z1 );
}

/*======== void draw_lines() ==========
  Inputs:   struct matrix * points
  screen s
  color c
  Returns:
  Go through points 2 at a time and call draw_line to add that line
  to the screen
  ====================*/
void draw_lines( struct matrix * points, screen s, zbuffer zb, color c) {

  if ( points->lastcol < 2 ) {
    printf("Need at least 2 points to draw a line!\n");
    return;
  }
  int point;
  for (point=0; point < points->lastcol-1; point+=2)
    draw_line( points->m[0][point],
               points->m[1][point],
               points->m[2][point],
               points->m[0][point+1],
               points->m[1][point+1],
               points->m[2][point+1],
               s, zb, c);
}// end draw_lines




void draw_line(int x0, int y0, double z0,
               int x1, int y1, double z1,
               screen s, zbuffer zb, color c) {


  int x, y, d, A, B;
  int dy_east, dy_northeast, dx_east, dx_northeast, d_east, d_northeast;
  int loop_start, loop_end;
  double distance;
  double z, dz;

  //swap points if going right -> left
  int xt, yt;
  if (x0 > x1) {
    xt = x0;
    yt = y0;
    z = z0;
    x0 = x1;
    y0 = y1;
    z0 = z1;
    x1 = xt;
    y1 = yt;
    z1 = z;
  }

  x = x0;
  y = y0;
  A = 2 * (y1 - y0);
  B = -2 * (x1 - x0);
  int wide = 0;
  int tall = 0;
  //octants 1 and 8
  if ( abs(x1 - x0) >= abs(y1 - y0) ) { //octant 1/8
    wide = 1;
    loop_start = x;
    loop_end = x1;
    dx_east = dx_northeast = 1;
    dy_east = 0;
    d_east = A;
    distance = x1 - x;
    if ( A > 0 ) { //octant 1
      d = A + B/2;
      dy_northeast = 1;
      d_northeast = A + B;
    }
    else { //octant 8
      d = A - B/2;
      dy_northeast = -1;
      d_northeast = A - B;
    }
  }//end octant 1/8
  else { //octant 2/7
    tall = 1;
    dx_east = 0;
    dx_northeast = 1;
    distance = abs(y1 - y);
    if ( A > 0 ) {     //octant 2
      d = A/2 + B;
      dy_east = dy_northeast = 1;
      d_northeast = A + B;
      d_east = B;
      loop_start = y;
      loop_end = y1;
    }
    else {     //octant 7
      d = A/2 - B;
      dy_east = dy_northeast = -1;
      d_northeast = A - B;
      d_east = -1 * B;
      loop_start = y1;
      loop_end = y;
    }
  }

  z = z0;
  dz = (z1 - z0) / distance;
  //printf("\t(%d, %d) -> (%d, %d)\tdistance: %0.2f\tdz: %0.2f\tz: %0.2f\n", x0, y0, x1, y1, distance, dz, z);

  while ( loop_start < loop_end ) {

    plot( s, zb, c, x, y, z );
    if ( (wide && ((A > 0 && d > 0) ||
                   (A < 0 && d < 0)))
         ||
         (tall && ((A > 0 && d < 0 ) ||
                   (A < 0 && d > 0) ))) {
      y+= dy_northeast;
      d+= d_northeast;
      x+= dx_northeast;
    }
    else {
      x+= dx_east;
      y+= dy_east;
      d+= d_east;
    }
    z+= dz;
    loop_start++;
  } //end drawing loop
  plot( s, zb, c, x1, y1, z );
} //end draw_line
