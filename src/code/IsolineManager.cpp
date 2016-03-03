#include "IsolineManager.h"

IsolineManager::IsolineManager()
{
    v1 = 0.3;
    v2 = 0;
    n = 1;

    // for testing purposes
    extern double *rho;
    Isoline test = Isoline(v1, rho);
    isoline_vector.push_back(test);
}

void IsolineManager::compute_isolines()
{
    for(vector<Isoline>::iterator it = isoline_vector.begin(); it != isoline_vector.end(); ++it)
        it->compute_isoline();
}
