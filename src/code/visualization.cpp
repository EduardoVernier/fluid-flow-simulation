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
#define SCALAR_VELOC_MAG 151
#define SCALAR_FORCE_MAG 152
#define SCALAR_VELOC_DIV 153
#define SCALAR_FORCE_DIV 154


int main_window;
int winWidth, winHeight;        //size of the graphics window, in pixels
int color_dir = 0;            //use direction color-coding or not
float vec_scale = 1000;			//scaling of hedgehogs
int draw_smoke = 1;           //draw the smoke or not
int draw_glyphs_flag = 0;            //draw the vector field or not
int scalar_col = COLOR_RAINBOW;  //method for scalar coloring
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

// all is global for now - refactor later!
unsigned int textureID[1];


void createTextures()					//Create one 1D texture for each of the available colormaps.
{														//We will next use these textures to color map scalar data.

	glGenTextures(1,textureID);							//Generate 3 texture names, for the textures we will create
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);				//Make sure that OpenGL will understand our CPU-side texture storage format

	for(int i=0;i<1;++i)
	{													//Generate all three textures:
		glBindTexture(GL_TEXTURE_1D,textureID[i]);		//Make i-th texture active (for setting it)
		const int size = 256;							//Allocate a texture-buffer large enough to store our colormaps with high resolution
		float textureImage[3*size];

		for(int j=0;j<size;++j)							//Generate all 'size' RGB texels for the current texture:
		{
			float v = float(j)/(size-1);				//Compute a scalar value in [0,1]
			//float R,G,B;
			Color c = rainbow.get_color(v);						//Map this scalar value to a color, using the current colormap

			textureImage[3*j]   = c.r;					//Store the color for this scalar value in the texture
			textureImage[3*j+1] = c.g;
			textureImage[3*j+2] = c.b;
		}
		glTexImage1D(GL_TEXTURE_1D,0,GL_RGB,size,0,GL_RGB,GL_FLOAT,textureImage);
														//The texture is ready - pass it to OpenGL
	}

	//colormap_type = (COLORMAP_TYPE)0;					//Reset the currently-active colormap to the default (first one)
}


void init_colormaps()
{
    // Example of how to "build" a colormap
    fire.add_color_range(Color(0,0,0), Color(1,0,0), 0, 0.5);
    fire.add_color_range(Color(1,0,0), Color(1,1,0), 0.5, 1);
    fire.add_color_range(Color(1,1,0), Color(1,1,1), 1, 10);

    createTextures();
    glEnable(GL_TEXTURE_1D);
    glBindTexture(GL_TEXTURE_1D,textureID[0]);
	
	glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
	
}


//set_colormap: Sets three different types of colormaps
void set_colormap(double vy)
{
    Color c;
    double out_min = 0, out_max = 1; // considering that values on the simulation and visualization range 0-1 (which they don't!)

    // test test
    if (vy > 0.99) vy = 0.99;
    if (vy < 0) vy = 0.01;


    if (clamp_flag)
    {
        if (vy > clamp_max) vy = clamp_max; if (vy < clamp_min) vy = clamp_min;
        // map interval clamp_min - clamp_max -> out_min - out_max
        vy = (vy - clamp_min) * (out_max - out_min) / (clamp_max - clamp_min) + out_min;
    }

    if (scaling_flag)
        vy = (vy - dataset_min) * (out_max - out_min) / (dataset_max - dataset_min) + out_min;


    //glShadeModel(GL_SMOOTH);
/*
    if(quantize_colormap != 0)
    {
        glShadeModel(GL_FLAT);
        vy *= quantize_colormap;
        vy = (int)(vy);
        vy/= quantize_colormap;
    }
*/
    switch(scalar_col)
    {
    case COLOR_BLACKWHITE:
        c = Color(vy,vy,vy);
        break;
    case COLOR_RAINBOW:
        glEnable(GL_TEXTURE_1D);
        glTexCoord1f(vy);
        //c = rainbow.get_color(vy);
        break;
    case COLOR_FIRE:
        c = fire.get_color(vy);
        break;
    case COLOR_CUSTOM:
        c = custom.get_color(vy);
        break;
    }
    //glColor3f(c.r,c.g,c.b);
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

    if (draw_smoke)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glShadeModel(GL_SMOOTH);
        for (int j = 0; j < DIM - 1; j++)
        {
            double px,py;
            glBegin(GL_TRIANGLE_STRIP);

            int i = 0;
            px = wn + (fftw_real)i * wn;
            py = hn + (fftw_real)j * hn;
            int idx = (j * DIM) + i;
            glTexCoord1f(dataset[idx]);
            glVertex2f(px,py);

            for (i = 0; i < DIM - 1; i++)
            {
                px = wn + (fftw_real)i * wn;
                py = hn + (fftw_real)(j + 1) * hn;
                idx = ((j + 1) * DIM) + i;
                glTexCoord1f(dataset[idx]);
                glVertex2f(px, py);
                px = wn + (fftw_real)(i + 1) * wn;
                py = hn + (fftw_real)j * hn;
                idx = (j * DIM) + (i + 1);
                glTexCoord1f(dataset[idx]);
                glVertex2f(px, py);
            }

            px = wn + (fftw_real)(DIM - 1) * wn;
            py = hn + (fftw_real)(j + 1) * hn;
            idx = ((j + 1) * DIM) + (DIM - 1);
            glTexCoord1f(dataset[idx]);
            glVertex2f(px, py);

            glEnd();
        }
    }

    if (draw_glyphs_flag)
        glyphs.draw_glyphs();

    // draw colormap
    draw_colormap();

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
