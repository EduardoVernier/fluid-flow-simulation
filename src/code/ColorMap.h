#ifndef COLORMAP_H
#define COLORMAP_H

using namespace std;

#include <cstring>
#include <vector>
#include <tuple>
#include <math.h>

#include "Color.cpp"

extern float **custom_color_ranges;

class ColorMap
{
public:
    ColorMap(char* _name) { strcpy(name, _name); };
    int add_color_range(Color a, Color b, float x, float y);
    Color get_color(float value);

private:
    char name[50];
    Color rainbow(float value);
    Color interpolate(Color a, Color b, float value);


//usage: cm.add_color_range(black, red, 0.0, 0.4); cm.addRange(red, yellow, 0.4, 0.8); cm.addRange(yellow, white, 0.8, 2);
private:
    vector<tuple <Color, Color, float, float> > color_ranges;
};

#endif
