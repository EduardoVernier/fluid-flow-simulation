#include "Color.cpp"
#include "ColorMap.h"
#include "Glyphs.h"

// different types of color mapping: black-and-white, rainbow, fire or custom
#define COLOR_BLACKWHITE 100
#define COLOR_RAINBOW 101
#define COLOR_BANDS 102
#define COLOR_FIRE 103
#define COLOR_CUSTOM 104
// scalar field selected to be displayed as matter
#define SCALAR_RHO 150
#define SCALAR_VELOCITY 151
#define SCALAR_FORCE 152


int main_window;
int winWidth, winHeight;        //size of the graphics window, in pixels
int color_dir = 0;            //use direction color-coding or not
float vec_scale = 1000;			//scaling of hedgehogs
int draw_smoke = 1;           //draw the smoke or not
int draw_glyphs_flag = 0;            //draw the vector field or not
int scalar_col = COLOR_BLACKWHITE;  //method for scalar coloring
int frozen = 0;               //toggles on/off the animation
int quantize_colormap = 0;

int clamp_flag = 0;
float clamp_min = 0, clamp_max = 0.2;

int scaling_flag = 0;
float dataset_min = 0, dataset_max = 10;

double colorbar_min = 0;
double colorbar_max = 1;

int dataset_id = SCALAR_RHO;

ColorMap fire = ColorMap((char*)"Fire");
ColorMap rainbow = ColorMap((char*)"Rainbow");

ColorMap custom = ColorMap((char*)"Custom");
int custom_color_index = 0;
float **custom_color_ranges = (float**) malloc(5*(sizeof(float*))); // up to 5 interpolations on a custom colormap

Glyphs glyphs = Glyphs(); // singleton

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


void draw_colormap()
{
    double n_samples = 256;

    for (int i = 0; i < n_samples; i++)
    {
        if (clamp_flag)
        {
            colorbar_max = clamp_max;
            colorbar_min = clamp_min;
        }
        else
        {
            colorbar_max = 1;
            colorbar_min = 0;
        }
        colorbar_max *=1.1;

        double current_value = colorbar_min + (i/n_samples)*(colorbar_max-colorbar_min);
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
            for (unsigned j = 0; j < 4; ++j ) //only first 4 characters
            {
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, str[j]);
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
    fftw_real wn = (fftw_real)(winWidth*0.9) / (fftw_real)(DIM + 1);   // Grid cell width
    fftw_real hn = (fftw_real)(winHeight) / (fftw_real)(DIM + 1);  // Grid cell heigh

    fftw_real *dataset;
    if (dataset_id == SCALAR_RHO)
        dataset = rho;
    else if (dataset_id == SCALAR_VELOCITY)
        dataset = v_mag;
    else if (dataset_id == SCALAR_FORCE)
        dataset = f_mag;

    if (draw_smoke)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        for (int j = 0; j < DIM - 1; j++)
        {
            double px,py;
            glBegin(GL_TRIANGLE_STRIP);

            int i = 0;
            px = wn + (fftw_real)i * wn;
            py = hn + (fftw_real)j * hn;
            int idx = (j * DIM) + i;
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

    if (draw_glyphs_flag)
        glyphs.draw_glyphs();

    // draw colormap
    draw_colormap();

}
