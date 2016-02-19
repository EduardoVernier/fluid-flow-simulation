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
#define GLYPH_ARROW 171
// colormap defines
#define COLOR_BLACKWHITE 100
#define COLOR_RAINBOW 101
#define COLOR_BANDS 102
#define COLOR_FIRE 103
#define COLOR_CUSTOM 104

#define PI 3.14159265


Glyphs::Glyphs()
{
    glyph_type = GLYPH_ARROW;
    scalar_field = SF_RHO_ID;
    vector_field = VF_VELOC_ID;
    vec_scale = 1000;
    x_axis_samples = 50;
    y_axis_samples = 50;
}

void Glyphs::draw_glyphs()
{
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

    if (glyph_type == GLYPH_LINE)
    {
        glBegin(GL_LINES);
        for (double i = 0; i < DIM+0.01; i += (DIM/(double)x_axis_samples))
        {
            for (double j = 0; j < DIM+0.01; j += (DIM/(double)y_axis_samples))
            {
                double vec [2] = {0,0};
                color_glyph(round(i), round(j)); // nearest neighbour polocy for coloring
                bilinear_interpolation(vec, vf_x, vf_y, i, j); // interpolation for vetor values
                glVertex2f(wn + i * wn, hn + j * hn);
                glVertex2f((wn + i * wn) + vec_scale * vec[0],
                (hn + j * hn) + vec_scale * vec[1]);
            }
        }
        glEnd();
    }
    else if (glyph_type == GLYPH_ARROW)
    {
        glBegin(GL_TRIANGLES);

        for (double i = 0; i < DIM+0.01; i += (DIM/(double)x_axis_samples))
            for (double j = 0; j < DIM+0.01; j += (DIM/(double)y_axis_samples))
            {
                double vec [2] = {0,0};
                color_glyph(round(i), round(j)); // nearest neighbour polocy for coloring
                bilinear_interpolation(vec, vf_x, vf_y, i, j); // interpolation for vetor values

                glVertex2f(wn + i*wn, hn + j*hn);
                glVertex2f((wn + i*wn) + vec_scale * vec[0],
                            (hn + j*hn) + vec_scale * vec[1]);
                double angle_rad = 150*PI/180;
                glVertex2f((wn + i*wn) + 0.2*vec_scale*(cos(angle_rad)*vec[0] -sin(angle_rad)*vec[1]),
                            (hn + j*hn) + 0.2*vec_scale*(sin(angle_rad)*vec[0] -cos(angle_rad)*vec[1]));


                glVertex2f(wn + i*wn, hn + j*hn);
                glVertex2f((wn + i*wn) + vec_scale * vec[0],
                            (hn + j*hn) + vec_scale * vec[1]);
                angle_rad = (360-150)*PI/180;
                glVertex2f((wn + i*wn) + 0.2*vec_scale*(cos(angle_rad)*vec[0] -sin(angle_rad)*vec[1]),
                            (hn + j*hn) + 0.2*vec_scale*(sin(angle_rad)*vec[0] -cos(angle_rad)*vec[1]));
            }
        glEnd();

    }

}

void Glyphs::bilinear_interpolation(double *vec, double *vf_x, double *vf_y, double i, double j)
{
    double x_fxy1 = vf_x[int(floor(j)*DIM + floor(i))]*(1 - (i-floor(i)))
                + vf_x[int(floor(j)*DIM + ceil(i))]*(i-floor(i));

    double x_fxy2 = vf_x[int(ceil(j)*DIM + floor(i))]*(1 - (i-floor(i)))
                + vf_x[int(ceil(j)*DIM + ceil(i))]*(i-floor(i));

    double x_fxy = x_fxy1*(1-(j-floor(j))) + x_fxy2*(j-floor(j));


    double y_fxy1 = vf_y[int(floor(j)*DIM + floor(i))]*(1 - (i-floor(i)))
                + vf_y[int(floor(j)*DIM + ceil(i))]*(i-floor(i));

    double y_fxy2 = vf_y[int(ceil(j)*DIM + floor(i))]*(1 - (i-floor(i)))
                + vf_y[int(ceil(j)*DIM + ceil(i))]*(i-floor(i));

    double y_fxy = y_fxy1*(1-(j-floor(j))) + y_fxy2*(j-floor(j));

    vec[0] = x_fxy;
    vec[1] = y_fxy;

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
