#ifndef ISOLINEMANAGER_H
#define ISOLINEMANAGER_H

// "import" as external variables the scalar fields we are interested in
extern double *vx, *vy, *v_mag, *fx, *fy, *f_mag, *rho, *div_vf;

class IsolineManager
{
public:
    IsolineManager();

    vecor <Isoline> isoline_vector;
    float v1, v2;
    int n;
}


#endif
