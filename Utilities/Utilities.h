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

#ifndef _Utilities_h_
#define _Utilities_h_

#include <stdint.h>

/* Reads a file to memory, will fail if size over MaxMiB. SizeOut in bytes, can
 * be NULL if u dont need it. Always puts a zero byte at the end for string purposes. */
void * Util_OpenFileToMemory(char * Path, uint64_t MaxMiB, uint64_t * SizeOut);
/* Frees a file opened with previous function */
void Util_CloseFileFromMemory(void * File);

/* sRGB transfer function */
double sRGB_to_linear(uint8_t CodeValue);
uint8_t linear_to_sRGB(double LinearValue);

/* HSV to RGB */
void Util_HSVToRGB(float H, float S, float V, float * RGBOut);

/* Writes a bitmap from an rgb int8 image */
void Util_WriteBitmap(unsigned char * data, int width, int height, char * filename, int Invert);

#endif