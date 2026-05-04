#include "main.h"
#include <stdlib.h>
//constant declarations
static uint8_t trueColor[8][3] = {{0, 0, 0}, {255, 0, 0}, {0, 255, 0}, {0, 0, 255}, {255, 255, 0}, {255, 0, 255}, {0, 255, 255}, {255, 255, 255}};
static uint8_t blackWhiteColor[8][3] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {255, 255, 255}, {255, 255, 255}, {255, 255, 255}, {255, 255, 255}};

void ColorReduce(uint8_t *pixels, int pixelCount, int hasAlpha, int blackWhite){
	//this function takes the given pixel array, and converts each pixel into a specific color it is closest to
	int pxShift = (hasAlpha)? 4 : 3;
    uint8_t red, green, blue, maxRed, maxGreen, maxBlue;
    uint16_t minDiff, diff;
    int colorId;
    //the color array is dependent of whether this is black and white or not
    uint8_t (*colorArr)[3] = (blackWhite)? blackWhiteColor : trueColor;
	//go through each pixel, and find which color is the closest
	for (int i = 0; i < pixelCount; i++){
		red = pixels[0];
		green = pixels[1];
		blue = pixels[2];
		//get all the differences
		maxRed = 255 - red;
		maxGreen = 255 - green;
		maxBlue = 255 - blue;
		//we look for the minimum difference between each color channel of the pixel and the reference colors
		//the first comparison is with black
		colorId = 0;
		minDiff = red + green + blue;
		//the next comparison is with red
		diff = maxRed + green + blue;
		if (diff < minDiff){
			minDiff = diff;
			colorId = 1;
		}
		//the next comparison is with green
		diff = red + maxGreen + blue;
		if (diff < minDiff){
			minDiff = diff;
			colorId = 2;
		}
		//the next comparison is with blue
		diff = red + green + maxBlue;
		if (diff < minDiff){
			minDiff = diff;
			colorId = 3;
		}
		//the next comparison is with yellow
		diff = maxRed + maxGreen + blue;
		if (diff < minDiff){
			minDiff = diff;
			colorId = 4;
		}
		//the next comparison is with purple
		diff = maxRed + green + maxBlue;
		if (diff < minDiff){
			minDiff = diff;
			colorId = 5;
		}
		//the next comparison is with cyan
		diff = red + maxGreen + maxBlue;
		if (diff < minDiff){
			minDiff = diff;
			colorId = 6;
		}
		//the last comparison is with white
		diff = maxRed + maxGreen + maxBlue;
		if (diff < minDiff){
			minDiff = diff;
			colorId = 7;
		}
		//overwrite the pixel colors with the closest true color
		pixels[0] = colorArr[colorId][0];
		pixels[1] = colorArr[colorId][1];
		pixels[2] = colorArr[colorId][2];
		//get to the next pixel
		pixels += pxShift;
	}
}

void SplitColor(uint8_t *pixels, int pixelCount, int hasAlpha, uint8_t *baseColor, int *threshold, int colorCount, int isBackground){
	//this function takes the given pixel array, calculate the difference of each pixel to the base color, then make the pixle black or white depending on wheteher they are above or below the average difference
	//we need an array to store the pixel difference and a variable to store the total difference
	uint16_t *pixDiff = (uint16_t *) malloc(sizeof(uint16_t) * pixelCount), *thrDiff = (uint16_t *) malloc(sizeof(uint16_t) * colorCount);
	int *colIndex = (int *) malloc(sizeof(int) * colorCount), *colIndexPx = (int *) malloc(sizeof(int) * pixelCount);
	uint64_t totalDiff = 0;
	uint16_t diff;
	int pxDepth = (hasAlpha)? 4 : 3;
	uint8_t *pxPt = pixels, *colorPt;
	uint8_t repCol, otherCol;
	int i, j;
	//calculate the color index for each color
	for (i = 0; i < colorCount; i++){
		colIndex[i] = i * 3;
	}
	//go through each pixel, get the minimum difference to all the base colors, store it, and add it to the total
	for (i = 0; i < pixelCount; i++){
		pixDiff[i] = 765;
		for (j = 0; j < colorCount; j++){
			diff = 0;
			//get the difference for red
			colorPt = baseColor + colIndex[j];
			if (pxPt[0] < *colorPt){
				diff = *colorPt - pxPt[0];
			}
			else {
				diff = pxPt[0] - *colorPt;
			}
			//get the difference for green
			colorPt++;
			if (pxPt[1] < *colorPt){
				diff += (*colorPt - pxPt[1]);
			}
			else {
				diff += (pxPt[1] - *colorPt);
			}
			//get the difference for blue
			colorPt++;
			if (pxPt[2] < *colorPt){
				diff += (*colorPt - pxPt[2]);
			}
			else {
				diff += (pxPt[2] - *colorPt);
			}
			//check if the difference is less than the current pixel difference, and store it if it is, as well as the index of the current color
			if (diff < pixDiff[i]){
				pixDiff[i] = diff;
				colIndexPx[i] = j;
			}
		}
		//add to the total difference
		totalDiff += pixDiff[i];
		//increment pixel pointer
		pxPt += pxDepth;
	}
	//the threshold difference depends on the threshold argument
	diff = (uint16_t) (totalDiff / (uint64_t)pixelCount); //average difference
	for (i = 0; i < colorCount; i++){
		if ((!threshold) || (threshold[i] < 0) || (threshold[i] > 765)){
			//if the threshold for this color does not exist, or is not between the correct values, we take the average difference as the threshold
			thrDiff[i] = diff;
		}
		else {
			//otherwise we use the supplied threshold
			thrDiff[i] = (uint16_t) threshold[i];
		}
	}
	//go through the pixel array again, and set the pixel to black or white depending on whether its difference is greater or lesser than the threshold and the color is background
	repCol = (isBackground)? 255 : 0; //if the colors are background the replacement color is white otherwise it is black
	otherCol = 255 - repCol; //the other color is the inverse
	for (i = 0; i < pixelCount; i++){
		//check the difference and assign the appropriate color
		pixels[0] = pixels[1] = pixels[2] = (pixDiff[i] < thrDiff[colIndexPx[i]])? repCol : otherCol;
		//increment pixel pointer
		pixels += pxDepth;
	}
	//clean up
	free((void *) pixDiff);
	free((void *) thrDiff);
	free((void *) colIndex);
	free((void *) colIndexPx);
}

