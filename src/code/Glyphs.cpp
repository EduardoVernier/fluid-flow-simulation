#include "Glyphs.h"

// scalar field selector id on listbox
#define SF_RHO_ID 150
#define SF_VELOC_ID 151
#define SF_FORCE_ID 152
#define SF_DIR_ID 162
#define SF_WHITE_ID 163
// vector fields ids
#define VF_VELOC_ID 160
#define VF_FORCE_ID 161
// glyph type ids
#define GLYPH_LINE 170
#define GLYPH_TRI 171
// colormap defines
#define COLOR_BLACKWHITE 100
#define COLOR_RAINBOW 101
#define COLOR_BANDS 102
#define COLOR_FIRE 103
#define COLOR_CUSTOM 104

Glyphs::Glyphs()
{
    glyph_type = GLYPH_LINE;
    scalar_field = SF_RHO_ID;
    vector_field = VF_VELOC_ID;
    vec_scale = 1000;
}

void Glyphs::draw_glyphs()
{
    int i, j, idx;
    double wn = (double)(winWidth*0.9) / (double)(DIM + 1);   // Grid cell width
    double hn = (double)(winHeight) / (double)(DIM + 1);  // Grid cell heigh

    if (vector_field == VF_VELOC_ID)
    {
        vf_x = vx;
        vf_y = vy;
    }
    else if (VF_FORCE_ID)
    {
        vf_x = fx;
        vf_y = fy;
    }

    glBegin(GL_LINES);
    for (i = 0; i < DIM; i++)
        for (j = 0; j < DIM; j++)
        {
            idx = (j * DIM) + i;
            //direction_to_color(vf_x[idx],vf_y[idx]);
            color_glyph(i, j);
            glVertex2f(wn + i * wn, hn + j * hn);
            glVertex2f((wn + i * wn) + vec_scale * vf_x[idx],
                        (hn + j * hn) + vec_scale * vf_y[idx]);
        }
    glEnd();
}

void Glyphs::color_glyph(int i, int j)
{
    int idx = (j * DIM) + i;
    switch (scalar_field) {
    case SF_WHITE_ID:
        glColor3f(1,1,1);
        break;
    case SF_DIR_ID:
        direction_to_color(vf_x[idx],vf_y[idx]);
        break;
    case SF_RHO_ID:
        set_colormap(rho[idx]);
        break;
    case SF_VELOC_ID:
        set_colormap(v_mag[idx]);
        break;
    case SF_FORCE_ID:
        set_colormap(f_mag[idx]);
        break;
    }
}

// Duplicate from visualization.cpp << careful!
//set_colormap: Sets three different types of colormaps
void Glyphs::set_colormap(float vy)
{
    Color c;
    float out_min = 0, out_max = 1; // considering that values on the simulation and visualization range 0-1 (which they don't!)

    if (clamp_flag)
    {
        if (vy > clamp_max) vy = clamp_max; if (vy < clamp_min) vy = clamp_min;
        // map interval clamp_min - clamp_max -> out_min - out_max
        vy = (vy - clamp_min) * (out_max - out_min) / (clamp_max - clamp_min) + out_min;
    }

    if (scaling_flag)
        vy = (vy - dataset_min) * (out_max - out_min) / (dataset_max - dataset_min) + out_min;


    glShadeModel(GL_SMOOTH);
    if(quantize_colormap != 0)
    {
        glShadeModel(GL_FLAT);
        vy *= quantize_colormap;
        vy = (int)(vy);
        vy/= quantize_colormap;
    }

    switch(scalar_col)
    {
    case COLOR_BLACKWHITE:
        c = Color(vy,vy,vy);
        break;
    case COLOR_RAINBOW:
        c = rainbow.get_color(vy);
        break;
    case COLOR_FIRE:
        c = fire.get_color(vy);
        break;
    case COLOR_CUSTOM:
        c = custom.get_color(vy);
        break;
    }
    glColor3f(c.r,c.g,c.b);
}


void Glyphs::direction_to_color(float x, float y)
{
    float r,g,b,f;

    f = atan2(y,x) / 3.1415927 + 1;
    r = f;
    if(r > 1) r = 2 - r;
    g = f + .66667;
    if(g > 2) g -= 2;
    if(g > 1) g = 2 - g;
    b = f + 2 * .66667;
    if(b > 2) b -= 2;
    if(b > 1) b = 2 - b;

    glColor3f(r,g,b);
}
