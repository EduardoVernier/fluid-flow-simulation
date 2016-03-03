#include "Isoline.h"

void Isoline::compute_isoline()
{
    points.clear();
    for (int i = 0; i < DIM; ++i)
    {
        for (int j = 0; j < DIM; ++j)
        {
            // check all 4 edges of cell
            find_point(i*DIM + j, i*DIM + j+1);
            find_point(i*DIM + j, (i+1)*DIM + j);
            find_point((i+1)*DIM + j+1, (i+1)*DIM + j);
            find_point((i+1)*DIM + j+1, i*DIM + j+1);
        }
    }
}

// Implementation of book algorithm
void Isoline::find_point(int ih, int il)
{

    // ih - higher sf value, il - lower sf value
    if (sfp[ih] < sfp[il])
    {
        int tmp = ih;
        ih = il;
        il = tmp;
    }

    double vh = sfp[ih];
    double vl = sfp[il];

    if(vh > v && vl < v)
    {
        // rebuild coordinates from array index
        int phx = ih/DIM;
        int phy = ih%DIM;
        int plx = il/DIM;
        int ply = il%DIM;

        float q1 = (plx*(vh - v) + phx*(v - vl)) / (vh - vl);
        float q2 = (ply*(vh - v) + phy*(v - vl)) / (vh - vl);

        points.push_back(make_pair(q1, q2));
    }
}