void ColorReplace(uint8_t *pixels, int pixelCount, int hasAlpha, int ignoreAlpha, uint8_t *oldColor, uint8_t *newColor){
	//this function goes through every pixel in the supplied image and replace the supplied olf color with new one
	int pxDepth = (hasAlpha)? 4 : 3, scanLen = (ignoreAlpha)? 3 : pxDepth;
	int i, j;
	int isOld;
	int pos = 0;
	for ( i = 0; i < pixelCount; i++){
		//first check that the pixel is of the right color
		isOld = 1;
		for (j = 0; j < scanLen; j++){
			if(pixels[pos + j] != oldColor[j]){
				isOld = 0;
				break;
			}
		}
		//if the pixel is of the old oclor, we replace it with the new
		if (isOld){
			for (j = 0; j < scanLen; j++){
				pixels[pos + j] = newColor[j];
			}
		}
		pos += pxDepth;
	}
}

int FillSquareColor(uint8_t *pixels, int width, int height, int hasAlpha, int sqx, int sqy, int sqw, int sqh, uint8_t *color){
	//this function fills a square of the specified dimensions with the required color on the provided image
	int pxLen = (hasAlpha)? 4 : 3;
	int lineLen = (width * pxLen);
	uint8_t colorF[pxLen];
	int i, j, k;
	int posPx, posStart;
	//exit if the square location puts it outside the image
	int sqr = sqx + sqw, sqb = sqy + sqh;
	if ((sqx >= width) || (sqr < 0) || (sqy >= height) || (sqb < 0)) return 0; //
	//adjust the dimensions of the square if they don't fall exactly inside the image
	if (sqx < 0){
		sqw += sqx;
		sqx = 0;
	}
	if (sqr > width){
		sqw = width - sqx;
	}
	if (sqy < 0){
		sqh += sqy;
		sqy = 0;
	}
	if (sqb > height){
		sqh = height - sqy;
	}
	//initialize the color array
	colorF[0] = color[0];
	colorF[1] = color[1];
	colorF[2] = color[2];
	if (hasAlpha) colorF[3] = 255;
	//if the square fits, color it
	posPx = posStart = (sqy * lineLen) + (sqx * pxLen);
	for (i = 0; i < sqh; i++){
		for (j = 0; j < sqw; j++){
			for (k = 0; k < pxLen; k++){
				pixels[posPx] = colorF[k];
				posPx++;
			}
		}
		posStart += lineLen;
		posPx = posStart;
	}
	return 1;
}

void PadImage(uint8_t *pImgPx, uint8_t *imgPx, int width, int height, int hasAlpha, int pad, uint8_t *paddingColor){
	//this function padds the provided image and pads it by the requested value with the requested color
	int pxDepth = (hasAlpha)? 4 : 3;
	int pWidth = width + (pad * 2), pHeight = height + (pad * 2), pPxCount = pWidth * pHeight * pxDepth;
	int i, pos = 0, posI = 0, top = pad, bottom = height + pad, left = pad, right = width + pad, row = 0, col = 0;
	while (posI < pPxCount){
		if ((row < top) || (row >= bottom) || (col < left) || (col >= right)){
			//if the pixel is in the padding area, we copy the padding color
			for (i = 0; i < pxDepth; i++){
				pImgPx[posI + i] = paddingColor[i];
			}
		}
		else {
			//otherwise we copy the image
			for (i = 0; i < pxDepth; i++){
				pImgPx[posI + i] = imgPx[pos + i];
			}
			pos += pxDepth;
		}
		posI += pxDepth;
		col += 1;
		if (col >= pWidth){
			col = 0;
			row += 1;
		}
	}
}

void CropImage(uint8_t *imgPx, uint8_t *imgCropPx, uint32_t *rectDim, int width, int hasAlpha){
	//this function creates a copy of the defined rectangle in the image and copies it to the cropped image container
    int i, j, k;
    int top = (int) rectDim[0], bottom = (int) rectDim[1],  left = (int) rectDim[2],  right = (int) rectDim[3];
    int pxLen = (hasAlpha)? 4 : 3;
    int lineJump = ((width - right - 1 + left) * pxLen);
    int pos = (top * width * pxLen) + (left * pxLen), posC = 0;
    for (i = top; i <= bottom; i++){
		for (j = left; j <= right; j++){
			for (k = 0; k < pxLen; k++){
				imgCropPx[posC++] = imgPx[pos++];
			}
		}
		//at the end of each line we must jump to the beginning of the next line in the image
		pos += lineJump;
    }
}

