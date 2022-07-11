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

#include <stdio.h>

#include "Matrix.h"

/* Very uzeful diagonal flip feature */
static int diagonal_flip[9] = { 0, 3, 6, 1, 4, 7, 2, 5, 8 };
/* Makes it like a function */
#define diag_flip(X) diagonal_flip[(X)]

/* Multiplies matrices A and B to outputMatrix */
void multiplyMatrices(double * A, double * B, double * outputMatrix)
{
    outputMatrix[0] = A[0] * B[0] + A[1] * B[3] + A[2] * B[6];
    outputMatrix[1] = A[0] * B[1] + A[1] * B[4] + A[2] * B[7];
    outputMatrix[2] = A[0] * B[2] + A[1] * B[5] + A[2] * B[8];
    outputMatrix[3] = A[3] * B[0] + A[4] * B[3] + A[5] * B[6];
    outputMatrix[4] = A[3] * B[1] + A[4] * B[4] + A[5] * B[7];
    outputMatrix[5] = A[3] * B[2] + A[4] * B[5] + A[5] * B[8];
    outputMatrix[6] = A[6] * B[0] + A[7] * B[3] + A[8] * B[6];
    outputMatrix[7] = A[6] * B[1] + A[7] * B[4] + A[8] * B[7];
    outputMatrix[8] = A[6] * B[2] + A[7] * B[5] + A[8] * B[8];
}

void invertMatrix(double * inputMatrix, double * outputMatrix)
{
    for (int y = 0; y < 3; ++y)
    {
        for (int x = 0; x < 3; ++x)
        {
            /* Determenant locations for 2 x 2 */
            int dX[2] = { (x + 1) % 3, (x + 2) % 3 };
            int dY[2] = { 3 * ((y + 1) % 3), 3 * ((y + 2) % 3) };

            outputMatrix[ diag_flip(y*3 + x) ] = 
            (   /* Determinant caluclation 2 x 2 */
                  inputMatrix[dY[0] + dX[0]] 
                * inputMatrix[dY[1] + dX[1]]
                - inputMatrix[dY[0] + dX[1]] 
                * inputMatrix[dY[1] + dX[0]]
            );
        }
    }

    /* Calculate whole matrix determinant */
    double determinant = 1.0 / (
          inputMatrix[0] * ( inputMatrix[8] * inputMatrix[4] - inputMatrix[7] * inputMatrix[5] )
        - inputMatrix[3] * ( inputMatrix[8] * inputMatrix[1] - inputMatrix[7] * inputMatrix[2] )
        + inputMatrix[6] * ( inputMatrix[5] * inputMatrix[1] - inputMatrix[4] * inputMatrix[2] )
    );

    /* Multiply all elements by the determinant */
    for (int i = 0; i < 9; ++i) outputMatrix[i] *= determinant;
}

void printMatrix(double * matrix)
{
    for (int i = 0; i < 9; i += 3)
        printf("[ %.6f, %.6f, %.6f ]\n", matrix[i], matrix[i+1], matrix[i+2]);
}

void printMatrix_f(float * matrix)
{
    for (int i = 0; i < 9; i += 3)
        printf("[ %.6f, %.6f, %.6f ]\n", matrix[i], matrix[i+1], matrix[i+2]);
}

/* SLOW, V is vector, M is matrix */
void applyMatrix(double * V, double * M)
{
    double V0 = V[0], V1 = V[1], V2 = V[2];
    V[0] = M[0] * V0 + M[1] * V1 + M[2] * V2;
    V[1] = M[3] * V0 + M[4] * V1 + M[5] * V2;
    V[2] = M[6] * V0 + M[7] * V1 + M[8] * V2;
}

void applyMatrix_f(float * V, double * M)
{
    double V0 = V[0], V1 = V[1], V2 = V[2];
    V[0] = M[0] * V0 + M[1] * V1 + M[2] * V2;
    V[1] = M[3] * V0 + M[4] * V1 + M[5] * V2;
    V[2] = M[6] * V0 + M[7] * V1 + M[8] * V2;
}

void applyMatrix_ff(float * V, float * M)
{
    double V0 = V[0], V1 = V[1], V2 = V[2];
    V[0] = M[0] * V0 + M[1] * V1 + M[2] * V2;
    V[1] = M[3] * V0 + M[4] * V1 + M[5] * V2;
    V[2] = M[6] * V0 + M[7] * V1 + M[8] * V2;
}

void matrixToFloat(double * In, float * Out)
{
    for (int i = 0; i < 9; ++i)
    {
        Out[i] = (float)In[i];
    }
}

static inline int is_1(double x) {
    return (x >= 0.999998 && x <= 1.000002);
}

static inline int is_0(double x) {
    return (x >= -0.000002 && x <= 0.000002);
}

int isMatrixIdentity(double * M)
{
    return is_1(M[0]) && is_0(M[1]) && is_0(M[2])
        && is_0(M[3]) && is_1(M[4]) && is_0(M[5])
        && is_0(M[6]) && is_0(M[7]) && is_1(M[8]);
}

int isMatrixJustGains(double * M)
{
    return is_0(M[1]) && is_0(M[2])
        && is_0(M[3]) && is_0(M[5])
        && is_0(M[6]) && is_0(M[7]);
}