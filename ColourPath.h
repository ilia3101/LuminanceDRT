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

/* A basic data structure to represent colour paths. Really just an array. */

#ifndef _ColourPath_h_
#define _ColourPath_h_

#define ColourPath_MAX_NUM_POINTS 100

typedef struct {
    float distance;
    float value[3]; /* Colour value in any model. */
} ColourPathPoint_t;

typedef struct {
    int num_points;
    ColourPathPoint_t points[ColourPath_MAX_NUM_POINTS];
} ColourPath_t;

void init_ColourPath(ColourPath_t * Path);
void ColourPathAddPoint(ColourPath_t * Path, ColourPathPoint_t Point);
void ColourPathAddPointByValues(ColourPath_t * Path, float A, float B, float C);
void ColourPathAddPointByValue(ColourPath_t * Path, float * Value);
void ColourPathCalculateDistance(ColourPath_t * Path, float WeightA, float WeightB, float WeightC);
float ColourPathGetDistanceOfPoint(ColourPath_t * Path, int PointIndex);
float ColourPathGetLength(ColourPath_t * Path);
void ColourPathInterpolate(ColourPath_t * Path, float DistanceValue, float * Out);
int ColourPathGetNumPoints(ColourPath_t * Path);
ColourPathPoint_t ColourPathGetPoint(ColourPath_t * Path, int PointIndex);
void ColourPathNormaliseDistance(ColourPath_t * Path, float NormaliseValue);

#endif