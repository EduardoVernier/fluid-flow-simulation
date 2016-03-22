#ifndef SLICES_H
#define SLICES_H

#include <deque>
#include <array>
using namespace std;

extern int DIM;

class Slices
{
public:
    Slices(int n);
    void add_slice(double *_vf_x, double *_vf_y);
    int get_number_of_slices() { return number_of_slices; };
    deque <array<double, 2500> > slices_x;
    deque <array<double, 2500> > slices_y;

private:
    int number_of_slices;
};

#endif
