#PSX Bitmap Manipulator Notes

#### Library: libbmpm.lib

#### Example: any examples using textures

This is a custom library for manipulating texture data into a 16 bit raw A1B5G5R5 data used
by the PlayStation.

#### 

#### Functions

* setSemiTrans(), set semiTrans data for bitmaps or raw data, will set bit to 1 indicating color wanted is semi-transparent.
  * NOTE: colors are only 5 bits, not 8.
* clearSemiTrans(), clear any semi-transparent bits set to 0
* bitmapToRAW(), convert bitmap data to raw image data, does not alter raw data bits.
* swapRedBlue(), swap red and blue in image data, works for raw and bitmap data.

#### Notes
* PlayStation needs the x and y coordinates where the texture resides in vram (get from xml)
* PlayStation needs the width and height as well.
* PlayStation uses A1B5G5R5 vs bitmaps that are A1R5G5B5.
* Bitmaps have the top of the image located at the msb row and the bottom located at the lsb row.
  * aka the image is flipped over the horizontal axis
  * data must be reordered

#### Creating a texture
* Simply use gimp, and keep the following in mind:
  * Size max is 256 x 256
  * must be saved as a 16 bit bitmap


### Example

##### Coming soon
```
//soon
```
