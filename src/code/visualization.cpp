#include "Color.cpp"
#include "ColorMap.h"

#define COLOR_BLACKWHITE 100   //different types of color mapping: black-and-white, rainbow, banded
#define COLOR_RAINBOW 101
#define COLOR_BANDS 102
#define COLOR_FIRE 103
#define COLOR_CUSTOM 104
#define DATASET_RHO 150
#define DATASET_VELOCITY 151
#define DATASET_FORCE 152

int main_window;				//set
int winWidth, winHeight;      //size of the graphics window, in pixels
int color_dir = 0;            //use direction color-coding or not
float vec_scale = 1000;			//scaling of hedgehogs
int draw_smoke = 1;           //draw the smoke or not
int draw_vecs = 0;            //draw the vector field or not
int scalar_col = COLOR_BLACKWHITE;  //method for scalar coloring
int frozen = 0;               //toggles on/off the animation

int clamp_flag = 0;
float clamp_min = 0, clamp_max = 0.2;

int scaling_flag = 0;
float dataset_min = 0, dataset_max = 10;

int dataset_id = DATASET_RHO;

ColorMap fire = ColorMap((char*)"Fire");
ColorMap rainbow = ColorMap((char*)"Rainbow");

ColorMap custom = ColorMap((char*)"Custom");
int custom_color_index = 0;
float **custom_color_ranges = (float**) malloc(5*(sizeof(float*))); // up to 5 interpolations on a custom colormap

void init_colormaps()
{
    fire.add_color_range(Color(0,0,0), Color(1,0,0), 0, 0.5);
    fire.add_color_range(Color(1,0,0), Color(1,1,0), 0.5, 1);
    fire.add_color_range(Color(1,1,0), Color(1,1,1), 1, 10);
}


//set_colormap: Sets three different types of colormaps
void set_colormap(float vy)
{
    Color c;
    float out_min = 0, out_max = 10.0; // considering that values on the simulation and visualization range 0-10

    if (clamp_flag)
    {
        if (vy > clamp_max) vy = clamp_max;
        if (vy < clamp_min) vy = clamp_min;
        vy = (vy - clamp_min) * (out_max - out_min) / (clamp_max - clamp_min) + out_min;
    }
    if (scaling_flag)
    {
        vy = (vy - dataset_min) * (out_max - out_min) / (dataset_max - dataset_min) + out_min;
    }

    int NLEVELS = 7;
    switch(scalar_col)
    {
    case COLOR_BLACKWHITE:
        c = Color(vy,vy,vy);
        break;
    case COLOR_RAINBOW:
        c = rainbow.get_color(vy);
        break;
    case COLOR_BANDS:
        vy *= NLEVELS; vy = (int)(vy); vy/= NLEVELS;
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


//direction_to_color: Set the current color by mapping a direction vector (x,y), using
//                    the color mapping method 'method'. If method==1, map the vector direction
//                    using a rainbow colormap. If method==0, simply use the white color
void direction_to_color(float x, float y, int method)
{
    float r,g,b,f;
    if (method)
    {
        f = atan2(y,x) / 3.1415927 + 1;
        r = f;
        if(r > 1) r = 2 - r;
        g = f + .66667;
        if(g > 2) g -= 2;
        if(g > 1) g = 2 - g;
        b = f + 2 * .66667;
        if(b > 2) b -= 2;
        if(b > 1) b = 2 - b;
    }
    else
    { r = g = b = 1; }
    glColor3f(r,g,b);
}


void draw_colormap()
{
    double n_samples = 256;

    for (int i = 0; i < n_samples; i++)
    {
        double c_min = 0;
        double c_max = 10;
        if (clamp_flag)
        {
            c_max = clamp_max;
            c_min = clamp_min;
        }

        double current_value = c_min + (i/n_samples)*(c_max-c_min);
        set_colormap(current_value);
        glRectd(0.9*winWidth, i*((winHeight-80)/n_samples)+40,
                0.95*winWidth, (i+1)*((winHeight-80)/n_samples)+40);

        if(i % ((int)n_samples/10) == 0)
        {
            glColor3f(1,1,1);
            std::string str = std::to_string(current_value);
            glMatrixMode( GL_MODELVIEW );
            glPushMatrix();
            glLoadIdentity();
            glRasterPos2i( 0.96*winWidth, i*((winHeight-80)/n_samples)+40);  // move in 10 pixels from the left and bottom edges
            for (unsigned j = 0; j < str.length(); ++j )
            {
                glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_10, str[j]);
            }
            glPopMatrix();

            glMatrixMode( GL_PROJECTION );
            glPopMatrix();
            glMatrixMode( GL_MODELVIEW );

        }
    }


}

//visualize: This is the main visualization function
void visualize(void)
{
    int i, j, idx;
    double px,py;
    fftw_real wn = (fftw_real)(winWidth*0.9) / (fftw_real)(DIM + 1);   // Grid cell width
    fftw_real hn = (fftw_real)(winHeight) / (fftw_real)(DIM + 1);  // Grid cell heigh

    fftw_real *dataset;
    if (dataset_id == DATASET_RHO)
        dataset = rho;
    else if (dataset_id == DATASET_VELOCITY)
        dataset = v_mag;
    else if (dataset_id == DATASET_FORCE)
        dataset = f_mag;

    if (draw_smoke)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        for (j = 0; j < DIM - 1; j++)
        {
            glBegin(GL_TRIANGLE_STRIP);

            i = 0;
            px = wn + (fftw_real)i * wn;
            py = hn + (fftw_real)j * hn;
            idx = (j * DIM) + i;
            set_colormap(dataset[idx]);
            glVertex2f(px,py);

            for (i = 0; i < DIM - 1; i++)
            {
                px = wn + (fftw_real)i * wn;
                py = hn + (fftw_real)(j + 1) * hn;
                idx = ((j + 1) * DIM) + i;
                set_colormap(dataset[idx]);
                glVertex2f(px, py);
                px = wn + (fftw_real)(i + 1) * wn;
                py = hn + (fftw_real)j * hn;
                idx = (j * DIM) + (i + 1);
                set_colormap(dataset[idx]);
                glVertex2f(px, py);
            }

            px = wn + (fftw_real)(DIM - 1) * wn;
            py = hn + (fftw_real)(j + 1) * hn;
            idx = ((j + 1) * DIM) + (DIM - 1);
            set_colormap(dataset[idx]);
            glVertex2f(px, py);

            glEnd();
        }
    }

    if (draw_vecs)
    {
        glBegin(GL_LINES);				//draw velocities
        for (i = 0; i < DIM; i++)
            for (j = 0; j < DIM; j++)
            {
                idx = (j * DIM) + i;
                direction_to_color(vx[idx],vy[idx],color_dir);
                glVertex2f(wn + (fftw_real)i * wn, hn + (fftw_real)j * hn);
                glVertex2f((wn + (fftw_real)i * wn) + vec_scale * vx[idx], (hn + (fftw_real)j * hn) + vec_scale * vy[idx]);
            }
        glEnd();
    }

    // draw colormap
    draw_colormap();

}
