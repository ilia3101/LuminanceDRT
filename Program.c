/*
    LuminanceDRT - Luminance based image formation
    Copyright (C) 2022  Ilia Sibiryakov

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; strictly version 2 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

/* 
This is the main program that does the image formation
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>

#include "Matrix.h"
#include "ColourPath.h"
#include "IPT.h"
#include "Utilities/Utilities.h"

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

/* Functions used by the code */
double do_contrast(double X, double Power, double Scale);

/* Compresses a value from 0-infinity to 0-1
 * Passing 1 to power = very smooth, higher values = sharper roll off */
float compress_value(float x, float power);

/* Finds a constant hue rib along the gamut's top surface. */
void find_RGB_hull_rib(double * XYZ_to_RGB, double * RGB_to_XYZ, float * EndRGB, int NumPoints, ColourPath_t * RibOut)
{
    /* Convert to perceptual space. Yeah too many lines of code. */
    float white_XYZ[3] = {1,1,1};
    float end_XYZ[3] = {EndRGB[0], EndRGB[1], EndRGB[2]};
    applyMatrix_f(white_XYZ, RGB_to_XYZ);
    applyMatrix_f(end_XYZ, RGB_to_XYZ);
    float white_CAM[3];     /* Perceptual coordinate of white */
    float end_CAM[3];       /* Perceptual coordinate of the end point */
    XYZ_to_IPT(white_XYZ, white_CAM, 1);
    XYZ_to_IPT(end_XYZ, end_CAM, 1);

    for (int i = 0; i < NumPoints; ++i)
    {
        float progress = ((float)i) / (NumPoints-1.0f);

        /* Interpolate between the white point and the colour point in perceptual space... */
        float result[3];
        for (int j = 0; j < 3; ++j)
        {
            result[j] = white_CAM[j] * progress + end_CAM[j] * (1.0f - progress);
        }

        /* Convert back to RGB... */
        IPT_to_XYZ(result, result, 1);
        applyMatrix_f(result, XYZ_to_RGB);

        /* Normalise so maxRGB = 1, this places te point on the hull's 'canopy' (maximum output brightness) */
        float max_rgb = MAX(result[0], MAX(result[1], result[2]));
        for (int j = 0; j < 3; ++j)
        {
            /* Normalise */
            result[j] /= max_rgb;
    
            /* Because some colours (THE REC709 BLUE PRIMARY) curve so strongly in perceptual space,
             * there is no straight line to white, so I clip negative channels to bring the path
             * back on to the edge of the gamut. In this case clipping is fine because
             * distances and precision don't matter, the path just needs to be brought to the
             * edge and will be interpolated on later. */
            if (result[j] < 0.0) result[j] = 0.0;
        }

        /* Convert back to perceptual space */
        applyMatrix_f(result, RGB_to_XYZ);
        XYZ_to_IPT(result, result, 1);

        /* Output the point to the colour path. */
        ColourPathAddPointByValue(RibOut, result);
    }
}


/* The 'paths' LUT is an exposure invariant (2D) LUT to find the 'path to white'
 * for a given chromaticity within the destination RGB space, it uses by RGB chromaticity for
 * 2D indexing - the same formula as xyY, except with RGB as inputs instead of XYZ. The main
 * issue is the very bad perceptual uniformity, other than that, works fine. Also only half of
 * it is actually used.
 * 
 * (Resultion should be 3n+1 so that the whitepoint falls on an integer coordinate)
 */
#define LUT_RESOLUTION 100
ColourPath_t paths[LUT_RESOLUTION][LUT_RESOLUTION];
/* (I put it as a global variable cause it would cause instant stack overflows when it's too big) */

