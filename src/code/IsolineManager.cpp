#include "IsolineManager.h"
#include "stdio.h"

IsolineManager::IsolineManager()
{
    v1 = 0.1;
    v2 = 0.8;
    n = 5;

}

void IsolineManager::reset()
{
    isoline_vector.clear();
    for (int i = 1; i <= n; ++i)
    {
        Isoline t = Isoline(v1 + i*((v2-v1)/(float)n), rho);
        isoline_vector.push_back(t);
        printf("%f ", v1 + i*((v2-v1)/(float)n));
    }
    printf("\n");
}

void IsolineManager::create_isoline()
{
    // for testing purposes
    Isoline test = Isoline(v1, rho);
    isoline_vector.push_back(test);
}

void IsolineManager::compute_isolines()
{
    for(vector<Isoline>::iterator it = isoline_vector.begin(); it != isoline_vector.end(); ++it)
        it->compute_isoline();
}
