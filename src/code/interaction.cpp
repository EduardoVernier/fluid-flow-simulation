#define CM_BW_ID 100
#define CM_RB_ID 101
#define CM_BD_ID 102
#define DT_INCREASE_ID 200
#define DT_DECREASE_ID 201
#define HH_INCREASE_ID 202
#define HH_DECREASE_ID 203
#define FV_INCREASE_ID 204
#define FV_DECREASE_ID 205
#define PP_ID 206
#define QT_ID 207


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

//keyboard: Handle key presses
void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	  case 't': dt -= 0.001; break;
	  case 'T': dt += 0.001; break;
	  case 'c': color_dir = 1 - color_dir; break;
	  case 'S': vec_scale *= 1.2; break;
	  case 's': vec_scale *= 0.8; break;
	  case 'V': visc *= 5; break;
	  case 'v': visc *= 0.2; break;
	  case 'x': draw_smoke = 1 - draw_smoke;
		    if (draw_smoke==0) draw_vecs = 1; break;
	  case 'y': draw_vecs = 1 - draw_vecs;
		    if (draw_vecs==0) draw_smoke = 1; break;
	  case 'm': scalar_col++; if (scalar_col>COLOR_BANDS) scalar_col=COLOR_BLACKWHITE; break;
	  case 'a': frozen = 1-frozen; break;
	  case 'q': exit(0);
	}
}


// drag: When the user drags with the mouse, add a force that corresponds to the direction of the mouse
//       cursor movement. Also inject some new matter into the field at the mouse location.
void drag(int mx, int my)
{
	int xi,yi,X,Y;
	double  dx, dy, len;
	static int lmx=0,lmy=0;				//remembers last mouse location

	// Compute the array index that corresponds to the cursor location
	xi = (int)clamp((double)(DIM + 1) * ((double)mx / (double)winWidth));
	yi = (int)clamp((double)(DIM + 1) * ((double)(winHeight - my) / (double)winHeight));

	X = xi; Y = yi;

	if (X > (DIM - 1))  X = DIM - 1; if (Y > (DIM - 1))  Y = DIM - 1;
	if (X < 0) X = 0; if (Y < 0) Y = 0;

	// Add force at the cursor location
	my = winHeight - my;
	dx = mx - lmx; dy = my - lmy;
	len = sqrt(dx * dx + dy * dy);
	if (len != 0.0) {  dx *= 0.1 / len; dy *= 0.1 / len; }
	fx[Y * DIM + X] += dx;
	fy[Y * DIM + X] += dy;
	rho[Y * DIM + X] = 10.0f;
	lmx = mx; lmy = my;
}


void control_cb(int control)
{
	switch (control)
	{
	case DT_INCREASE_ID:
		dt += 0.01;
		break;
	case DT_DECREASE_ID:
		dt -= 0.01;
		break;
	case HH_INCREASE_ID:
		vec_scale *= 1.2;
		break;
	case HH_DECREASE_ID:
		vec_scale *= 0.8;
		break;
	case FV_INCREASE_ID:
		visc *= 1.2;
		break;
	case FV_DECREASE_ID:
		visc *= 0.8;
		break;
	case PP_ID:
		(frozen == 0)? frozen = 1: frozen = 0;
		break;
	case QT_ID:
		exit(0);
		break;
	case CM_BD_ID:
	case CM_RB_ID:
	case CM_BW_ID:
		scalar_col = control;
		break;
	}

	glutPostRedisplay();


	printf("dt: %.2f   Hedgehog Scale: %0.2f   Fluid Viscosity:%.5f \n", dt, vec_scale, visc);
}



void init_control_window()
{
    GLUI *glui = GLUI_Master.create_glui( "GLUI" );

	glui->add_checkbox( "Matter", &draw_smoke);
	glui->add_checkbox( "Vector Field", &draw_vecs);
	glui->add_checkbox( "Direction Coloring", &color_dir);
	glui->add_checkbox( "Thru Scalar Coloring", &color_dir);

	GLUI_Panel *dt_panel = new GLUI_Panel (glui, "Time Step");
  	new GLUI_Button(dt_panel, "Increase", DT_INCREASE_ID, control_cb );
	new GLUI_Button(dt_panel, "Decrease", DT_DECREASE_ID, control_cb );

	GLUI_Panel *hedgehog_panel = new GLUI_Panel (glui, "Hedgehog Scaling");
  	new GLUI_Button(hedgehog_panel, "Increase", HH_INCREASE_ID, control_cb );
	new GLUI_Button(hedgehog_panel, "Decrease", HH_DECREASE_ID, control_cb );

	GLUI_Panel *visc_panel = new GLUI_Panel (glui, "Fluid Viscosity");
	new GLUI_Button(visc_panel, "Increase", FV_INCREASE_ID, control_cb );
	new GLUI_Button(visc_panel, "Decrease", FV_DECREASE_ID, control_cb );

	GLUI_Panel *color_panel = new GLUI_Panel (glui, "Color Mapping");
	new GLUI_Button(color_panel, "Black and White", CM_BW_ID, control_cb );
	new GLUI_Button(color_panel, "Rainbow", CM_RB_ID, control_cb );
	new GLUI_Button(color_panel, "Banded", CM_BD_ID, control_cb );


	new GLUI_Button(glui, "Pause/Play", PP_ID, control_cb );
	new GLUI_Button(glui, "Quit", QT_ID, control_cb );
	GLUI_Master.auto_set_viewport();

	glui->set_main_gfx_window(main_window);
}
