#include <math.h>

#include "ColourPath.h"

void init_ColourPath(ColourPath_t * Path)
{
    Path->num_points = 0;
}

void ColourPathAddPoint(ColourPath_t * Path, ColourPathPoint_t Point)
{
    Path->points[Path->num_points++] = Point;
}

void ColourPathAddPointByValues(ColourPath_t * Path, float A, float B, float C)
{
    ColourPathAddPoint(Path, (ColourPathPoint_t){.distance = 0, .value = {A,B,C}});
}

void ColourPathAddPointByValue(ColourPath_t * Path, float * Value)
{
    ColourPathAddPoint(Path, (ColourPathPoint_t){.distance = 0, .value = {Value[0],Value[1],Value[2]}});
}

void ColourPathCalculateDistance(ColourPath_t * Path, float WeightA, float WeightB, float WeightC)
{
    Path->points[0].distance = 0.0f;
    for (int p = 1; p < Path->num_points; ++p) {
        float * va = Path->points[p].value;
        float * vb = Path->points[p-1].value;
        float d0 = (va[0] - vb[0]) * WeightA;
        float d1 = (va[1] - vb[1]) * WeightB;
        float d2 = (va[2] - vb[2]) * WeightC;
        Path->points[p].distance = Path->points[p-1].distance + sqrt(d0*d0 + d1*d1 + d2*d2);
    }
}

float ColourPathGetDistanceOfPoint(ColourPath_t * Path, int PointIndex)
{
    return Path->points[PointIndex].distance;
}

float ColourPathGetLength(ColourPath_t * Path)
{
    return ColourPathGetDistanceOfPoint(Path, Path->num_points-1);
}

void ColourPathInterpolate(ColourPath_t * Path, float DistanceValue, float * Out)
{
    float value_buf[3];
    float * value = value_buf;

    if (DistanceValue >= ColourPathGetLength(Path))
    {
        value = Path->points[Path->num_points-1].value;
    }
    else if (DistanceValue <= 0.0)
    {
        value = Path->points[0].value;
    }
    else /* We must interpolate */
    {
        int base_point = 0;
        while (!(Path->points[base_point].distance <= DistanceValue && Path->points[base_point+1].distance >= DistanceValue)) ++base_point;
        float fac = (DistanceValue - Path->points[base_point].distance) / (Path->points[base_point+1].distance - Path->points[base_point].distance);
        for (int i = 0; i < 3; ++i)
            value[i] = Path->points[base_point].value[i] * (1.0f-fac) + Path->points[base_point+1].value[i] * fac;
    }

    for (int i = 0; i < 3; ++i)
        Out[i] = value[i];
}

int ColourPathGetNumPoints(ColourPath_t * Path)
{
    return Path->num_points;
}

ColourPathPoint_t ColourPathGetPoint(ColourPath_t * Path, int PointIndex)
{
    return Path->points[PointIndex];
}

void ColourPathNormaliseDistance(ColourPath_t * Path, float NormaliseValue)
{
    float multiplier = NormaliseValue / ColourPathGetLength(Path);
    for (int p = 0; p < Path->num_points; ++p)
        Path->points[p].distance *= multiplier;
}