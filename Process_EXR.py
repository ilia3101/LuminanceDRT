import OpenEXR
import numpy
import Imath
import sys
import os
import array

pt = Imath.PixelType(Imath.PixelType.FLOAT)

exr_file = OpenEXR.InputFile(sys.argv[1])


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

os.system("./process_data binary_data " + str(image_width) + " " + str(image_height))
print("test")
