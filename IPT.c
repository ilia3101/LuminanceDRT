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

/* Simple and inefficient implementation of IPT */

#include "IPT.h"

#include <math.h>

#include "Matrix.h"

static double XYZ_to_LMS_D65_HPE[9] = {
    0.4002, 0.7075, -0.0807,
    -0.2280, 1.1500, 0.0612,
    0.0000, 0.0000, 0.9184
};

static double opponency_matrix[9] = {
    0.4000, 0.4000, 0.2000,
    4.4550, -4.8510, 0.3960,
    0.8056, 0.3572, -1.1628
};

#define IPT_POWER 0.43f

static inline float nonlinearity(float x)
{
    if (x < 0.0) return - powf(-x, IPT_POWER);
    else return powf(x, IPT_POWER);
}

static inline float nonlinearity_inverse(float x)
{
    if (x < 0.0) return - powf(-x, 1.0f / IPT_POWER);
    else return powf(x, 1.0f / IPT_POWER);
}

void XYZ_to_IPT(float * In, float * Out, uint64_t N)
{
    for (uint64_t i = 0; i < N; ++i)
    {
        float values[3] = {In[0], In[1], In[2]};
        applyMatrix_f(values, XYZ_to_LMS_D65_HPE);

        for (int j = 0; j < 3; ++j)
            values[j] = nonlinearity(values[j]);

        applyMatrix_f(values, opponency_matrix);

        for (int j = 0; j < 3; ++j) Out[j] = values[j];

        In += 3;
        Out += 3;
    }
}

void IPT_to_XYZ(float * In, float * Out, uint64_t N)
{
    double opponent_to_LMS[9];
    invertMatrix(opponency_matrix, opponent_to_LMS);

    double LMS_to_XYZ[9];
    invertMatrix(XYZ_to_LMS_D65_HPE, LMS_to_XYZ);

    for (uint64_t i = 0; i < N; ++i)
    {
        float values[3] = {In[0], In[1], In[2]};
        applyMatrix_f(values, opponent_to_LMS);

        for (int j = 0; j < 3; ++j)
            values[j] = nonlinearity_inverse(values[j]);

        applyMatrix_f(values, LMS_to_XYZ);

        for (int j = 0; j < 3; ++j) Out[j] = values[j];

        In += 3;
        Out += 3;
    }
}

float IPT_curve(float X)
{
    return nonlinearity(X);
}

float IPT_curve_inverse(float X)
{
    return nonlinearity_inverse(X);
}