void EraseLongSegments(uint8_t *imgPx, int width, int height, int hasAlpha, int maxWidth, int maxHeight, uint8_t *backgroundColor){
	//this function looks for all horizontal segments wider than the mex width and vertical segments higher than the max height and erases them
	int pxLen = (hasAlpha)? 4 : 3;
	int imgSize = width * height, lineLen = (width * pxLen);
	uint8_t *relArr = (uint8_t *) malloc(imgSize);
	int *segHArr = (int *) malloc(sizeof(int) * imgSize), *segVArr = (int *) malloc(sizeof(int) * imgSize);
	int segHCount, segVCount;
	int segStart, segLen;
	int pos, posS;
	int i, j, k;
	//build the relevant pixel array
	pos = 0;
	for (i = 0; i < imgSize; i++){
		relArr[i] = 0;
		for (j = 0; j < 3; j++){
			if (imgPx[pos + j] != backgroundColor[j]){
				relArr[i] = 1;
				break;
			}
		}
		pos += pxLen;
	}
	//go through the array and find all horizontal segments greater than the maximum width
	segHCount = 0;
	pos = posS = segStart = 0;
	for (i = 0; i < height; i++){
		segLen = 0;
		for (j = 0; j < width; j++){
			if (relArr[pos]){
				//if this is a relevant pixel, add it to the segment
				if (!segLen){
					//if there is no started segment, we start it here
					segStart = pos;
				}
				segLen++;
			}
			else if (segLen){
				//if this is not a relevant pixel, and there is a segment started
				if (segLen > maxWidth){
					//if the length of the segment is greater than the maximum width add it to the segment array
					segHArr[posS++] = segStart;
					segHArr[posS++] = segLen;
					segHCount++;
				}
				//in any case reset the segment length to 0
				segLen = 0;
			}
			pos++;
		}
		//at the end of the line, check if there is a segment started and add it to the array if it fits
		if (segLen > maxWidth){
			segHArr[posS++] = segStart;
			segHArr[posS++] = segLen;
			segHCount++;
		}
	}
	//go through the array and find all vertical segments greater than the maximum height
	segVCount = 0;
	pos = posS = segStart = 0;
	for (i = 0; i < width; i++){
		segLen = 0;
		pos = i;
		for (j = 0; j < height; j++){
			if (relArr[pos]){
				//if this is a relevant pixel, add it to the segment
				if (!segLen){
					//if there is no started segment, we start it here
					segStart = pos;
				}
				segLen++;
			}
			else if (segLen){
				//if this is not a relevant pixel, and there is a segment started
				if (segLen > maxHeight){
					//if the length of the segment is greater than the maximum height add it to the segment array
					segVArr[posS++] = segStart;
					segVArr[posS++] = segLen;
					segVCount++;
				}
				//in any case reset the segment length to 0
				segLen = 0;
			}
			pos += width;
		}
		//at the end of the column, check if there is a segment started and add it to the array if it fits
		if (segLen > maxHeight){
			segVArr[posS++] = segStart;
			segVArr[posS++] = segLen;
			segVCount++;
		}
	}
	//erase horizontal segments (replace with background color)
	posS = 0;
	for (i = 0; i < segHCount; i++){
		segStart = pos = segHArr[posS++] * pxLen;
		segLen = segHArr[posS++];
		for (j = 0; j < segLen; j++){
			for (k = 0; k < pxLen; k++){
				imgPx[pos + k] = backgroundColor[k];
			}
			pos += pxLen;
		}
	}
	//erase vertical segments (replace with background color)
	posS = 0;
	for (i = 0; i < segVCount; i++){
		segStart = pos = segVArr[posS++] * pxLen;
		segLen = segVArr[posS++];
		for (j = 0; j < segLen; j++){
			for (k = 0; k < pxLen; k++){
				imgPx[pos + k] = backgroundColor[k];
			}
			pos += lineLen;
		}
	}
	//cleanup
	free((void *) relArr);
	free((void *) segHArr);
	free((void *) segVArr);
}

int RemoveEmptyLines(uint8_t *imgPx, uint8_t *imgRPx, int width, int height, int hasAlpha, int maxLines, uint8_t *backgroundColor){
	//this function removes the lines in the provided image that are empty (have only pixels of the background color) after the specified maximum allowed
	int pxDepth = (hasAlpha)? 4 : 3;
    int lineLen = width * pxDepth;
    int pos, posL, posR;
    int i, j, k;
    int emptyLines = 0, lineCount = 0, isFull;
	//go through each line to check if the line is empty
	posL = posR = 0;
	for (i = 0; i < height; i++){
		pos = posL;
		isFull = 0;
		for (j = 0; j < width; j++){
			for (k = 0; k < 3; k++){
                if (imgPx[pos + k] != backgroundColor[k]){
					//if the pixel differs from the background color in any color channel, the line is not empty, and we move on
					isFull = 1;
					break;
                }
			}
			//if the pixel is not empty, the line is not empty, move on
			if (isFull)
				break;
			pos += pxDepth;
		}
		//if the line is full, or it is not but the empty lines counter is below the required threshold, copy the image to the buffer
		if (isFull || (emptyLines < maxLines)){
			for (j = 0; j < lineLen; j++){
				imgRPx[posR++] = imgPx[posL++];
			}
			lineCount += 1;
		}
		else {
			//otherwise we simply move the starting position to the next line
			posL += lineLen;
		}
		//if the line is not full add one to the empty line ocunter otherwise set it to 0
		emptyLines = (isFull)? 0 : (emptyLines + 1);
	}
	//return the line count
	return lineCount;
}

