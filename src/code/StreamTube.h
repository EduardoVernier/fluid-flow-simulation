#ifndef STREAMTUBE_H
#define STREAMTUBE_H

#include <math.h>
#include <vector>
#include <tuple>
#include "Slices.h"
#include "Globals.cpp"

extern Slices slices;
extern double *vx, *vy, *v_mag, *fx, *fy, *f_mag, *rho;

using namespace std;

class StreamTube
{
public:
    StreamTube(int x, int y) : p0x(x), p0y(y) {};
    void calc_all_points(float dt);
    int vf_bilinear_interpolation(float *vec, int slice_idx, float i, float j);

    vector< tuple<float, float, float> > stream_tube_points;
    int p0x, p0y;
    int n_points = slices.number_of_slices;
    int scalar_field = SCALAR_RHO;
};

#endif
