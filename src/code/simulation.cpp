#define SCALAR_RHO 150
#define SCALAR_VELOCITY 151
#define SCALAR_FORCE 152

int DIM = 50; 					//size of simulation grid
float dt = 0.4;                 //simulation time step
float visc = 0.001;				//fluid viscosity
fftw_real *vx, *vy;             //(vx,vy)   = velocity field at the current moment
double *der_vx, *der_vy;     //(vx,vy)   = derivate of the velocity field at the current moment
fftw_real *v_mag;               //vector that stores velocity magnitude dataset
fftw_real *vx0, *vy0;           //(vx0,vy0) = velocity field at the previous moment
fftw_real *fx, *fy;	            //(fx,fy)   = user-controlled simulation forces, steered with the mouse
fftw_real *f_mag;               //vector that stores force magnitude values
fftw_real *rho, *rho0;			//smoke density at the current (rho) and previous (rho0) moment
rfftwnd_plan plan_rc, plan_cr;  //simulation domain discretization

extern int main_window;
extern int frozen;
extern float dataset_max, dataset_min;
extern int dataset_id;

//init_simulation: Initialize simulation data structures as a function of the grid size 'n'.
//                 Although the simulation takes place on a 2D grid, we allocate all data structures as 1D arrays,
//                 for compatibility with the FFTW numerical library.
void init_simulation(int n)
{
	int i; size_t dim;

	dim     = n * 2*(n/2+1)*sizeof(fftw_real);        //Allocate data structures
    vx      = (fftw_real*) malloc(dim);
    vy      = (fftw_real*) malloc(dim);
    der_vx  = (fftw_real*) malloc(dim);
    der_vy  = (fftw_real*) malloc(dim);
    v_mag   = (fftw_real*) malloc(dim);
    vx0     = (fftw_real*) malloc(dim);
    vy0     = (fftw_real*) malloc(dim);
    dim     = n * n * sizeof(fftw_real);
	fx      = (fftw_real*) malloc(dim);
	fy      = (fftw_real*) malloc(dim);
    f_mag   = (fftw_real*) malloc(dim);
    rho     = (fftw_real*) malloc(dim);
	rho0    = (fftw_real*) malloc(dim);
	plan_rc = rfftw2d_create_plan(n, n, FFTW_REAL_TO_COMPLEX, FFTW_IN_PLACE);
	plan_cr = rfftw2d_create_plan(n, n, FFTW_COMPLEX_TO_REAL, FFTW_IN_PLACE);

	for (i = 0; i < n * n; i++)                      //Initialize data structures to 0
    {
        vx[i] = vy[i] = der_vx[i] = der_vy[i] = 0.0f;
        v_mag[i] = vx0[i] = vy0[i] = fx[i] = fy[i] = f_mag[i] = rho[i] = rho0[i] = 0.0f;
    }

}


//FFT: Execute the Fast Fourier Transform on the dataset 'vx'.
//     'dirfection' indicates if we do the direct (1) or inverse (-1) Fourier Transform
void FFT(int direction,void* vx)
{
	if(direction==1)
		rfftwnd_one_real_to_complex(plan_rc,(fftw_real*)vx,(fftw_complex*)vx);
	else
		rfftwnd_one_complex_to_real(plan_cr,(fftw_complex*)vx,(fftw_real*)vx);
}

int clamp(float x)
{
	return ((x)>=0.0?((int)(x)):(-((int)(1-(x)))));
}

float max(float x, float y)
{
	return x > y ? x : y;
}

//solve: Solve (compute) one step of the fluid flow simulation
void solve(int n, fftw_real* vx, fftw_real* vy, fftw_real* vx0, fftw_real* vy0, fftw_real visc, fftw_real dt)
{
    fftw_real x, y, x0, y0, f, r, U[2], V[2], s, t;
    int i, j, i0, j0, i1, j1;

    for (i=0;i<n*n;i++)
    {
        vx[i] += dt*vx0[i]; vx0[i] = vx[i]; vy[i] += dt*vy0[i]; vy0[i] = vy[i];
    }

    for ( x=0.5f/n,i=0 ; i<n ; i++,x+=1.0f/n )
    {
        for ( y=0.5f/n,j=0 ; j<n ; j++,y+=1.0f/n )
        {
            x0 = n*(x-dt*vx0[i+n*j])-0.5f;
            y0 = n*(y-dt*vy0[i+n*j])-0.5f;
            i0 = clamp(x0); s = x0-i0;
            i0 = (n+(i0%n))%n;
            i1 = (i0+1)%n;
            j0 = clamp(y0); t = y0-j0;
            j0 = (n+(j0%n))%n;
            j1 = (j0+1)%n;
            vx[i+n*j] = (1-s)*((1-t)*vx0[i0+n*j0]+t*vx0[i0+n*j1])+s*((1-t)*vx0[i1+n*j0]+t*vx0[i1+n*j1]);
            vy[i+n*j] = (1-s)*((1-t)*vy0[i0+n*j0]+t*vy0[i0+n*j1])+s*((1-t)*vy0[i1+n*j0]+t*vy0[i1+n*j1]);
        }
    }
    for(i=0; i<n; i++)
        for(j=0; j<n; j++)
        {
            vx0[i+(n+2)*j] = vx[i+n*j]; vy0[i+(n+2)*j] = vy[i+n*j];
        }

    FFT(1,vx0);
    FFT(1,vy0);

    for (i=0;i<=n;i+=2)
    {
        x = 0.5f*i;
        for (j=0;j<n;j++)
        {
            y = j<=n/2 ? (fftw_real)j : (fftw_real)j-n;
            r = x*x+y*y;
            if ( r==0.0f ) continue;
            f = (fftw_real)exp(-r*dt*visc);
            U[0] = vx0[i  +(n+2)*j]; V[0] = vy0[i  +(n+2)*j];
            U[1] = vx0[i+1+(n+2)*j]; V[1] = vy0[i+1+(n+2)*j];

            vx0[i  +(n+2)*j] = f*((1-x*x/r)*U[0]     -x*y/r *V[0]);
            vx0[i+1+(n+2)*j] = f*((1-x*x/r)*U[1]     -x*y/r *V[1]);
            vy0[i+  (n+2)*j] = f*(  -y*x/r *U[0] + (1-y*y/r)*V[0]);
            vy0[i+1+(n+2)*j] = f*(  -y*x/r *U[1] + (1-y*y/r)*V[1]);
        }
    }

    FFT(-1,vx0);
    FFT(-1,vy0);

    f = 1.0/(n*n);
    for (i=0;i<n;i++)
    {
        for (j=0;j<n;j++)
        {
            vx[i+n*j] = f*vx0[i+(n+2)*j]; vy[i+n*j] = f*vy0[i+(n+2)*j];
        }

    }
}

