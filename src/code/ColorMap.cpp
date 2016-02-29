#include "ColorMap.h"


Color ColorMap::interpolate(Color a, Color b, float value)
{
    float nr = a.r*(1-value) + b.r*(value);
    float ng = a.g*(1-value) + b.g*(value);
    float nb = a.b*(1-value) + b.b*(value);

    Color c = Color(nr, ng, nb);
    return c;
}

int ColorMap::add_color_range(Color a, Color b, float x, float y)
{
    auto t = make_tuple(a,b,x,y);
    color_ranges.push_back(t);
    return 0; //for now - needs verification
}


Color ColorMap::get_color(float value)
{
    if (strcmp(this->name, "Rainbow") == 0)
    {
        return rainbow(value);
    }
    else if (strcmp(this->name, "Black and White") == 0)
    {
        return Color(value, value, value);
    }
    else
    {
        Color c = Color(1,1,1);
        for (unsigned it = 0; it < color_ranges.size(); ++it)
        {
            float x, y;
            Color a = get<0>(color_ranges[it]);
            Color b = get<1>(color_ranges[it]);
            x = get<2>(color_ranges[it]);
            y = get<3>(color_ranges[it]);

            if (value >= x && value <= y)
            {
                c = interpolate(a, b, (value - x)/(y - x));
            }
        }
        return c;
    }
}

//rainbow: Implements a color palette, mapping the scalar 'value' to a rainbow color RGB
Color ColorMap::rainbow(float value)
{
    const float dx=0.8;
    if (value<0)
        value = 0;
    if (value>1)
        value = 1;

    value = (6-2*dx)*value+dx;
    float R = max(0.0,(3-fabs(value-4)-fabs(value-5))/2);
    float G = max(0.0,(4-fabs(value-2)-fabs(value-4))/2);
    float B = max(0.0,(3-fabs(value-1)-fabs(value-2))/2);

    Color c = Color(R,G,B);
    return c;
}
