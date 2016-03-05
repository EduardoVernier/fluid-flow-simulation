#include "Color.cpp"
#include "ColorMap.h"
#include "Glyphs.h"
#include "IsolineManager.h"

// Viewing, camera and ilumination parameters
int main_window;
int winWidth, winHeight; // size of the graphics window, in pixels
// Perspective projection parameters
float fov = 80;
float aspect = 1;
float z_near = 0.1;
float z_far = 1000;
// Modelview (camera extrinsic) parameters
float eye_x = 400, eye_y = -150, eye_z = 200;
float c_x   = 400, c_y   = 300,  c_z   = 0;
float up_x  = 0,  up_y  = 0,  up_z  = 1;

// Simulation control variables
int frozen = 0; //toggles on/off the animation

// Colormapping variables
int clamp_flag = 0;
float clamp_min = 0, clamp_max = 0.2;
int scaling_flag = 0;
float dataset_min = 0, dataset_max = 10;
int quantize_colormap = 0;
double colorbar_min = 0;
double colorbar_max = 1;
ColorMap fire = ColorMap((char*)"Fire");
ColorMap rainbow = ColorMap((char*)"Rainbow");
ColorMap custom = ColorMap((char*)"Custom");
int custom_color_index = 0;
float **custom_color_ranges = (float**) malloc(5*(sizeof(float*))); // up to 5 interpolations on a custom colormap

// Matter visualization variables
int draw_matter = 1; // draw the smoke (matter) or not
int scalar_colormap = COLOR_BLACKWHITE; // method for scalar coloring
int dataset_id = SCALAR_RHO;

// Glyphs visualization variables
int draw_glyphs_flag = 0; //draw the vector field or not
int color_dir = 0; //use direction color-coding or not
float vec_scale = 1000;	//scaling of hedgehogs
Glyphs glyphs = Glyphs();

// Isolines variables
int draw_isolines_flag = 0;
int isoline_colormap = COLOR_RAINBOW;
IsolineManager isoline_manager = IsolineManager();

// Height Plot variables
int draw_height_flag = 0;
float dataset_scale = 1;


void init_colormaps()
{
    // Example of how to "build" a colormap
    fire.add_color_range(Color(0,0,0), Color(1,0,0), 0, 0.5);
    fire.add_color_range(Color(1,0,0), Color(1,1,0), 0.5, 1);
    fire.add_color_range(Color(1,1,0), Color(1,1,1), 1, 10);
}

