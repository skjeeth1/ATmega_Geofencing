#ifndef RAYCAST_H_
#define RAYCAST_H_

#include <stdlib.h>
#include <math.h>

#ifndef HUGE_VAL
#define HUGE_VAL (__builtin_huge_val())
#endif

typedef struct
{
    double x, y;
} vec;

typedef struct
{
    int n;
    vec *v;
} polygon_t, *polygon;

vec vmadd(vec a, double s, vec b);
int intersect(vec x0, vec x1, vec y0, vec y1, double tol, vec *sect);
double dist(vec x, vec y0, vec y1, double tol);
int inside(vec v, polygon p, double tol);

#endif // RAYCAST_H_