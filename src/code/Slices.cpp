#include "Slices.h"


Slices::Slices(int n)
{
    number_of_slices = n;
}


void Slices::add_slice(double *_vf_x, double *_vf_y)
{

    array<double, 2500> vf_x;
    array<double, 2500> vf_y;

    for (int i = 0; i < DIM*DIM; ++i)
    {
        vf_x[i] = _vf_x[i];
        vf_y[i] = _vf_y[i];
    }

    slices_x.push_back(vf_x);
    slices_y.push_back(vf_y);

    // after filed, deque always maintains same size (number_of_slices)
    if (int(slices_x.size()) > number_of_slices)
    {
        slices_x.pop_front();
        slices_y.pop_front();
    }

}
