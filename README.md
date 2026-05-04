# ImageMod

This C library contains functions for modifying images, or searching them.

All the functions work with images converted to byte streams.\
The python module is an adapter to facilitate using the library in python.

## Function descriptions:
- ### ColorReduce
    This function takes an image and turns each pixel into the color that is closest to it (in terms of difference with red green and blue values).\
    It can be used with a limited color set, or black and white.
- ### SplitColor
    This function takes an image and turns each pixel black or white depending on whether its color i above or below a specific threshold.\
    If no threshold is supplied, it will default to the average color of the image.
- ### ColorReplace
    This function replaces one specific color with another in the supplied image.
- ### FillSquareColor
    This function fills a square of specified dimensions at a specific location in the supplied image.
- ### PadImage
    This function adds padding of a specified color to the supplied image.
- ### CropImage
    This function makes a cropped copy of the supplied image.
- ### EraseLongSegments
    This function replaces lines and columns of a single color by the supplied background color.\
    The maximum line width and column height must be specified.
- ### RemoveEmptyLines
    This function makes a copy of the supplied image where lines containing a certain proportion of a specified color are removed.
- ### PixelMatch
    This function calculates the maximum percentage of pixels in a small image that match a larger image (how alike the images are).
- ### GetImagePosition
    This function finds all positions in a large image that match a smaller image.\
    The exact matching percentage can be specified.
- ### GetRelevantRectangle
    This function finds in the supplied image the top left and bottom right of a rectangle containing any pixel of a color different from the supplie background color.
- ### GetElementList
    This function finds groups of pixels of any color different from the specified background color in the supplied image.
