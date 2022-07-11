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
My basic 3x3 matrix library that I wrote when I first started coding but still use to this day
*/

#ifndef _matrix_h_
#define _matrix_h_

/* Matrix functions for 3x3, not for whole images, just individual calculations */

/* Multiply 3x3 matrices - A and B */
void multiplyMatrices(double * A, double * B, double * outputMatrix);

/* Amazing inversion function */
void invertMatrix(double * inputMatrix, double * outputMatrix);

/* Prints a matrix! */
void printMatrix(double * matrix);
void printMatrix_f(float * matrix);

/* SLOW!!! V is vector, M is matrix */
void applyMatrix(double * V, double * M);
void applyMatrix_f(float * V, double * M);
void applyMatrix_ff(float * V, float * M);

/* Convery double matrix to float (for actual use) */
void matrixToFloat(double * In, float * Out);

/* Checks if a mtrix is an identity matrix, to ~6dp */
int isMatrixIdentity(double * M);

/* Checks if a matrix is effectively a set of multipliers, like to an identity
 * matrix, zeros be in the same spots, but the 1s can be other values */
int isMatrixJustGains(double * M);

#endif