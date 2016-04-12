// button/checkbox/listbox ids
enum UI_IDs
{
DT_INCREASE_ID = 1000,
DT_DECREASE_ID,
HH_INCREASE_ID,
HH_DECREASE_ID,
FV_INCREASE_ID,
FV_DECREASE_ID,
PP_ID,
QT_ID,
CLAMPING_ID,
SCALING_ID,
QUANT_ID,
SX_INCREASE_ID,
SX_DECREASE_ID,
SY_INCREASE_ID,
SY_DECREASE_ID,
ENABLE_GLYPHS,
APPLY_ISOLINES,
ENABLE_HEIGHT,
EYE_X_INCREASE_ID,
EYE_X_DECREASE_ID,
EYE_Y_INCREASE_ID,
EYE_Y_DECREASE_ID,
EYE_Z_INCREASE_ID,
EYE_Z_DECREASE_ID,
C_X_INCREASE_ID,
C_X_DECREASE_ID,
C_Y_INCREASE_ID,
C_Y_DECREASE_ID,
C_Z_INCREASE_ID,
C_Z_DECREASE_ID,
SCALE_INCREASE_ID,
SCALE_DECREASE_ID,
UPDATE_SCALAR_COLORMAP,
ENABLE_ST,
RESET_ST
};

// statictext objects pointers are global because control_cb callback
// function can't handle arguments except int values
GLUI_StaticText *dt_text, *hh_text, *visc_text;
GLUI_StaticText *x_sample_text, *y_sample_text;
GLUI_EditText *scale_edittext;
GLUI_EditText *eye_x_edittext, *eye_y_edittext, *eye_z_edittext;
GLUI_EditText *c_x_edittext, *c_y_edittext, *c_z_edittext;

// parameters window and custom colormap window global pointers
GLUI *glui;
GLUI *cust_window = NULL;
GLUI *st_seeds_window = NULL;

// mouse variables
int mxi = 0, myi = 0;

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
    case 'x': draw_matter = 1 - draw_matter;
        if (draw_matter==0) draw_glyphs_flag = 1; break;
    case 'y': draw_glyphs_flag = 1 - draw_glyphs_flag;
        if (draw_glyphs_flag==0) draw_matter = 1; break;
    //case 'm': scalar_colormap++; if (scalar_colormap>COLOR_BANDS) scalar_colormap=COLOR_BLACKWHITE; break;
    case 'a': frozen = 1-frozen; break;
    case ' ': stream_tube_manager.add_stream_tube(mxi, myi); break;
    case 'q': exit(0);
    }
}

void mouse_hover(int mx, int my)
{
    // Compute the array index that corresponds to the cursor location
    mxi = (int)clamp((double)(DIM + 1) * ((double)mx / (double)winWidth*1.05));
    myi = (int)clamp((double)(DIM + 1) * ((double)(winHeight - my) / (double)winHeight));
    // printf("%d  %d\n", mxi, myi);
}

