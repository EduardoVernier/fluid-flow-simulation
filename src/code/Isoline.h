#ifndef ISOLINE_H
#define ISOLINE_H

using namespace std;

#include <vector>
extern int DIM;

class Isoline
{
private:
    void find_point(int ih, int il);

public:
    Isoline(float _v, double *_sfp) : v(_v), sfp(_sfp) {};
    void compute_isoline();
    // TODO: implement destructor freeing the vector

    float v;
    double *sfp; // current scalar field pointer (e.g. rho)
    vector<pair<float, float> > points;
};


#endif
