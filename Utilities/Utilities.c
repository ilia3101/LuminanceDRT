#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>

#include "Utilities.h"
#include "../Matrix.h"


#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#define CLIP(X) MIN(1.0, MAX(0.0,X))

static float encode_sRGB(float u) {
    if(u<0)return 0;else if(u<0.0031308f)return(323.0f*u)/25; else if(u<1)return 1.055f*powf(u,1.0f/2.4f)-0.055f;else return 1.0f;
}

static double sRGB_2part_tolinear(double x) {
    if(x<0)return 0;else if(x<0.04045)return(25.0*x)/323.0; else if(x<1)return pow((x+0.055)/1.055, 2.4);else return 1.0;
}
static double decode_sRGB(uint8_t x) {
    return sRGB_2part_tolinear(((double)x) / 255.0);
}

void * Util_OpenFileToMemory(char * Path, uint64_t MaxMiB, uint64_t * SizeOut)
{
    FILE * f = fopen(Path, "r");

    uint64_t size = 0;
    uint8_t * data = NULL;

    if (f != NULL)
    {
        fseek(f, 0, SEEK_END);
        size = ftell(f);

        if (size > (MaxMiB*1024*1024))
        {
            size = 0;
        }
        else
        {
            fseek(f, 0, SEEK_SET);
            data = malloc(size+1);
            if (data != NULL) {
                fread(data, size, 1, f);
                data[size] = 0;
            } else size = 0;
        }

        fclose(f);
    }

    if (SizeOut != NULL) *SizeOut = size;
    return data;
}

void Util_CloseFileFromMemory(void * File)
{
    free(File);
}

double sRGB_to_linear(uint8_t CodeValue)
{
    return decode_sRGB(CodeValue);
}

uint8_t linear_to_sRGB(double LinearValue)
{
    return (uint8_t)(encode_sRGB(LinearValue)*255.0);
}

static float hsv_fract(float x) { return x - ((int)(x)); }
static float hsv_mix(float a, float b, float t) { return a + (b - a) * t; }
static float hsv_step(float e, float x) { return x < e ? 0.0 : 1.0; }
static float hsv_constrain(float x, float a, float b) {
    if (x < a) return a;
    if (x > b) return b;
    return x;
}
void Util_HSVToRGB(float H, float S, float V, float * RGBOut)
{
    while (H <= 0.0) H += 1.0;
    RGBOut[0] = V * hsv_mix(1.0, hsv_constrain(fabs(hsv_fract(H + 1.0) * 6.0 - 3.0) - 1.0, 0.0, 1.0), S);
    RGBOut[1] = V * hsv_mix(1.0, hsv_constrain(fabs(hsv_fract(H + 0.6666666) * 6.0 - 3.0) - 1.0, 0.0, 1.0), S);
    RGBOut[2] = V * hsv_mix(1.0, hsv_constrain(fabs(hsv_fract(H + 0.3333333) * 6.0 - 3.0) - 1.0, 0.0, 1.0), S);
}

void Util_WriteBitmap(unsigned char * data, int width, int height, char * filename, int Invert)
{
    int rowbytes = width*3+(4-(width*3%4))%4, imagesize = rowbytes*height, y;
    uint16_t header[] = {0x4D42,0,0,0,0,26,0,12,0,width,height,1,24};
    *((uint32_t *)(header+1)) = 26 + imagesize-((4-(width*3%4))%4);
    FILE * file = fopen(filename, "wb"); fwrite(header, 1, 26, file);
    if (file) {
        for (int i = 0; i < width*height; ++i) {int t = data[i*3]; data[i*3] = data[i*3+2]; data[i*3+2] = t;}
        if (!Invert)
            for (y = height-1; y >= 0; --y) fwrite(data+(y*width*3), rowbytes, 1, file);
        else
            for (y = 0; y < height; ++y) fwrite(data+(y*width*3), rowbytes, 1, file);
        fwrite(data, width*3, 1, file); fclose(file);
    }
}