float PixelMatch(uint8_t *smlImgPx, uint8_t *bigImgPx, int widthS, int heightS, int hasAlphaS, int widthB, int heightB, int hasAlphaB, int ignoreAlpha){
	//this function checks the small and big pictures and returns the percentage of pixels that match for the best match
	//check that the small image is smaller in width and height ahn the big image
	if ((widthS > widthB) || (heightS > heightB))
		return 0;
	//build the pixel grids
	int i, j;
	int pxCountS = widthS * heightS, pxCountB = widthB * heightB;
	int pxCount[] = {pxCountS, pxCountB}, hasAlpha[] = {hasAlphaS, hasAlphaB}, pxDepth[] = {(hasAlphaS)? 4 : 3, (hasAlphaB)? 4 : 3};
	uint8_t *imgPx[] = {smlImgPx, bigImgPx};
	uint32_t *pxGridS = (uint32_t *) malloc(sizeof(uint32_t) * pxCountS), *pxGridB = (uint32_t *) malloc(sizeof(uint32_t) * pxCountB);
	uint32_t *pxGrid[] = {pxGridS, pxGridB};
	for (i = 0; i < 2; i++){
		if (hasAlpha[i] && !ignoreAlpha){
			for (j = 0; j < pxCount[i]; j++){
				pxGrid[i][j] = (((uint32_t) imgPx[i][0]) << 24) | (((uint32_t) imgPx[i][1]) << 16) | (((uint32_t) imgPx[i][2]) << 8) | ((uint32_t) imgPx[i][3]);
				imgPx[i] += pxDepth[i];
			}
		}
		else {
			for (j = 0; j < pxCount[i]; j++){
				pxGrid[i][j] = (((uint32_t) imgPx[i][0]) << 24) | (((uint32_t) imgPx[i][1]) << 16) | (((uint32_t) imgPx[i][2]) << 8) | 0xFF;
				imgPx[i] += pxDepth[i];
			}
		}
	}
	//get the number of possible position that the small image can fir in the large one
	int maxPosH = (widthB - widthS + 1), posCount = maxPosH * (heightB - heightS + 1);
	int posB = 0, colB = 0, pos = 0, posS, colS;
	int matchPx, matchPxAll = 0;
	int betterMatch;
	for (i = 0; i < posCount; i++){
		posS = pos;
		j = colS = 0;
		matchPx = pxCountS;
		betterMatch = 1;
		while (j < pxCountS){
			matchPx -= (pxGridB[posS + colS] != pxGridS[j]);
			if (matchPx < matchPxAll){
				betterMatch = 0;
				break;
			}
			j++;
			colS++;
			if (colS >= widthS){
				//if the pixel column is greater than the width of the small image, it is on the next row, and we must increment the position of the big picture by a row
				posS += widthB;
				colS = 0;
			}
		}
		//check if it's a better match and assign it if it is
		if (betterMatch) matchPxAll = matchPx;
		//change the match position
		colB++;
		if (colB >= maxPosH){
			//reset the column and change the position if the small image position column is above the maximum possible horizontal positions
			colB = 0;
			posB += widthB;
			pos = posB;
		}
		else {
			pos++;
		}
	}
	//cleanup and return the match value
	free((void *) pxGridS);
	free((void *) pxGridB);
	return ((float) matchPxAll / (float) pxCountS);
}

