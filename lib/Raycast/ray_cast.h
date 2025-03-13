#ifndef RAYCAST_H_
#define RAYCAST_H_

#include <stdlib.h>
#include <math.h>
#include <stdint.h>

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
vec vsub(vec a, vec b);
vec vadd(vec a, vec b);
double vdot(vec a, vec b);
double vcross(vec a, vec b);
int8_t intersect(vec x0, vec x1, vec y0, vec y1, double tol, vec *sect);
double dist(vec x, vec y0, vec y1, double tol);
int8_t inside(vec v, polygon p, double tol);
double vmag(vec a);

#endif // RAYCAST_H_