int main(int argc, char ** argv)
{
    /* Main parameters that control everything */
    float compression_smoothness = 1.05; /* 1 = smoothest, higher values are sharper */
    float contrast_slope = 1.5;
    float saturation_factor = 1.0 * sqrt(contrast_slope);

    /* Rec709 will be our RGB space */
    double RGB_to_XYZ[9] = {
        0.4124564, 0.3575761, 0.1804375,
        0.2126729, 0.7151522, 0.0721750,
        0.0193339, 0.1191920, 0.9503041
    };
    double XYZ_to_RGB[9];
    invertMatrix(RGB_to_XYZ, XYZ_to_RGB);


    /* For a basic footprint compression. This is a TODO. */
    // float saturation_boundary = 0.0;
    // for (int p = 0; p < 3; ++p) {
    //     float value[3];
    //     Util_HSVToRGB(p / 3.0, 1.0, 1.0, value);
    //     applyMatrix_f(value, RGB_to_XYZ);
    //     XYZ_to_IPT(value, value, 1);
    //     value[1] /= value[0];
    //     value[2] /= value[0];
    //     float saturation = sqrt(value[1]*value[1] + value[2]*value[2]);
    //     if (saturation > saturation_boundary) saturation_boundary = saturation_boundary;
    // }
    // saturation_boundary *= 1.1;


    /***********************************************************/
    /***************** Create the LUT now... *******************/
    /***********************************************************/

    /* Generate paths */
    for (int r = 0; r < LUT_RESOLUTION; ++r)
    {
        float r_chromaticity = ((float)r) / (LUT_RESOLUTION-1.0);
        for (int g = 0; g < LUT_RESOLUTION; ++g)
        {
            float g_chromaticity = ((float)g) / (LUT_RESOLUTION-1.0);

            /* Restore the RGB value for this chromaticity, and normalise to maxRGB = 1 */
            float RGB[3] = {r_chromaticity, g_chromaticity, 1.0 - r_chromaticity - g_chromaticity};
            float max_rgb = MAX(RGB[0], MAX(RGB[1], RGB[2]));
            for (int c = 0; c < 3; ++c) RGB[c] /= max_rgb;

            /* Generate a path like this:
             * - Black point
             * - Maximum intensity colour on the top surface of the gamut
             * - A hue linear rib until white, at maximum RGB intensity
             */
            int rib_length = 20;
            int path_length = rib_length+1;
            ColourPath_t path;
            init_ColourPath(&path);
            /* The black point... */
            ColourPathAddPointByValues(&path, 0,0,0);
            /* Now the rib */
            find_RGB_hull_rib(XYZ_to_RGB, RGB_to_XYZ, RGB, rib_length, &path);
            /* Now update path distance values, for interpolating along them */
            ColourPathCalculateDistance(&path, 1.0, 1.0, 1.0);

            /* Now create the rounded path */
            ColourPath_t * path_final = &paths[r][g];
            init_ColourPath(path_final);
            ColourPathAddPoint(path_final, (ColourPathPoint_t){.distance = 0, .value = {0,0,0}});

            int bevel_resolution = 20; /* How many points the rounded section will have */
            float bevel_scale = 0.4;
            float bevel_mid = ColourPathGetDistanceOfPoint(&path, 1);
            float bevel_start = bevel_mid * (1.0-bevel_scale);
            float bevel_end = bevel_mid + (bevel_mid - bevel_start);
            if (bevel_end > ColourPathGetLength(&path)) bevel_end = ColourPathGetLength(&path);

            /* Do bevel... */
            for (int b = 0; b < bevel_resolution; ++b)
            {
                float fac = ((float)b) / (bevel_resolution-1.0);
                float pos_a = fac * (bevel_mid - bevel_start) + bevel_start;
                float pos_b = fac * (bevel_end - bevel_mid) + bevel_mid;

                float value_a[3];
                float value_b[3];

                ColourPathInterpolate(&path, pos_a, value_a);
                ColourPathInterpolate(&path, pos_b, value_b);

                float value[3];
                for (int i = 0; i < 3; ++i)
                    value[i] = value_a[i] * (1.0-fac) + value_b[i] * fac;

                ColourPathAddPointByValue(path_final, value);
            }

            for (int p = 0; p < ColourPathGetNumPoints(&path); ++p)
            {
                if (ColourPathGetDistanceOfPoint(&path, p) > bevel_end)
                {
                    ColourPathAddPoint(path_final, ColourPathGetPoint(&path, p));
                }
            }

            /* Convert each point to RGB now, and set luminance as the 'distance'.
             * The path will be interpolated along using luminance (Y) and will return
             * the resulting RGB values directly */
            for (int p = 0; p < ColourPathGetNumPoints(path_final); ++p)
            {
                ColourPathPoint_t * point = path_final->points + p;
                float XYZ[3];
                IPT_to_XYZ(point->value, XYZ, 1);
                point->value[0] = XYZ[0];
                point->value[1] = XYZ[1];
                point->value[2] = XYZ[2];
                applyMatrix_f(point->value, XYZ_to_RGB);
                point->distance = XYZ[1];
            }
        }
    }



    /* Generate sweep */

    int image_width = 1000;
    int image_height = 481;
    float * colour_image = malloc(image_height*image_width*3*sizeof(float));
    float sweep_min_stops = -5;
    float sweep_max_stops = 12;

    /********** SWEEP **********/
    // for (int y = 0; y < image_height; ++y) {
    //     float hue = ((double)y) / (image_height - 1.0);
    //     for (int x = 0; x < image_width; ++x) {
    //         float * pix = colour_image + (y*image_width+x)*3;
    //         float exposure_stops = (((float)x)/(image_width-1.0f)) * (sweep_max_stops - sweep_min_stops) + sweep_min_stops;
    //         Util_HSVToRGB(hue*1/* .333333 */, 1.0, powf(2.0, exposure_stops), pix);
    //     }
    // }

    /********* Red XMAS EXR ********/
    float EXPOsure = 0.8;
    uint64_t file_size = 0;
    image_width = 1920;
    image_height = 1080;
    uint64_t imgdata_size = image_width * image_height * sizeof(float) * 4;
    uint8_t * file_data = Util_OpenFileToMemory(argv[1], 1000, &file_size);
    float * image_data = (float *)(file_data + (file_size - imgdata_size));

    colour_image = malloc(image_height*image_width*sizeof(float)*3);

    for (int y = 0; y < image_height; ++y)
    {
        for (int x = 0; x < image_width; ++x)
        {
            int i = y * (image_width) + x;
            int i2 = y * (image_width*4+2) + (x-236);
            int i3 = y * (image_width*4+2) + (x-236) + 1920;
            int i4 = y * (image_width*4+2) + (x-236) + 3840;
            colour_image[i*3] = image_data[i4] * EXPOsure;
            colour_image[i*3+1] = image_data[i3] * EXPOsure;
            colour_image[i*3+2] = image_data[i2] * EXPOsure;
        }
    }


/*

_ _  _ ____ ____ ____
| |\/| |__| | __ |___
| |  | |  | |__] |___

____ ____ _  _ ____ ____ ___ _ ____ _  _ 
|___ |  | |\/| |__/ |__|  |  | |  | |\ | 
|    |__| |  | |  \ |  |  |  | |__| | \| . . .

*/

    for (int p = 0; p < (image_height*image_width*3); p+=3)
    {
        float * pix = colour_image + p;
        applyMatrix_f(pix, RGB_to_XYZ);
        XYZ_to_IPT(pix, pix, 1);

        /* Do the slope contrast */
        pix[0] = IPT_curve(do_contrast(IPT_curve_inverse(pix[0]), contrast_slope, 1.0));

        /* Do saturation */
        pix[1] *= saturation_factor;
        pix[2] *= saturation_factor;

        IPT_to_XYZ(pix, pix, 1);

        /* Grab the luminance */
        float Y = compress_value(pix[1], compression_smoothness);
        applyMatrix_f(pix, XYZ_to_RGB);

        /* Clip negative channels, as footprint compression. This is a todo. */
        for (int c = 0; c < 3; ++c) if (pix[c] < 0.0) pix[c] = 0.0;

        /* Index the LUT */
        float sum = (pix[0] + pix[1] + pix[2]);
        if (sum == 0.0) sum = 1.0; /* Just to division by zero if it's a black pixel */
        float r = pix[0] / sum;
        float g = pix[1] / sum;
        int ir = r * LUT_RESOLUTION * 0.99999;
        int ig = g * LUT_RESOLUTION * 0.99999;

        ColourPathInterpolate(&paths[ir][ig], Y, pix);
    }



    uint8_t * bmp = malloc(image_height*image_width*3);
    for (int p = 0; p < (image_height*image_width*3); p += 3)
    {
        bmp[ p ] = linear_to_sRGB(colour_image[ p ]*1.00001); /* The tiny multiplier makes values able to reach RGB 255 */
        bmp[p+1] = linear_to_sRGB(colour_image[p+1]*1.00001);
        bmp[p+2] = linear_to_sRGB(colour_image[p+2]*1.00001);
    }

    Util_WriteBitmap(bmp, image_width, image_height, "result.bmp", 0);

    return 0;
}


