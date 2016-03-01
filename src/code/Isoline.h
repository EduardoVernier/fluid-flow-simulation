#ifndef ISOLINE_H
#define ISOLINE_H

#import <vector>


class Isolines
{
    Isolines(float _v, double *_sfp) : v(_v), spf(_sfp) {};
    void recompute_isoline ();
    // TODO: implement destructor freeing the vector

    float v;
    double *sfp; // current scalar field pointer (e.g. rho)
    vector<float[2]> points;
}


#endif