//set_color: Sets three different types of colormaps
void set_color(double vy, int colormap)
{
    Color c;
    double out_min = 0, out_max = 1; // considering that values on the simulation and visualization range 0-1 (which they don't!)

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

    switch(colormap)
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


void draw_colorbar()
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
        set_color(current_value, scalar_colormap);
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
            for (unsigned j = 0; j < 5; ++j ) //only first 5 characters
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


void draw_isolines(double *dataset)
{
    double wn = (double)(winWidth*0.9) / (double)(DIM + 1);   // Grid cell width
    double hn = (double)(winHeight) / (double)(DIM + 1);  // Grid cell heigh

    for(vector<Isoline>::iterator iso_it = isoline_manager.isoline_vector.begin(); iso_it != isoline_manager.isoline_vector.end(); ++iso_it)
    {
        for(vector<pair<float, float> >::iterator points_it = iso_it->points.begin(); points_it != iso_it->points.end(); points_it+=2)
        {
            float p1x = wn + (float)points_it->second * wn;
            float p1y = hn + (float)points_it->first * hn;

            float p2x = wn + (float)(points_it+1)->second * wn;
            float p2y = hn + (float)(points_it+1)->first * hn;

            glBegin(GL_LINES);
            set_color(iso_it->v, isoline_colormap);
            glVertex2f(p1x,p1y);
            glVertex2f(p2x,p2y);
            glEnd();
       }
    }
}

// testetete
float f(float x, float y)								//A simple two-variable function to plot
{														//The function is samples
	float ret = 8*expf(-(x*x+y*y)/5);
	return ret;
}

//visualize: This is the main visualization function
void visualize(void)
{
    fftw_real wn = (fftw_real)(winWidth*0.9) / (fftw_real)(DIM + 1);   // Grid cell width
    fftw_real hn = (fftw_real)(winHeight) / (fftw_real)(DIM + 1);  // Grid cell heigh

    fftw_real *dataset;
    if (dataset_id == SCALAR_RHO)
        dataset = rho;
    else if (dataset_id == SCALAR_VELOC_MAG)
        dataset = v_mag;
    else if (dataset_id == SCALAR_FORCE_MAG)
        dataset = f_mag;
    else if (dataset_id == SCALAR_FORCE_DIV || dataset_id == SCALAR_VELOC_DIV)
        dataset = div_vf;

    if (draw_matter)
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
            set_color(dataset[idx], scalar_colormap);
            glVertex2f(px,py);

            for (i = 0; i < DIM - 1; i++)
            {
                px = wn + (fftw_real)i * wn;
                py = hn + (fftw_real)(j + 1) * hn;
                idx = ((j + 1) * DIM) + i;
                set_color(dataset[idx], scalar_colormap);
                glVertex2f(px, py);
                px = wn + (fftw_real)(i + 1) * wn;
                py = hn + (fftw_real)j * hn;
                idx = (j * DIM) + (i + 1);
                set_color(dataset[idx], scalar_colormap);
                glVertex2f(px, py);
            }
            px = wn + (fftw_real)(DIM - 1) * wn;
            py = hn + (fftw_real)(j + 1) * hn;
            idx = ((j + 1) * DIM) + (DIM - 1);
            set_color(dataset[idx], scalar_colormap);
            glVertex2f(px, py);
            glEnd();
        }
    }

    if (draw_height_flag)
    {

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity ();
        gluLookAt(eye_x,eye_y,eye_z,c_x,c_y,c_z,up_x,up_y,up_z);
        glMatrixMode (GL_PROJECTION);
        glLoadIdentity ();
        gluPerspective(fov,float(winWidth)/winHeight,z_near,z_far);

        glClearColor(1,1,1,1);								//1. Clear the frame and depth buffers
    	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        for (int j = 0; j < DIM - 1; j++)
        {
            double px,py;
            glBegin(GL_TRIANGLE_STRIP);

            int i = 0;
            px = wn + (fftw_real)i * wn;
            py = hn + (fftw_real)j * hn;
            int idx = (j * DIM) + i;
            set_color(dataset[idx], scalar_colormap);
            glVertex3f(px, py, dataset[idx]*dataset_scale);

            for (i = 0; i < DIM - 1; i++)
            {
                px = wn + (fftw_real)i * wn;
                py = hn + (fftw_real)(j + 1) * hn;
                idx = ((j + 1) * DIM) + i;
                set_color(dataset[idx], scalar_colormap);
                glVertex3f(px, py, dataset[idx]*dataset_scale);
                px = wn + (fftw_real)(i + 1) * wn;
                py = hn + (fftw_real)j * hn;
                idx = (j * DIM) + (i + 1);
                set_color(dataset[idx], scalar_colormap);
                glVertex3f(px, py, dataset[idx]*dataset_scale);
            }
            px = wn + (fftw_real)(DIM - 1) * wn;
            py = hn + (fftw_real)(j + 1) * hn;
            idx = ((j + 1) * DIM) + (DIM - 1);
            set_color(dataset[idx], scalar_colormap);
            glVertex3f(px, py, dataset[idx]*dataset_scale);
            glEnd();
        }
         glutSwapBuffers();
    }

    if (draw_glyphs_flag)
        glyphs.draw_glyphs();

    if (draw_isolines_flag)
    {
        if(isoline_manager.isoline_vector.empty())
            isoline_manager.create_isoline();
        isoline_manager.compute_isolines();
        draw_isolines(dataset);
    }

    draw_colorbar();
}


//display: Handle window redrawing events. Simply delegates to visualize().
void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    visualize();
    glFlush();
    glutSwapBuffers();
}

//reshape: Handle window resizing (reshaping) events
void reshape(int w, int h)
{

    glViewport(0.0f, 0.0f, (GLfloat)w, (GLfloat)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, (GLdouble)w, 0.0, (GLdouble)h);
    winWidth = w; winHeight = h;

}