/* Implementation of not so relevant functions... */

float compress(float x)
{
    return (x / (1.0+x));
}

float compress_value(float x, float power)
{
    if (x < 0) return x;
    return powf(compress(pow(x, power)), 1.0f/power);
}

/* I don't remember what all this "contrast" code does, but it definitely does do a sloped contrast */
#define MIDDLE_GREY 0.18
#define middle_grey (MIDDLE_GREY)
#define linear_start (MIDDLE_GREY/3.5)
double _do_power_contrast(double x, double power, double pivot){
    x /= pivot;
    if (x < 1.0) x = pow(x, power);
    else {
        x = (x-1.0) * power + 1.0;
    }
    x *= pivot;
    return x;
}double do_power_contrast(double x, double power){
    x = _do_power_contrast(x, power, linear_start);
    x /= _do_power_contrast(middle_grey, power, linear_start);
    x *= middle_grey;
    return x;
}double contrast_base(double X, double Power){
    if (X < 0.0) return 0;
    if (X < 1.0) return pow(X, Power);
    else return (X-1.0) * Power + 1.0;
}double contrast_scaled(double X, double Power, double Scale){
    return contrast_base(X * Scale, Power) / Scale;
}double do_contrast_about1(double X, double Power, double Scale){
    return contrast_scaled((X) - (contrast_scaled(1.0, Power, Scale) / Power) + (1.0 / Power), Power, Scale);
} double do_contrast(double X, double Power, double Scale){
    return do_contrast_about1(X/middle_grey, Power, Scale) * middle_grey;
}