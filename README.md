# LuminanceDRT
Simple luminance based image formation algorithm with hue linearity (using IPT). Renders smooth images from linear ("scene referred") data without clipping or producing out of gamut values.
The process consists of these steps:
1. Slope contrast in IPT, as this compensates Hunt and Abney effect for free, unlike doing it on linear luminance
2. `Insert footprint compression here` (I simply clip negative channels)
3. Luminance is compressed from (0,∞) to (0,1) using a variation of the `x / (x + 1)` formula, basically the Reinhard procedure
4. Use RGB chromaticity and the compressed luminance value to index a LUT of precalculated "paths to white", which outputs the final RGB value. Most of the code is precalculating these paths with hue linearity and rounded cornders for smoothness.

Code is work in progress. Assumes rec709 while I figure out if EXR has colour space metadata.

## Usage

At least for now, you'll need to be on Linux or macOS, or figure out how to compile C on Windows yourself.

1. Compile (you'll need to have gcc/clang installed):
```
./build.sh
```
2. Run:
```
python3 Process_EXR.py --file /path/to/your.exr
```
Options:
```
usage: Process_EXR.py [-h] [--exposure EXPOSURE] [--slope SLOPE] [--smoothness SMOOTHNESS] [--saturation SATURATION] [--output OUTPUT] --file FILE

optional arguments:
  -h, --help            show this help message and exit
  --exposure EXPOSURE   Exposure correction in stops
  --slope SLOPE         Contrast slope (steepness), default is 1.7
  --smoothness SMOOTHNESS
                        How much to round the corners of the gamut volume (smoothness), from 0-1, default value is 0.4
  --saturation SATURATION
                        Saturation factor, default is 1.0
  --output OUTPUT       Specify output path/filename, ending in .bmp
  --file FILE           Path to an EXR file
```

## Issues

Assumes all input EXRs are rec709, and outputs with rec709 primaries.

Negative luminance values caused by 3x3 matrices may cause black spots in the image. 

## Examples

Does a decent job with bright colours. All were processed with slope 1.7

Red Xmas
![red_xmas](https://user-images.githubusercontent.com/23642861/178275374-bdfafd62-c1c7-4017-85f8-8b9ed73da43e.jpg)

Lightsabers
![ligthsaber](https://user-images.githubusercontent.com/23642861/178275918-9d4168d1-6df5-4afe-aa3a-48f5a65074cd.jpg)

Skin tones
![skintone](https://user-images.githubusercontent.com/23642861/178276754-b6c0e136-8d0b-464e-b078-97b59ea6b04a.jpg)

Blue bar
![bluebar](https://user-images.githubusercontent.com/23642861/178523007-ddb9efc2-390a-4c8e-8e32-b7ff9e031fed.jpg)

Test data from https://github.com/sobotka/Testing_Imagery
