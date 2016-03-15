#ifndef GLYPHS_H
#define GLYPHS_H

#include <GL/glut.h>
#include <math.h>

#include "Globals.cpp"
#include "Color.cpp"
#include "ColorMap.h"

extern int winWidth, winHeight, DIM;
extern double *vx, *vy, *v_mag, *fx, *fy, *f_mag, *rho;

// All global variables that are required for glyph coloring (terrible OOP)
extern int clamp_flag, scaling_flag, quantize_colormap, scalar_colormap;
extern float clamp_min, clamp_max, dataset_min, dataset_max;
extern ColorMap rainbow, fire, custom;

class Glyphs
{
public:
    Glyphs();
    void draw_glyphs();
    void color_glyph(int i, int j);
    void direction_to_color(float x, float y);
    void bilinear_interpolation(double *vec, double *vf_x, double *vf_y, double i, double j);

    double *vf_x, *vf_y;
    double vec_scale;
    int glyph_type;
    int scalar_field;
    int vector_field;
    int x_axis_samples, y_axis_samples;
};

#endif // GLYPHS_H