void compute_derivatives()
{

    for(int i = 0; i < DIM; ++i)
    {
        for (int j = 0; j < DIM; ++j)
        {
            // calculate x axis derivative
            if (j == DIM-1) // make algorithm circular
                der_vx[i*DIM + j] = vx[i*DIM] - vx[i*DIM + j];
            else
                der_vx[i*DIM + j] = vx[i*DIM + j + 1] - vx[i*DIM + j];

            // calculate y axis derivative
            if (i == DIM - 1)
                der_vy[i*DIM + j] = vy[j] - vy[i*DIM + j];
            else
                der_vy[i*DIM + j] = vy[(i+1)*DIM + j] - vy[i*DIM + j];
        }
    }
}

// diffuse_matter: This function diffuses matter that has been placed in the velocity field. It's almost identical to the
// velocity diffusion step in the function above. The input matter densities are in rho0 and the result is written into rho.
void diffuse_matter(int n, fftw_real *vx, fftw_real *vy, fftw_real *rho, fftw_real *rho0, fftw_real dt)
{
	fftw_real x, y, x0, y0, s, t;
	int i, j, i0, j0, i1, j1;

    fftw_real *dataset;
    if (dataset_id == SCALAR_RHO)
        dataset = rho;
    else if (dataset_id == SCALAR_VELOCITY)
        dataset = v_mag;
    else if (dataset_id == SCALAR_FORCE)
        dataset = f_mag;

    float min_ds = 10000, max_ds = -10000;

	for ( x=0.5f/n,i=0 ; i<n ; i++,x+=1.0f/n )
		for ( y=0.5f/n,j=0 ; j<n ; j++,y+=1.0f/n )
		{
            // Calculate velocity vector magnitude
            v_mag[i+n*j] = sqrt (vx[i+n*j]*vx[i+n*j] + vy[i+n*j]*vy[i+n*j]);
            // Calculate force field vector magnitude
            f_mag[i+n*j] = sqrt (fx[i+n*j]*fx[i+n*j] + fy[i+n*j]*fy[i+n*j]);

			x0 = n*(x-dt*vx[i+n*j])-0.5f;
			y0 = n*(y-dt*vy[i+n*j])-0.5f;
			i0 = clamp(x0);
			s = x0-i0;
			i0 = (n+(i0%n))%n;
			i1 = (i0+1)%n;
			j0 = clamp(y0);
			t = y0-j0;
			j0 = (n+(j0%n))%n;
			j1 = (j0+1)%n;
			rho[i+n*j] = (1-s)*((1-t)*rho0[i0+n*j0]+t*rho0[i0+n*j1])+s*((1-t)*rho0[i1+n*j0]+t*rho0[i1+n*j1]);

            // get dataset max/min in case scaling is on
            if (dataset[i+n*j] > max_ds) max_ds = dataset[i+n*j];
            if (dataset[i+n*j] < min_ds) min_ds = dataset[i+n*j];
        }
    // set values used if scaling is on
    dataset_max = max_ds;
    dataset_min = min_ds;
    //printf ("%f  %f\n", min_ds, max_ds);
    compute_derivatives();
}

//set_forces: copy user-controlled forces to the force vectors that are sent to the solver.
//            Also dampen forces and matter density to get a stable simulation.
void set_forces(void)
{
	int i;
	for (i = 0; i < DIM * DIM; i++)
	{
        rho0[i]  = 0.995 * rho[i];
        fx[i] *= 0.85;
        fy[i] *= 0.85;
        vx0[i]    = fx[i];
        vy0[i]    = fy[i];
	}
}


//do_one_simulation_step: Do one complete cycle of the simulation:
//      - set_forces:
//      - solve:            read forces from the user
//      - diffuse_matter:   compute a new set of velocities
//      - gluPostRedisplay: draw a new visualization frame
void do_one_simulation_step(void)
{
	if ( glutGetWindow() != main_window )
		glutSetWindow(main_window);

	if (!frozen)
	{
	  set_forces();
	  solve(DIM, vx, vy, vx0, vy0, visc, dt);
	  diffuse_matter(DIM, vx, vy, rho, rho0, dt);
	  glutPostRedisplay();
	}
}
