import OpenEXR
import numpy
import Imath
import sys
import os
import array
import argparse
import math

exposure = 0.0
slope = 1.7
smoothness = 0.4
saturation = 1.0

# Construct the argument parser
ap = argparse.ArgumentParser()
# Add the arguments to the parser
ap.add_argument("--exposure", type=float, required=False,
   help="Exposure correction in stops")
ap.add_argument("--slope", type=float, required=False,
   help="Contrast slope (steepness), default is " + str(slope))
ap.add_argument("--smoothness", type=float, required=False,
   help="How much to round the corners of the gamut volume (smoothness), from 0-1, default value is " + str(smoothness))
ap.add_argument("--saturation", type=float, required=False,
   help="Saturation factor, default is " + str(saturation))
ap.add_argument("--file", type=str, required=True,
   help="Path to an EXR file")
args = vars(ap.parse_args())
# Calculate the sum
# print("Sum is {}".format(int(args['foperand']) + int(args['soperand'])))
print(args)
# exit()

input_file_path = args['file']
if (args['saturation']): saturation = args['saturation']
if (args['slope']): slope = args['slope']
if (args['smoothness']): smoothness = args['smoothness']
if (args['exposure']): exposure = args['exposure']

print("smoothness = " + str(smoothness))
print("slope = " + str(slope))
print("exposure = " + str(exposure))
print("saturation = " + str(saturation))

pt = Imath.PixelType(Imath.PixelType.FLOAT)

exr_file = OpenEXR.InputFile(input_file_path)

red = array.array('f', exr_file.channel('R', pt))
green = array.array('f', exr_file.channel('G', pt))
blue = array.array('f', exr_file.channel('B', pt))

#just making an array of the same size as all channels together cauz i dont know how to use array module or python at all
result_buffer = array.array('f', red + blue + green) 

print("length = " + str(len(result_buffer)))

dw = exr_file.header()['dataWindow']
image_width = dw.max.x - dw.min.x + 1
image_height = dw.max.y - dw.min.y + 1

print("width = " + str(image_width))
print("height = " + str(image_height))

print(type(result_buffer).__name__)

for i in range (0, image_height*image_width):
    result_buffer[i * 3] = red[i]
    result_buffer[i*3+1] = green[i]
    result_buffer[i*3+2] = blue[i]

# binary reference to the float data in RGB
as_bytes = bytes(result_buffer)
# save it to a file
with open("binary_data", "wb") as binary_file:
    # Write bytes to file
    binary_file.write(as_bytes)

os.system("./process_data binary_data " + str(image_width)
                                        + " " + str(image_height)
                                        + " " + str(saturation)
                                        + " " + str(slope)
                                        + " " + str(smoothness)
                                        + " " + str(exposure))

os.remove("binary_data")