int GetImagePosition(uint8_t *smlImgPx, uint8_t *bigImgPx, int *matchData, int widthS, int heightS, int hasAlphaS, int widthB, int heightB, int hasAlphaB, int ignoreAlpha, float precision, int bestMatch, int merge){
	//this function tries to find the small image in the big image with the given precision (percentage of matching pixels)
	int i, j, k, l, m, maxPosH, posCount, posB, colB, lineB, pos, posS, posM, lenT, lenC, movTC, movTL, posT, posTL, jumpPS, jumpPB, movPS, movPB, lenP, movOS, movOB, lenO, lenR, posAS, posAB, posCS, posCB;
	int matchPx, matchPxAll, matchPxMin, matchCount, isMatch;
	int pxCountS, pxCountB, pxDepthS, pxDepthB, useAlphaS, useAlphaB;
	int *widthT, *heightT, *widthTC, *heightTC, widthLenT, heightLenT;
	int colorCount, colorFound, colorPixelCount[20], colorPixelCountC[21], changeCount, changeCountH, changeCountBuf, *changeArr;
	int *mergeMatchData, mergeMatchCount;
	float adjPrec;
	uint32_t colorArr[30], pixel;
	uint8_t *pxGridS, *pxGridB, pxPrev;
	//check that the small image is smaller in width and height than the big image
	if ((widthS > widthB) || (heightS > heightB))
		return 0;
	//build the pixel grids
	pxCountS = widthS * heightS, pxCountB = widthB * heightB;
	pxDepthS = (hasAlphaS)? 4 : 3;
	pxDepthB = (hasAlphaB)? 4 : 3;
	useAlphaS = (hasAlphaS && !ignoreAlpha);
	useAlphaB = (hasAlphaB && !ignoreAlpha);
	pxGridS = (uint8_t *) malloc(sizeof(uint8_t) * pxCountS);
	pxGridB = (uint8_t *) malloc(sizeof(uint8_t) * pxCountB);
	//first go though the small image and make a list of colors (20 at most)
	colorCount = 1;
	colorArr[0] = (((uint32_t) smlImgPx[0]) << 24) | (((uint32_t) smlImgPx[1]) << 16) | (((uint32_t) smlImgPx[2]) << 8) | ((useAlphaS)? ((uint32_t) smlImgPx[3]) : 0xFF);
	colorPixelCount[0] = 1;
	pxGridS[0] = 0;
	smlImgPx += pxDepthS;
	for (i = 1; i < pxCountS; i++){
		pixel = (((uint32_t) smlImgPx[0]) << 24) | (((uint32_t) smlImgPx[1]) << 16) | (((uint32_t) smlImgPx[2]) << 8) | ((useAlphaS)? ((uint32_t) smlImgPx[3]) : 0xFF);
		//check that the current pixel has a known color
		colorFound = 0;
		for (j = 0; j < colorCount; j++){
			if (pixel == colorArr[j]){
				//if the pixel is a known color increment the color pixel counter for that color, and put it in the pixel grid
				colorFound = 1;
				colorPixelCount[j]++;
				pxGridS[i] = j;
				break;
			}
		}
		//if the color is not yet known, add it to the color array if there is space remaining
		if (!colorFound){
			if (colorCount < 20){
				colorArr[colorCount] = pixel;
				colorPixelCount[colorCount] = 1;
				pxGridS[i] = colorCount;
				colorCount++;
			}
			else {
				pxGridS[i] = 20;
			}
		}
		smlImgPx += pxDepthS;
	}
	//now that the color array is done, create the pixel grid for the big image
	for (i = 0; i < pxCountB; i++){
		pixel = (((uint32_t) bigImgPx[0]) << 24) | (((uint32_t) bigImgPx[1]) << 16) | (((uint32_t) bigImgPx[2]) << 8) | ((useAlphaB)? ((uint32_t) bigImgPx[3]) : 0xFF);
		//check that the current pixel has a known color
		colorFound = 0;
		for (j = 0; j < colorCount; j++){
			if (pixel == colorArr[j]){
				//if the pixel is a known color, put it in the big pixel grid
				colorFound = 1;
				pxGridB[i] = j;
				break;
			}
		}
		//if the color was not found, put a -1 in the pixel grid
		if (!colorFound) pxGridB[i] = 20;
		bigImgPx += pxDepthB;
	}
	//calculate the scanning direction and starting position with the greatest distinctiveness, to speed up the image matching process
	//start with the horizontal scanning
	posS = 0;
	changeArr = (int *) malloc(sizeof(int) * heightS);
	for (i = 0; i < heightS; i++){
		//for each line of the small image, count up how often the color changes
		pos = posS;
		changeArr[i] = 0;
		pxPrev = pxGridS[pos];
		pos++;
		for (j = 1; j < widthS; j++){
			changeArr[i] += (pxPrev != pxGridS[pos]);
			pxPrev = pxGridS[pos];
			pos++;
		}
		posS += widthS;
	}
	//after all the color changes have been counted, try to find the starting area (defined as one quarter of the height of the small image) with the greates amount of change
	lenC = (heightS / 4) + 1;
	lenT = heightS - lenC + 1;
	changeCount = 0;
	for (i = 0; i < lenT; i++){
		//start to count changes on each line
		changeCountBuf = 0;
		for (j = 0; j < lenC; j++){
			changeCountBuf += changeArr[i + j];
		}
		if (changeCountBuf > changeCount){
			changeCount = changeCountBuf;
			posS = i;
		}
	}
	changeCountH = changeCount;
	posB = posS;
	free((void *) changeArr);
	//now do the same with vertical scanning
	posS = 0;
	changeArr = (int *) malloc(sizeof(int) * widthS);
	for (i = 0; i < widthS; i++){
		//for each column of the small image, count up how often the color changes
		pos = posS;
		changeArr[i] = 0;
		pxPrev = pxGridS[pos];
		pos += widthS;
		for (j = 1; j < heightS; j++){
			changeArr[i] += (pxPrev != pxGridS[pos]);
			pxPrev = pxGridS[pos];
			pos += widthS;
		}
		posS++;
	}
	//after all the color changes have been counted, try to find the starting area (defined as one quarter of the width of the small image) with the greates amount of change
	lenC = (widthS / 4) + 1;
	lenT = widthS - lenC + 1;
	changeCount = 0;
	for (i = 0; i < lenT; i++){
		//start to count changes on each column
		changeCountBuf = 0;
		for (j = 0; j < lenC; j++){
			changeCountBuf += changeArr[i + j];
		}
		if (changeCountBuf > changeCount){
			changeCount = changeCountBuf;
			posS = i;
		}
	}
	free((void *) changeArr);
	//finally check whether the area with the greatest change is in the horizontal or vertical direction
	if (((changeCount * 10000) / heightS) > ((changeCountH * 10000) / widthS)){ //we must compare the change density instead of the change volume to ensure a faster search
		//if the adjusted change count is greater for the vertical scan
		jumpPS = posS;
		jumpPB = posS;
		movPS = widthS;
		movPB = widthB;
		lenP = heightS;
		movOS = 1;
		movOB = 1;
		lenO = widthS - posS;
		lenR = posS;
	}
	else {
		//otherwise we use the horizontal scan
		jumpPS = posB * widthS;
		jumpPB = posB * widthB;
		movPS = 1;
		movPB = 1;
		lenP = widthS;
		movOS = widthS;
		movOB = widthB;
		lenO = heightS - posB;
		lenR = posB;
	}
	//adjust the color pixel counts to match the required precision minus 10 (to try and compensate for the random distribution of errors)
	adjPrec = precision - 0.1;
	for (i = 0; i < colorCount; i++){
		colorPixelCountC[i] = colorPixelCount[i] = (int)(((float) colorPixelCount[i]) * adjPrec);
	}
	//determine the size of the color check grid by seeing how many color check tiles can fit in the big image
	//get the size of the tile grid in the horizontal direction
	lenT = 2 * widthS; //the standard grid tile's width is twice the width of the small image
	widthLenT = widthB / lenT;
	widthLenT += ((widthB % lenT) / widthS); //the tile grid has a final horizontal tile if the remaining space after all the normal tiles are subtracted is bigger than the width of the small image
	widthT = (int *) malloc(sizeof(int) * widthLenT);
	widthTC  = (int *) malloc(sizeof(int) * widthLenT);
	//fill the width array with the calculated tile width up to the last tile
	widthLenT--;
	lenC = (3 * widthS) - 1; //the area we need to check for the color is bigger than the tile, since it encompasses the area of the small image when starting from the last pixel of the tile
	for (i = 0; i < widthLenT; i++){
		widthT[i] = lenT;
		widthTC[i] = lenC;
	}
	//the last tile's width is dependent on the remaining space
	widthTC[widthLenT] = widthB - (2 * widthS * widthLenT);
	widthT[widthLenT] = widthTC[widthLenT] - widthS + 1;
	widthLenT++;
	//get the size of the tile grid in the vertical direction
	lenT = 2 * heightS; //the standard grid tile's height is twice the height of the small image
	heightLenT = heightB / lenT;
	heightLenT += ((heightB % lenT) / heightS); //the tile grid has a final vertical tile if the remaining space after all the normal tiles are subtracted is bigger than the height of the small image
	heightT = (int *) malloc(sizeof(int) * heightLenT);
	heightTC = (int *) malloc(sizeof(int) * heightLenT);
	//fill the height array with the calculated tile height up to the last tile
	heightLenT--;
	lenC = (3 * heightS) - 1; //the area we need to check for the color is bigger than the tile, since it encompasses the area of the small image when starting from the last pixel of the tile
	for (i = 0; i < heightLenT; i++){
		heightT[i] = lenT;
		heightTC[i] = lenC;
	}
	//the last tile's height is dependent on the remaining space
	heightTC[heightLenT] = heightB - (2 * heightS * heightLenT);
	heightT[heightLenT] = heightTC[heightLenT] - heightS + 1;
	heightLenT++;
	//check each tile in the grid to see if the colors match those of the small image, and check to see if the small image is found if that is the case
	posT = posTL = 0;
	lenT = 2 * widthS;
	movTC = (lenT < widthB)? lenT : widthB;
	lenT = 2 * heightS;
	movTL = ((lenT < heightB)? lenT : heightB) * widthB;
	colorPixelCountC[20] = 0;
	posM = 0;
	matchPxAll = 0;
	matchPxMin = (int) ((float) pxCountS * precision);
	matchCount = 0;
	for (i = 0; i < heightLenT; i++){
		for (j = 0; j < widthLenT; j++){
			//go through all pixels of the tile and count up the colors
			pos = posS = posT;
			for (k = 0; k < heightTC[i]; k++){
				for (l = 0; l < widthTC[j]; l++){
					colorPixelCountC[pxGridB[pos++]]--; //we already converted all colors of the big pixel grid into indexes of the colors present in the small image, which we are tracking with the color pixel count array
				}
				posS += widthB;
				pos = posS;
			}
			//check if all the required colors are present in the tile and reset the color pixel count array
			colorFound = 1;
			for (k = 0; k < colorCount; k++){
				if (colorPixelCountC[k] > 0) colorFound = 0;
				colorPixelCountC[k] = colorPixelCount[k];
			}
			colorPixelCountC[20] = 0;
			//if all the color pixels were found, try to find the small image in the current tile
			if (colorFound){
				//go through every pixel in the tile and try to match the small image starting from that tile
				posCount = widthT[j] * heightT[i];
				posS = posB = posT;
				colB = lenT = j * 2 * widthS;
				maxPosH = lenT + widthT[j];
				lineB = i * 2 * heightS;
				for (k = 0; k < posCount; k++){
					//set the starting position to the predetermined area of greatest change
					posAB = posS + jumpPB;
					posAS = jumpPS;
					//scan both images to macth each pixel
					matchPx = pxCountS;
					isMatch = 1;
					for (l = 0; l < lenO; l++){
						posCS = posAS;
						posCB = posAB;
						for (m = 0; m < lenP; m++){
							matchPx -= (pxGridB[posCB] != pxGridS[posCS]);
							if (matchPx < matchPxMin){
								isMatch = 0;
								goto matchCheck; //if this is not a match stop checking right away
							}
							posCS += movPS;
							posCB += movPB;
						}
						posAS += movOS;
						posAB += movOB;
					}
					//check the rest of the image
					posAB = posS;
					posAS = 0;
					//scan both images to macth each pixel
					for (l = 0; l < lenR; l++){
						posCS = posAS;
						posCB = posAB;
						for (m = 0; m < lenP; m++){
							matchPx -= (pxGridB[posCB] != pxGridS[posCS]);
							if (matchPx < matchPxMin){
								isMatch = 0;
								goto matchCheck; //if this is not a match stop checking right away
							}
							posCS += movPS;
							posCB += movPB;
						}
						posAS += movOS;
						posAB += movOB;
					}
					//check whether it's a match
					matchCheck: if (isMatch) {
						//if it is the action taken is dependent on whether we are looking for the best match
						if (bestMatch){
							//if we are looking for the best match compare it to the previous one and replace it if it is better
							if (matchPx > matchPxAll){
								matchCount = 1;
								matchPxAll = matchPx;
								matchData[0] = lineB;
								matchData[1] = colB;
								matchData[2] =  (matchPx * 1000) / pxCountS;
							}
						}
						else {
							//otherwise simply add it to the list of matches
							matchCount++;
							matchData[posM++] = lineB;
							matchData[posM++] = colB;
							matchData[posM++] = (matchPx * 1000) / pxCountS;
							if (matchPx > matchPxAll) matchPxAll = matchPx;
						}
					}
					//change the match position
					colB++;
					if (colB >= maxPosH){
						//reset the column and change the position if the small image position column is beyond the current tile
						colB = lenT;
						lineB++;
						posB += widthB;
						posS = posB;
					}
					else {
						posS++;
					}
				}
			}
			posT += movTC;
		}
		posTL += movTL;
		posT = posTL;
	}
	//merge the matches if we must
	if (!bestMatch && matchCount && merge){
		mergeMatchData = (int *) malloc(sizeof(int) * (matchCount * 5));
		mergeMatchCount = 0;
		posT = matchCount * 3;
		posTL = 0;
		//for each match check whether it occupies the same space an existing merged match
		for (i = 0; i < posT; i += 3){
			isMatch = 0;
			lineB = matchData[i] + heightS - 1;
			colB = matchData[i + 1] + widthS - 1;
			for (j = 0; j < posTL; j += 5){
				//check whether the match occupies the same space as the current merged match
				if ((((matchData[i] >= mergeMatchData[j]) && (matchData[i] <= mergeMatchData[j + 1])) || ((lineB >= mergeMatchData[j]) && (lineB <= mergeMatchData[j + 1]))) && (((matchData[i + 1] >= mergeMatchData[j + 2]) && (matchData[i + 1] <= mergeMatchData[j + 3])) || ((colB >= mergeMatchData[j + 2]) && (colB <= mergeMatchData[j + 3])))){
					//if it does, compare their match percentages
					if (matchData[i + 2] > mergeMatchData[j + 4]){
						//if the match percentage for the current match is greater than the merged match, replace the merged match
						mergeMatchData[j] = matchData[i];
						mergeMatchData[j + 1] = lineB;
						mergeMatchData[j + 2] = matchData[i + 1];
						mergeMatchData[j + 3] = colB;
						mergeMatchData[j + 4] = matchData[i + 2];
					}
					else if (matchData[i + 2] == mergeMatchData[j + 4]){
						//if the match percentage is equal, check whether the current match is closer to the top left of the screen
						if ((matchData[i] <= mergeMatchData[j]) && (matchData[i + 1] <= mergeMatchData[j + 2])){
							mergeMatchData[j] = matchData[i];
							mergeMatchData[j + 1] = lineB;
							mergeMatchData[j + 2] = matchData[i + 1];
							mergeMatchData[j + 3] = colB;
							mergeMatchData[j + 4] = matchData[i + 2];
						}
					}
					//move on to the next match
					isMatch = 1;
					break;
				}
			}
			//add it to the merged match data if it did not shared space with an existing merged match
			if (!isMatch){
				mergeMatchData[posTL] = matchData[i];
				mergeMatchData[posTL + 1] = lineB;
				mergeMatchData[posTL + 2] = matchData[i + 1];
				mergeMatchData[posTL + 3] = colB;
				mergeMatchData[posTL + 4] = matchData[i + 2];
				mergeMatchCount++;
				posTL += 5;
			}
		}
		//copy the merge match data into the match data array
		matchCount = mergeMatchCount;
		pos = 0;
		for (i = 0; i < posTL; i += 5){
			matchData[pos++] = mergeMatchData[i];
			matchData[pos++] = mergeMatchData[i + 2];
			matchData[pos++] = mergeMatchData[i + 4];
		}
		//clean up
		free((void *) mergeMatchData);
	}
	//cleanup and return the match value
	free((void *) pxGridS);
	free((void *) pxGridB);
	free((void *) widthT);
	free((void *) widthTC);
	free((void *) heightT);
	free((void *) heightTC);
	return matchCount;
}

