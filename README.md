# LuminanceDRT
Simple luminance based image formation algorithm with hue linearity and smooth tonality. Renders good looking images from linear ("scene referred") data without clipping or producing out of gamut values.

Does not do footprint compression.

Code is work in progress. Assumes rec709 while I figure out if EXR has colour space metadata.


## Examples

Does a decent job with bright colours. All were processed with slope 1.7

Red Xmas
![red_xmas](https://user-images.githubusercontent.com/23642861/178275374-bdfafd62-c1c7-4017-85f8-8b9ed73da43e.jpg)

Lightsabers
![ligthsaber](https://user-images.githubusercontent.com/23642861/178275918-9d4168d1-6df5-4afe-aa3a-48f5a65074cd.jpg)

Skin tones
![skintone](https://user-images.githubusercontent.com/23642861/178276754-b6c0e136-8d0b-464e-b078-97b59ea6b04a.jpg)

Test data from https://github.com/sobotka/Testing_Imagery