// drag: When the user drags with the mouse, add a force that corresponds to the direction of the mouse
//       cursor movement. Also inject some new matter into the field at the mouse location.
void drag(int mx, int my)
{
    int xi,yi,X,Y;
    double  dx, dy, len;
    static int lmx=0,lmy=0;				//remembers last mouse location

    // Compute the array index that corresponds to the cursor location
    xi = (int)clamp((double)(DIM + 1) * ((double)mx / (double)winWidth*1.05));
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

// update_variables_config_window: Used to write on the statictext values
void update_variables_config_window()
{
    // Update variable values on the config window
    char buffer[50];
    sprintf(buffer, "= %.2f", dt);
    dt_text->set_text(buffer);
    sprintf(buffer, "= %.2f", glyphs.vec_scale);
    hh_text->set_text(buffer);
    sprintf(buffer, "= %.6f", visc);
    visc_text->set_text(buffer);
    //sprintf(buffer, "X axis = %d", glyphs.x_axis_samples);
    //x_sample_text->set_text(buffer);
    //sprintf(buffer, "Y axis = %d", glyphs.y_axis_samples);
    //y_sample_text->set_text(buffer);

    scale_edittext->set_float_val(dataset_scale);
    eye_x_edittext->set_float_val(eye_x);
    eye_y_edittext->set_float_val(eye_y);
    eye_z_edittext->set_float_val(eye_z);
}

// resume: Unfortunate solution to rendering glui windows
void resume (int t){ frozen = t;}

// add_range_to_custom_cm: Takes an index i that corresponds to the possition on
//                         the custom_color_ranges global variable and sets it's values
//                         to the custom colormap;
void add_range_to_custom_cm (int i)
{
    float *args = custom_color_ranges[i];

    Color a = Color(args[0], args[1], args[2]);
    Color b = Color(args[3], args[4], args[5]);

    custom.add_color_range(a, b, args[6], args[7]);
    update_textures();
    //resume(0);
    cust_window->close();
    cust_window = NULL;
}

// handle_custom_colormap: Deals with the custom colormap window
void handle_custom_colormap()
{
    if (cust_window != NULL)
        return;

    cust_window = GLUI_Master.create_glui("Custom Dataset");

    float *colormap_args = (float *) malloc(8*sizeof(float));
    custom_color_ranges[custom_color_index] = colormap_args; //warning -> aliasing
    for (int i = 0; i < 8; i++)
        colormap_args[i] = 0.0;

    cust_window->add_statictext("Color 1");
    cust_window->add_edittext("R", GLUI_EDITTEXT_FLOAT, &colormap_args[0]);
    cust_window->add_edittext("G", GLUI_EDITTEXT_FLOAT, &colormap_args[1]);
    cust_window->add_edittext("B", GLUI_EDITTEXT_FLOAT, &colormap_args[2]);

    cust_window->add_column(true);

    cust_window->add_statictext("Color 2");
    cust_window->add_edittext("R", GLUI_EDITTEXT_FLOAT, &colormap_args[3]);
    cust_window->add_edittext("G", GLUI_EDITTEXT_FLOAT, &colormap_args[4]);
    cust_window->add_edittext("B", GLUI_EDITTEXT_FLOAT, &colormap_args[5]);

    cust_window->add_column(true);

    cust_window->add_statictext("Interpolation Interval");
    cust_window->add_edittext("Start", GLUI_EDITTEXT_FLOAT, &colormap_args[6]);
    cust_window->add_edittext("End", GLUI_EDITTEXT_FLOAT, &colormap_args[7]);

    new GLUI_Button(cust_window, "Add interpolation to colormap",
                    custom_color_index, add_range_to_custom_cm);
    custom_color_index++;

    GLUI_Master.auto_set_viewport();
    cust_window->set_main_gfx_window(main_window);
}

void handle_st_seeds()
{
    if (cust_window != NULL)
        return;

    st_seeds_window = GLUI_Master.create_glui("Manage ST Seeds");

}

// control_cb: Takes (almost) all callbacks from buttons, checkboxes, etc.
void control_cb(int control)
{
    // In order to glut have enough time to render the config window
    // the program needs to be frozen for 100ms so the config window
    // becomes target of the display function instead of the main
    if (control == PP_ID)
    {
        (frozen == 0)? frozen = 1: frozen = 0;
    }
    else
    {
        //frozen = 1;
        //glutTimerFunc(100, resume, 0);

        switch (control)
        {
        case DT_INCREASE_ID:
            dt += 0.02;
            break;
        case DT_DECREASE_ID:
            dt -= 0.02;
            break;
        case HH_INCREASE_ID:
            glyphs.vec_scale *= 1.1;
            break;
        case HH_DECREASE_ID:
            glyphs.vec_scale *= 0.9;
            break;
        case FV_INCREASE_ID:
            visc *= 1.2;
            break;
        case FV_DECREASE_ID:
            visc *= 0.8;
            break;
        case QT_ID:
            exit(0);
            break;
        case COLOR_CUSTOM:
            //glutTimerFunc(100, resume, 1);
            handle_custom_colormap();
        case COLOR_RAINBOW:
        case COLOR_BLACKWHITE:
        case COLOR_FIRE:
            scalar_colormap = control;
            break;
        case SX_INCREASE_ID:
            glyphs.x_axis_samples++;
            break;
        case SX_DECREASE_ID:
            glyphs.x_axis_samples--;
            break;
        case SY_INCREASE_ID:
            glyphs.y_axis_samples++;
            break;
        case SY_DECREASE_ID:
            glyphs.y_axis_samples--;
            break;
        case APPLY_ISOLINES:
            isoline_manager.reset();
            update_textures();
            break;
        case ENABLE_HEIGHT:
            reshape(winWidth, winHeight);
            break;
        case EYE_X_INCREASE_ID:
            eye_x += 10;
            break;
        case EYE_X_DECREASE_ID:
            eye_x -= 10;
            break;
        case EYE_Y_INCREASE_ID:
            eye_y += 10;
            break;
        case EYE_Y_DECREASE_ID:
            eye_y -= 10;
            break;
        case EYE_Z_INCREASE_ID:
            eye_z += 10;
            break;
        case EYE_Z_DECREASE_ID:
            eye_z -= 10;
            break;
        case C_X_INCREASE_ID:
            c_x += 10;
            break;
        case C_X_DECREASE_ID:
            c_x -= 10;
            break;
        case C_Y_INCREASE_ID:
            c_y += 10;
            break;
        case C_Y_DECREASE_ID:
            c_y -= 10;
            break;
        case C_Z_INCREASE_ID:
            c_z += 10;
            break;
        case C_Z_DECREASE_ID:
            c_z -= 10;
            break;
        case SCALE_INCREASE_ID:
            dataset_scale += 10;
            break;
        case SCALE_DECREASE_ID:
            dataset_scale -= 10;
            break;
        case QUANT_ID:
        case UPDATE_SCALAR_COLORMAP:
            update_textures();
            break;
        case ENABLE_ST:
            update_textures();
            break;
        case RESET_ST:
            stream_tube_manager = StreamTubeManager();
            break;
        }
        update_textures();

        update_variables_config_window();
        glutPostRedisplay();
        //printf("dt: %.2f   Hedgehog Scale: %0.2f   Fluid Viscosity:%.5f \n", dt, vec_scale, visc);
    }
}

// init_control_window: Creates and configures UI elements
void init_control_window()
{

    glui = GLUI_Master.create_glui( "GLUI" );

    // Simulation Parameters
    GLUI_Rollout *simu_rollout = glui->add_rollout ("Simulation", true);
    GLUI_StaticText *spacer_simu = glui->add_statictext_to_panel(simu_rollout, "");
    spacer_simu->set_w(260); // Ugly trick to make panels have the width I desire
    spacer_simu->set_h(0);
    GLUI_Panel *fix = new GLUI_Panel(simu_rollout,"");
    GLUI_Panel *dt_panel = glui->add_panel_to_panel(fix, "Time Step");
    dt_text = glui->add_statictext_to_panel(dt_panel, "");
    new GLUI_Button(dt_panel, "Increase", DT_INCREASE_ID, control_cb);
    new GLUI_Button(dt_panel, "Decrease", DT_DECREASE_ID, control_cb);

    glui->add_column_to_panel(fix, false);

    GLUI_Panel *visc_panel = glui->add_panel_to_panel(fix, "Fluid Viscosity");
    visc_text = glui->add_statictext_to_panel(visc_panel, "");
    new GLUI_Button(visc_panel, "Increase", FV_INCREASE_ID, control_cb);
    new GLUI_Button(visc_panel, "Decrease", FV_DECREASE_ID, control_cb);

    simu_rollout->close();

    // Matter Parameters
    GLUI_Rollout *matter_rollout = glui->add_rollout("Matter", true);
    GLUI_StaticText *spacer_matter = glui->add_statictext_to_panel(matter_rollout, "");
    spacer_matter->set_w(260);
    spacer_matter->set_h(0);
    glui->add_checkbox_to_panel(matter_rollout, "Enable Matter", &draw_matter, 0, control_cb);
    glui->add_checkbox_to_panel(matter_rollout, "Enable Matter Colorbar", &draw_matter_colorbar, 0, control_cb);


    GLUI_Panel *dataset_panel = new GLUI_Panel (matter_rollout, "Dataset Selection");
    GLUI_Listbox *matter_dataset_lb = glui->add_listbox_to_panel(dataset_panel, "", &matter_dataset);
    matter_dataset_lb->add_item(SCALAR_RHO, "Fluid Density");
    matter_dataset_lb->add_item(SCALAR_VELOC_MAG, "Fluid Velocity Magnitude");
    matter_dataset_lb->add_item(SCALAR_FORCE_MAG, "Force Field Magnitude");
    matter_dataset_lb->add_item(SCALAR_VELOC_DIV, "Velocity Field Divergency");
    matter_dataset_lb->add_item(SCALAR_FORCE_DIV, "Force Field Divergency");

    GLUI_Panel *color_panel = new GLUI_Panel (matter_rollout, "Color Mapping");
    GLUI_Listbox *scalar_colormap_lb = glui->add_listbox_to_panel(color_panel, "Colormap: ", &scalar_colormap, UPDATE_SCALAR_COLORMAP, control_cb);
    scalar_colormap_lb->add_item(COLOR_RAINBOW, "Rainbow");
    scalar_colormap_lb->add_item(COLOR_BLACKWHITE, "Greyscale");
    scalar_colormap_lb->add_item(COLOR_FIRE, "Fire");
    scalar_colormap_lb->add_item(COLOR_CUSTOM, "Custom");
    //scalar_colormap_lb->add_item(COLOR_RAINBOW_1D, "Rainbow (1D Texture)");
    new GLUI_Button(color_panel, "Edit custom colormap", COLOR_CUSTOM, control_cb);
    glui->add_edittext_to_panel(color_panel, "Quantize:", GLUI_EDITTEXT_INT, &quantize_colormap, QUANT_ID, control_cb);

    GLUI_Panel *clamp_ro = glui->add_panel_to_panel(matter_rollout, "Options", true);
    glui->add_checkbox_to_panel(clamp_ro, "Clampling", &clamp_flag, 0, control_cb);
    glui->add_edittext_to_panel(clamp_ro, "MIN", GLUI_EDITTEXT_FLOAT, &clamp_min);
    glui->add_edittext_to_panel(clamp_ro, "MAX", GLUI_EDITTEXT_FLOAT, &clamp_max);
    glui->add_checkbox_to_panel(clamp_ro, "Scaling", &scaling_flag, 0, control_cb);

    matter_rollout->close();

    // Glyphs Parameters
    GLUI_Rollout *glyph_rollout = glui->add_rollout("Glyphs", true);
    GLUI_StaticText *spacer_glyph = glui->add_statictext_to_panel(glyph_rollout, "");
    spacer_glyph->set_w(260);
    spacer_glyph->set_h(0);
    glui->add_checkbox_to_panel(glyph_rollout, "Enable Glyphs", &draw_glyphs_flag, 0, control_cb);

    GLUI_Listbox *vector_dataset_lb = glui->add_listbox_to_panel(glyph_rollout, "Vector Field:", &glyphs.vector_field);
    vector_dataset_lb->add_item(VECTOR_VELOC, "Fluid Velocity Field");
    vector_dataset_lb->add_item(VECTOR_FORCE, "Force Field");

    GLUI_Listbox *scalar_dataset_glyph_lb = glui->add_listbox_to_panel(glyph_rollout, "Color value:", &glyphs.scalar_field);
    scalar_dataset_glyph_lb->add_item(SCALAR_RHO, "Fluid Density");
    scalar_dataset_glyph_lb->add_item(SCALAR_VELOC_MAG, "Fluid Velocity Magnitude");
    scalar_dataset_glyph_lb->add_item(SCALAR_FORCE_MAG, "Force Field Magnitude");
    scalar_dataset_glyph_lb->add_item(SCALAR_DIR, "Vector Direction");
    scalar_dataset_glyph_lb->add_item(SCALAR_WHITE, "White");

    GLUI_Listbox *glyph_type_lb = glui->add_listbox_to_panel(glyph_rollout, "Glypth Type:", &glyphs.glyph_type);
    glyph_type_lb->add_item(GLYPH_ARROW, "Arrow");
    glyph_type_lb->add_item(GLYPH_NEEDLE, "Needle");
    glyph_type_lb->add_item(GLYPH_LINE, "Line");

    GLUI_Panel *hedgehog_panel = new GLUI_Panel (glyph_rollout, "Glyph Scaling");
    hh_text = glui->add_statictext_to_panel(hedgehog_panel, "");
    new GLUI_Button(hedgehog_panel, "Increase", HH_INCREASE_ID, control_cb);
    new GLUI_Button(hedgehog_panel, "Decrease", HH_DECREASE_ID, control_cb);


    GLUI_Panel *sample_panel = new GLUI_Panel (glyph_rollout, "Number of samples");
    glui->add_edittext_to_panel(sample_panel, "X: ", GLUI_EDITTEXT_INT, &glyphs.x_axis_samples);
    glui->add_edittext_to_panel(sample_panel, "Y: ", GLUI_EDITTEXT_INT, &glyphs.y_axis_samples);
    glui->add_checkbox_to_panel(sample_panel, "Enable Jitter", &glyphs.jitter, 0, control_cb);

    glyph_rollout->close();

    // Isolines
    GLUI_Rollout *iso_rollout = glui->add_rollout ("Isolines", true);
    GLUI_StaticText *spacer_iso = glui->add_statictext_to_panel(iso_rollout, "");
    spacer_iso->set_w(260);
    spacer_iso->set_h(0);
    glui->add_checkbox_to_panel(iso_rollout, "Enable Isolines", &draw_isolines_flag, 0, control_cb);
    glui->add_checkbox_to_panel(iso_rollout, "Enable Isolines Colorbar", &draw_isolines_colorbar, 0, control_cb);
    glui->add_edittext_to_panel(iso_rollout, "v1", GLUI_EDITTEXT_FLOAT, &isoline_manager.v1);
    glui->add_edittext_to_panel(iso_rollout, "v2", GLUI_EDITTEXT_FLOAT, &isoline_manager.v2);
    glui->add_edittext_to_panel(iso_rollout, "n" , GLUI_EDITTEXT_INT,   &isoline_manager.n);
    GLUI_Listbox *scalar_dataset_lb = glui->add_listbox_to_panel(iso_rollout, "Colormap: ", &isoline_colormap);
    scalar_dataset_lb->add_item(COLOR_CUSTOM, "Custom");
    scalar_dataset_lb->add_item(COLOR_BLACKWHITE, "Greyscale");
    scalar_dataset_lb->add_item(COLOR_RAINBOW, "Rainbow");
    scalar_dataset_lb->add_item(COLOR_FIRE, "Fire");
    new GLUI_Button(iso_rollout, "Apply paramenters", APPLY_ISOLINES, control_cb);
    iso_rollout->close();

    // Height Plot Rollout
    GLUI_Rollout *height_rollout = glui->add_rollout ("Height Plot", true);
    GLUI_StaticText *spacer_height = glui->add_statictext_to_panel(height_rollout, "");
    spacer_height->set_w(260); spacer_height->set_h(0);
    glui->add_checkbox_to_panel(height_rollout, "Enable Height Plot", &draw_height_flag, ENABLE_HEIGHT, control_cb);

    GLUI_Listbox *scalar_dataset_height_lb = glui->add_listbox_to_panel(height_rollout, "Height value:", &matter_dataset);
    scalar_dataset_height_lb->add_item(SCALAR_RHO, "Fluid Density");
    scalar_dataset_height_lb->add_item(SCALAR_VELOC_MAG, "Fluid Velocity Magnitude");
    scalar_dataset_height_lb->add_item(SCALAR_FORCE_MAG, "Force Field Magnitude");

    GLUI_Listbox *height_colormap_dataset_lb = glui->add_listbox_to_panel(height_rollout, "Coloring dataset: ", &height_dataset_coloring);
    height_colormap_dataset_lb->add_item(SCALAR_RHO, "Fluid Density");
    height_colormap_dataset_lb->add_item(SCALAR_VELOC_MAG, "Fluid Velocity Magnitude");
    height_colormap_dataset_lb->add_item(SCALAR_FORCE_MAG, "Force Field Magnitude");
    height_colormap_dataset_lb->add_item(SCALAR_VELOC_DIV, "Velocity Field Divergency");
    height_colormap_dataset_lb->add_item(SCALAR_FORCE_DIV, "Force Field Divergency");

    GLUI_Panel *eye_x_panel = new GLUI_Panel (height_rollout, "");
    eye_x_edittext = glui->add_edittext_to_panel(eye_x_panel, "Eye x:", GLUI_EDITTEXT_FLOAT, &eye_x);
    glui->add_column_to_panel(eye_x_panel, false);
    GLUI_Button *increase_eye_x = new GLUI_Button(eye_x_panel, "+", EYE_X_INCREASE_ID, control_cb);
    increase_eye_x->set_w(10);
    glui->add_column_to_panel(eye_x_panel, false);
    GLUI_Button *decrease_eye_x = new GLUI_Button(eye_x_panel, "-", EYE_X_DECREASE_ID, control_cb);
    decrease_eye_x->set_w(10);

    GLUI_Panel *eye_y_panel = new GLUI_Panel (height_rollout, "");
    eye_y_edittext = glui->add_edittext_to_panel(eye_y_panel, "Eye y:", GLUI_EDITTEXT_FLOAT, &eye_y);
    glui->add_column_to_panel(eye_y_panel, false);
    GLUI_Button *increase_eye_y = new GLUI_Button(eye_y_panel, "+", EYE_Y_INCREASE_ID, control_cb);
    increase_eye_y->set_w(10);
    glui->add_column_to_panel(eye_y_panel, false);
    GLUI_Button *decrease_eye_y = new GLUI_Button(eye_y_panel, "-", EYE_Y_DECREASE_ID, control_cb);
    decrease_eye_y->set_w(10);

    GLUI_Panel *eye_z_panel = new GLUI_Panel (height_rollout, "");
    eye_z_edittext = glui->add_edittext_to_panel(eye_z_panel, "Eye z:", GLUI_EDITTEXT_FLOAT, &eye_z);
    glui->add_column_to_panel(eye_z_panel, false);
    GLUI_Button *increase_eye_z = new GLUI_Button(eye_z_panel, "+", EYE_Z_INCREASE_ID, control_cb);
    increase_eye_z->set_w(10);
    glui->add_column_to_panel(eye_z_panel, false);
    GLUI_Button *decrease_eye_z = new GLUI_Button(eye_z_panel, "-", EYE_Z_DECREASE_ID, control_cb);
    decrease_eye_z->set_w(10);

    GLUI_Panel *c_x_panel = new GLUI_Panel (height_rollout, "");
    c_x_edittext = glui->add_edittext_to_panel(c_x_panel, "C x:", GLUI_EDITTEXT_FLOAT, &c_x);
    glui->add_column_to_panel(c_x_panel, false);
    GLUI_Button *increase_c_x = new GLUI_Button(c_x_panel, "+", C_X_INCREASE_ID, control_cb);
    increase_c_x->set_w(10);
    glui->add_column_to_panel(c_x_panel, false);
    GLUI_Button *decrease_c_x = new GLUI_Button(c_x_panel, "-", C_X_DECREASE_ID, control_cb);
    decrease_c_x->set_w(10);

    GLUI_Panel *c_y_panel = new GLUI_Panel (height_rollout, "");
    c_y_edittext = glui->add_edittext_to_panel(c_y_panel, "C y:", GLUI_EDITTEXT_FLOAT, &c_y);
    glui->add_column_to_panel(c_y_panel, false);
    GLUI_Button *increase_c_y = new GLUI_Button(c_y_panel, "+", C_Y_INCREASE_ID, control_cb);
    increase_c_y->set_w(10);
    glui->add_column_to_panel(c_y_panel, false);
    GLUI_Button *decrease_c_y = new GLUI_Button(c_y_panel, "-", C_Y_DECREASE_ID, control_cb);
    decrease_c_y->set_w(10);

    GLUI_Panel *c_z_panel = new GLUI_Panel (height_rollout, "");
    c_z_edittext = glui->add_edittext_to_panel(c_z_panel, "C z:", GLUI_EDITTEXT_FLOAT, &c_z);
    glui->add_column_to_panel(c_z_panel, false);
    GLUI_Button *increase_c_z = new GLUI_Button(c_z_panel, "+", C_Z_INCREASE_ID, control_cb);
    increase_c_z->set_w(10);
    glui->add_column_to_panel(c_z_panel, false);
    GLUI_Button *decrease_c_z = new GLUI_Button(c_z_panel, "-", C_Z_DECREASE_ID, control_cb);
    decrease_c_z->set_w(10);

    GLUI_Panel *scale_panel = new GLUI_Panel (height_rollout, "");
    scale_edittext = glui->add_edittext_to_panel(scale_panel, "Scale dataset:", GLUI_EDITTEXT_FLOAT, &dataset_scale);
    glui->add_column_to_panel(scale_panel, false);
    GLUI_Button *increase_scale = new GLUI_Button(scale_panel, "+", SCALE_INCREASE_ID, control_cb);
    increase_scale->set_w(10);
    glui->add_column_to_panel(scale_panel, false);
    GLUI_Button *decrease_scale = new GLUI_Button(scale_panel, "-", SCALE_DECREASE_ID, control_cb);
    decrease_scale->set_w(10);

    height_rollout->close();

    // Stream tubes
    GLUI_Rollout *st_rollout = glui->add_rollout ("Stream Tubes", true);
    GLUI_StaticText *spacer_st = glui->add_statictext_to_panel(st_rollout, "");
    spacer_st->set_w(260);
    spacer_st->set_h(0);
    glui->add_checkbox_to_panel(st_rollout, "Enable Stream Tubes", &draw_st_flag, ENABLE_ST, control_cb);

    GLUI_Listbox *st_radius_lb = glui->add_listbox_to_panel(st_rollout, "Radius:", &st_radius);
    st_radius_lb->add_item(SCALAR_RHO, "Fluid Density");
    st_radius_lb->add_item(SCALAR_VELOC_MAG, "Fluid Velocity Magnitude");
    st_radius_lb->add_item(SCALAR_FORCE_MAG, "Force Field Magnitude");
    st_radius_lb->add_item(3, "3");
    st_radius_lb->add_item(10, "10");

    glui->add_edittext_to_panel(st_rollout, "Z scaling:", GLUI_EDITTEXT_FLOAT, &st_height);
    new GLUI_Button(st_rollout, "Reset Stream Tubes", RESET_ST, control_cb);

    st_rollout->close();

    new GLUI_Button(glui, "Pause/Play", PP_ID, control_cb);
    new GLUI_Button(glui, "Quit", QT_ID, control_cb);

    update_variables_config_window(); // update values of statictexts
    GLUI_Master.auto_set_viewport();
    glui->set_main_gfx_window(main_window);
}