void GetRelevantRectangle(uint8_t *imgPx, uint32_t *rectDim, int width, int height, int hasAlpha, uint8_t *backgroundColor){
	//this function is used to get the dimensions of the rectangle containing relevant pixels (different from the background color)
	int top = height, bottom = 0, left = width, right = 0;
	//go through every pixel and try to find the top bottom left and right pixels that differ from the background color
	int pxLen = (hasAlpha)? 4 : 3;
	int i, j, k, pos = 0;
	int isRelevant;
	for (i = 0; i < height; i++){
		for (j = 0; j < width; j++){
			//check if the pixel is different from the background color (the alpha channelis not checked)
			isRelevant = 0;
			for (k = 0; k < 3; k++){
				if (imgPx[pos + k] != backgroundColor[k]){
					isRelevant = 1;
					break;
				}
			}
			pos += pxLen;
			//if the pixel is relevant, we see if it lies beyond the borders of the curent relevant rectanlge
			if (isRelevant){
				if (i < top)
					top = i;
				if (i > bottom)
					bottom = i;
				if (j < left)
					left = j;
				if (j > right)
					right = j;
			}
		}
	}
	//assign the values to the relevant rectangle
	rectDim[0] = (uint32_t) top;
	rectDim[1] = (uint32_t) bottom;
	rectDim[2] = (uint32_t) left;
	rectDim[3] = (uint32_t) right;
}

