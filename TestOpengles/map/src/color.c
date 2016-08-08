#include"color.h"

unsigned int     
color_argb
(unsigned int a,
unsigned int r,
unsigned int g,
unsigned int b)
{
    return (a << 24) | (r << 16) | (g << 8) | b;
}

unsigned int     
color_rgb
(unsigned int r,
unsigned int g,
unsigned int b)
{
    return (0xFF << 24) | (r << 16) | (g << 8) | b;
}


unsigned int    
color_alpha
(unsigned int color)
{
    return color >> 24;
}

unsigned int    
color_red
(unsigned int color)
{
    return (color >> 16) & 0xFF;
}


unsigned int    
color_green
(unsigned int color)
{
    return (color >> 8) & 0xFF;
}


unsigned int    
color_blue
(unsigned int color)
{
    return color & 0xFF;
}


double           
color_alpha_range
(unsigned int color)
{
    return color_alpha(color) / 255.0;
}


double           
color_red_range
(unsigned int color)
{
    return color_red(color) / 255.0;
}


double           
color_green_range
(unsigned int color)
{
    return color_green(color) / 255.0;
}


double           
color_blue_range
(unsigned int color)
{
    return color_blue(color) / 255.0;
}


