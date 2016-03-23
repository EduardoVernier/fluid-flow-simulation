#ifndef STREAMTUBE_H
#define STREAMTUBE_H

#include <math.h>
#include <vector>
#include "Slices.h"
extern Slices slices;

using namespace std;

class StreamTube
{
public:
    StreamTube(int x, int y) : p0x(x), p0y(y) {};
    void calc_all_points(float dt);
    int vf_bilinear_interpolation(float *vec, int slice_idx, float i, float j);

    vector< pair<float, float> > stream_tube_points;
    int p0x, p0y;
    int n_points = slices.get_number_of_slices();
};

#endif
