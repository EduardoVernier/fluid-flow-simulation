#ifndef GLOBALS_H
#define GLOBALS_H

enum GLOBAL_IDS
{
// different types of color mapping: black-and-white, rainbow, fire or custom
COLOR_BLACKWHITE = 100,
COLOR_RAINBOW,
COLOR_RAINBOW_1D,
COLOR_FIRE,
COLOR_CUSTOM,
// scalar field selector id on listbox
SCALAR_RHO = 140,
SCALAR_VELOC_MAG,
SCALAR_FORCE_MAG,
SCALAR_VELOC_DIV,
SCALAR_FORCE_DIV,
SCALAR_DIR,
SCALAR_WHITE,
// vector fields ids
VECTOR_VELOC = 160,
VECTOR_FORCE,
// glyph type ids
GLYPH_LINE = 170,
GLYPH_ARROW,
GLYPH_NEEDLE
};

#endif
