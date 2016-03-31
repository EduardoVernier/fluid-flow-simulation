#include "StreamTube.h"


void StreamTube::calc_all_points(float dt)
{
    float px, py, pnx, pny;
    float vec[2];
    int error = 0;
    stream_tube_points.clear();
    stream_tube_points.push_back(make_tuple(p0x, p0y, rho[int(floor(pnx)+50*floor(pny))]));
    for (int i = 0; i < slices.number_of_slices; ++i)
    {
        px = std::get<0>(stream_tube_points.back());
        py = std::get<1>(stream_tube_points.back());

        error = vf_bilinear_interpolation(vec, i, px, py);
        if (error) return;

        pnx = px + vec[0]*dt;
        pny = py + vec[1]*dt;

        if (isfinite(pnx)&&isfinite(pny))
            stream_tube_points.push_back(make_tuple(pnx, pny, rho[int(floor(pnx)+50*floor(pny))]));
        else
            return;

    }
}

int StreamTube::vf_bilinear_interpolation(float *vec, int slice_idx, float i, float j)
{
    if (i >= 0 && i <= 50 && j >= 0 && j <= 50)
    {
        double x_fxy1 = slices.slices_x.at(slice_idx)[int(floor(j)*DIM + floor(i))]*(1 - (i-floor(i)))
        + slices.slices_x.at(slice_idx)[int(floor(j)*DIM + ceil(i))]*(i-floor(i));

        double x_fxy2 = slices.slices_x.at(slice_idx)[int(ceil(j)*DIM + floor(i))]*(1 - (i-floor(i)))
        + slices.slices_x.at(slice_idx)[int(ceil(j)*DIM + ceil(i))]*(i-floor(i));

        double x_fxy = x_fxy1*(1-(j-floor(j))) + x_fxy2*(j-floor(j));

        double y_fxy1 = slices.slices_y.at(slice_idx)[int(floor(j)*DIM + floor(i))]*(1 - (i-floor(i)))
        + slices.slices_y.at(slice_idx)[int(floor(j)*DIM + ceil(i))]*(i-floor(i));

        double y_fxy2 = slices.slices_y.at(slice_idx)[int(ceil(j)*DIM + floor(i))]*(1 - (i-floor(i)))
        + slices.slices_y.at(slice_idx)[int(ceil(j)*DIM + ceil(i))]*(i-floor(i));

        double y_fxy = y_fxy1*(1-(j-floor(j))) + y_fxy2*(j-floor(j));

        vec[0] = x_fxy;
        vec[1] = y_fxy;
        return 0;
    }
    else
        return 1;

    //if (vec[0] < 0) vec[0] = 0;
    //if (vec[1] < 0) vec[1] = 0;


}