int GetElementList(uint8_t *imgPx, int *elDim, int width, int height, int hasAlpha, uint8_t *backgroundColor, int dimH, int dimV){
	//this function goes through each pixel in the image and creates elements to group them together
	int pxLen = (hasAlpha)? 4 : 3;
	int imgSize = width * height;
	uint8_t *relArr = (uint8_t *) malloc(imgSize);
	int elCount = 0;
	int i, j, k, l, m;
	int left, right, top, bottom, mLeft[8], mRight[8], mTop[8], mBottom[8];
	//the search margins have directions associated with them, which are communicated with masks with top = 1, right = 2, bottom = 4, left = 8
	// |  9 | 1 | 3 |
	// |  8 |   | 2 |
	// | 12 | 4 | 6 |
	int mMask[] = {9, 1, 3, 8, 2, 12, 4, 6}; //this is the margin mask array
	int lineJump, posSEl, pxFound, pxFoundNew;
	int pos = 0, posEl;
	//build the relevant pixel array
	for (i = 0; i < imgSize; i++){
		relArr[i] = 0;
		for (j = 0; j < 3; j++){
			if (imgPx[pos + j] != backgroundColor[j]){
				relArr[i] = 1;
				break;
			}
		}
		pos += pxLen;
	}
	//go through the relevant pixel array and create elements for each sets of pixels separated by less than the defined pad values
	pos = posEl = 0;
	for (i = 0; i < height; i++){
		for (j = 0; j < width; j++){
			//if the current pixel is relevant, create a new element
			if (relArr[pos]){
				relArr[pos] = 0; //turn off the pixel, now that it has been used
				//store the initial dimensions of hte element (one pixel)
				elDim[posEl] = j;
				elDim[posEl + 1] = j + 1;
				elDim[posEl + 2] = i;
				elDim[posEl + 3] = i + 1;
				//loop until no pixel can be found in the search area
				pxFoundNew = 15;
				while (pxFoundNew){
					pxFound = pxFoundNew;
					pxFoundNew = 0;
					//get the dimensions of the maximum search box
					left = elDim[posEl] - dimH;
					if (left < 0)
						left = 0;
					right = elDim[posEl + 1] + dimH;
					if (right > width)
						right = width;
					top = elDim[posEl + 2] - dimV;
					if (top < 0)
						top = 0;
					bottom = elDim[posEl + 3] + dimV;
					if (bottom > height)
						bottom = height;
					//get the dimensions of the search margins
					//the search margins are arranged in this manner around the element:
					// | 0 | 1 | 2 |
					// | 3 |   | 4 |
					// | 5 | 6 | 7 |
					//get the horizontal dimensions
					mLeft[0] = mLeft[3] = mLeft[5] = left;
					mRight[0] = mRight[3] = mRight[5] = mLeft[1] = mLeft[6] = elDim[posEl];
					mRight[1] = mRight[6] = mLeft[2] = mLeft[4] = mLeft[7] = elDim[posEl + 1];
					mRight[2] = mRight[4] = mRight[7] = right;
					//get the vertical dimensions
					mTop[0] = mTop[1] = mTop[2] = top;
					mBottom[0] = mBottom[1] = mBottom[2] = mTop[3] = mTop[4] = elDim[posEl + 2];
					mBottom[3] = mBottom[4] = mTop[5] = mTop[6] = mTop[7] = elDim[posEl + 3];
					mBottom[5] = mBottom[6] = mBottom[7] = bottom;
					//search for a valid pixel in each active search margin
					for (k = 0; k < 8; k++){
						//an area is active if a pixel was found there last round
						if (pxFound & mMask[k]){
							//get the dimensions of the line jump and the starting position of the search pixel
							lineJump = (width - mRight[k]) + mLeft[k];
							posSEl = (mTop[k] * width) + mLeft[k];
							for (l = mTop[k]; l < mBottom[k]; l++){
								for (m = mLeft[k]; m < mRight[k]; m++){
									if (relArr[posSEl]){
										//if we find an active pixel in the seacrh area, we check to see if it is beyond the current borders of the element, and update them if it is
										relArr[posSEl] = 0; //turn off the pixel, now that it has been used
										pxFoundNew |= mMask[k]; //indicate in which margin the pixel was found
										if (m < elDim[posEl]){
											elDim[posEl] = m;
										}
										else if (m >= elDim[posEl + 1]){
											elDim[posEl + 1] = m + 1;
										}
										if (l < elDim[posEl + 2]){
											elDim[posEl + 2] = l;
										}
										else if (l >= elDim[posEl + 3]){
											elDim[posEl + 3] = l + 1;
										}
									}
									posSEl++;
								}
								posSEl += lineJump;
							}
						}
					}
				}
				//increment element position tracker and element count
				posEl += 4;
				elCount++;
			}
			pos++;
		}
	}
	//cleanup
	free((void *) relArr);
	//return the element count
	return elCount;
}