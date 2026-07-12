#include "imageMod.h"
#include <stdlib.h>
#include <string.h>
//***** type definitions *****//
typedef struct sSegment{
	int start;
	int length;
} Segment;
//constant declarations
static UBYTE trueColor[8][3] = {{0, 0, 0}, {255, 0, 0}, {0, 255, 0}, {0, 0, 255}, {255, 255, 0}, {255, 0, 255}, {0, 255, 255}, {255, 255, 255}};
static UBYTE blackWhiteColor[2][3] = {{0, 0, 0}, {255, 255, 255}};
//***** Function prototypes *****//
static inline void GetColors(UBYTE* pixel, UBYTE* red, UBYTE* green, UBYTE* blue, UBYTE* maxRed, UBYTE* maxGreen, UBYTE* maxBlue);
static inline void CompareColor(int* minDiff, int* colorId, int newColorId, UBYTE red, UBYTE green, UBYTE blue);
static inline void SetColor(UBYTE* pixel, UBYTE* colorArr);
static inline int ColorLine(UBYTE* bA, UBYTE* color, int width, int top, int left, int lineLen, int pxLen);
static inline int FindSegments(char* relArr, Segment* segArr, int dimP, int dimS, int moveP, int moveS, int maxLen);
static inline void ClearSegments(ImageData* img, Segment* segArr, int segCount, int pxLen, int move, UBYTE* backgroundColor);
static inline void BuildPixelArray(ImageData* img, uint32_t *pxArr, int ignoreAlpha);
static inline int CheckMatch(uint32_t *pxArrA, uint32_t *pxArrB, int matchScore, int posStart, int height, int width, int threshold, int lineJump);
static inline void MoveSearchColumn(int* col, int* line, int* pos, int maxPosH, int posChange);

void ColorReduce(ImageData* img, int blackWhite){
	//this function takes the given pixel array, and converts each pixel into a specific color it is closest to
	int i, pixelCount, pxLen, minDiff, colorId;
	UBYTE red, green, blue, maxRed, maxGreen, maxBlue;
	UBYTE* bA = img->bA;
	//go through each pixel, and find which color is the closest
	pixelCount = img->width * img->height;
	pxLen = 3 + img->hasAlpha;
	if (blackWhite){
		//if this is a black and white comparison
		for (i = 0; i < pixelCount; i++){
			GetColors(bA, &red, &green, &blue, &maxRed, &maxGreen, &maxBlue);
			//we look for the minimum difference between each color channel of the pixel and the reference colors
			//by default we start with the assumption that the pixel color is black (0 red, 0 green, 0 blue)
			colorId = 0;
			minDiff = red + green + blue;
			//compare it with white
			CompareColor(&minDiff, &colorId, 1, maxRed, maxGreen, maxBlue);
			//overwrite the pixel colors with the closest true color and shift to next pixel
			SetColor(bA, blackWhiteColor[colorId]);
			bA += pxLen;
		}
	}
	else {
		for (i = 0; i < pixelCount; i++){
			GetColors(bA, &red, &green, &blue, &maxRed, &maxGreen, &maxBlue);
			//we look for the minimum difference between each color channel of the pixel and the reference colors
			//by default we start with the assumption that the pixel color is black (0 red, 0 green, 0 blue)
			colorId = 0;
			minDiff = red + green + blue;
			//compare it with all other colors to find the closest match
			CompareColor(&minDiff, &colorId, 1, maxRed, green, blue); //red
			CompareColor(&minDiff, &colorId, 2, red, maxGreen, blue); //green
			CompareColor(&minDiff, &colorId, 3, red, green, maxBlue); //blue
			CompareColor(&minDiff, &colorId, 4, maxRed, maxGreen, blue); //yellow
			CompareColor(&minDiff, &colorId, 5, maxRed, green, maxBlue); //purple
			CompareColor(&minDiff, &colorId, 6, red, maxGreen, maxBlue); //cyan
			CompareColor(&minDiff, &colorId, 7, maxRed, maxGreen, maxBlue); //white
			//overwrite the pixel colors with the closest true color and shift to next pixel
			SetColor(bA, trueColor[colorId]);
			bA += pxLen;
		}
	}
 }

static inline void GetColors(UBYTE* pixel, UBYTE* red, UBYTE* green, UBYTE* blue, UBYTE* maxRed, UBYTE* maxGreen, UBYTE* maxBlue){
	//this function gets the colors and inverse colors for the current pixel
	*red = pixel[0];
	*green = pixel[1];
	*blue = pixel[2];
	//get all the differences
	*maxRed = 255 - *red;
	*maxGreen = 255 - *green;
	*maxBlue = 255 - *blue;
}

static inline void CompareColor(int* minDiff, int* colorId, int newColorId, UBYTE red, UBYTE green, UBYTE blue){
	//this function compare the current minimum color difference with a color difference with the given color
	int diff = (int) (red + green + blue);
	if (diff < *minDiff){
		//if the new color difference is less than the prvious minimum color difference, it becomes the new minimum color difference
		*minDiff = diff;
		*colorId = newColorId;
	}
}

static inline void SetColor(UBYTE* pixel, UBYTE* colorArr){
	//this function sets the selected color in the current pixel
	pixel[0] = colorArr[0];
	pixel[1] = colorArr[1];
	pixel[2] = colorArr[2];
}

void SplitColor(ImageData* img, UBYTE* baseColor, int* threshold, int colorCount, int isBackground){
	//this function takes the given pixel array, calculate the difference of each pixel to the base color, then make the pixle black or white depending on wheteher they are above or below the average difference
	//we need an array to store the pixel difference and a variable to store the total difference
	int i, j, pixelCount, pxLen, diff, totalDiff;
	int *pixDiff, *thrDiff, *colIndexPx;
	UBYTE repCol, otherCol, *pxPt, *colorPt;
	pixelCount = img->width * img->height;
	totalDiff = 0;
	pxLen = 3 + img->hasAlpha;
	pixDiff = (int *) malloc(sizeof(int) * pixelCount);
	thrDiff = (int *) malloc(sizeof(int) * colorCount);
	colIndexPx = (int *) malloc(sizeof(int) * pixelCount);
	pxPt = img->bA;
	//go through each pixel, get the minimum difference to all the base colors, store it, and add it to the total
	for (i = 0; i < pixelCount; i++){
		pixDiff[i] = 765; //this is 255 times 3, the maximum color difference possible
		colorPt = baseColor;
		for (j = 0; j < colorCount; j++){
			diff = (((pxPt[0] < colorPt[0])? (colorPt[0] - pxPt[0]) : (pxPt[0] - colorPt[0]))); //difference with red
			diff += (((pxPt[1] < colorPt[1])? (colorPt[1] - pxPt[1]) : (pxPt[1] - colorPt[1]))); //difference with green
			diff += (((pxPt[2] < colorPt[2])? (colorPt[2] - pxPt[2]) : (pxPt[2] - colorPt[2]))); //difference with blue
			//check if the total difference is less than the current pixel difference, and store it if it is, as well as the index of the current color
			if (diff < pixDiff[i]){
				pixDiff[i] = diff;
				colIndexPx[i] = j;
			}
			colorPt += 3;
		}
		//add to the total difference
		totalDiff += pixDiff[i];
		//increment pixel pointer
		pxPt += pxLen;
	}
	//the threshold difference depends on the threshold argument
	diff = totalDiff / pixelCount; //average difference
	for (i = 0; i < colorCount; i++){
		//if the threshold for this color does not exist, or is not between the correct values, we take the average difference as the threshold
		thrDiff[i] = ((!threshold) || (threshold[i] < 0) || (threshold[i] > 765))? diff : threshold[i];
	}
	//go through the pixel array again, and set the pixel to black or white depending on whether its difference is greater or lesser than the threshold and the color is background
	repCol = (isBackground)? 255 : 0; //if the colors are background the replacement color is white otherwise it is black
	otherCol = 255 - repCol; //the other color is the inverse
	pxPt = img->bA;
	for (i = 0; i < pixelCount; i++){
		//check the difference and assign the appropriate color
		pxPt[0] = pxPt[1] = pxPt[2] = (pixDiff[i] < thrDiff[colIndexPx[i]])? repCol : otherCol;
		//increment pixel pointer
		pxPt += pxLen;
	}
	//clean up
	free((void *) pixDiff);
	free((void *) thrDiff);
	free((void *) colIndexPx);
}

void ColorReplace(ImageData* img, int ignoreAlpha, UBYTE* oldColor, UBYTE* newColor){
	//this function goes through every pixel in the supplied image and replace the supplied old color with new one
	int i, pixelCount, pos, pxLen;
	pixelCount = img->width * img->height;
	pos = 0;
	if (img->hasAlpha && !ignoreAlpha){
		for ( i = 0; i < pixelCount; i++){
			if ((img->bA[pos] == oldColor[0]) && (img->bA[pos + 1] == oldColor[1]) && (img->bA[pos + 2] == oldColor[2]) && (img->bA[pos + 3] == oldColor[3])) memcpy(&img->bA[pos], newColor, 4);
			pos += 4;
		}
	}
	else {
		pxLen = 3 + img->hasAlpha;
		for ( i = 0; i < pixelCount; i++){
			if ((img->bA[pos] == oldColor[0]) && (img->bA[pos + 1] == oldColor[1]) && (img->bA[pos + 2] == oldColor[2])) memcpy(&img->bA[pos], newColor, 3);
			pos += pxLen;
		}
	}
}

int FillSquareColor(ImageData* img, int sqx, int sqy, int sqw, int sqh, UBYTE *color){
	//this function fills a square of the specified dimensions with the required color on the provided image
	int i, pxLen, lineLen, pos, posStart, sqr, sqb;
	UBYTE colorF[4];
	pxLen = 3 + img->hasAlpha;
	lineLen = (img->width * pxLen);
	//exit if the square location puts it outside the image
	sqr = sqx + sqw;
	sqb = sqy + sqh;
	if ((sqx >= img->width) || (sqr < 0) || (sqy >= img->height) || (sqb < 0)) return -1;
	//adjust the dimensions of the square if they don't fall exactly inside the image
	if (sqx < 0){
		sqw += sqx;
		sqx = 0;
	}
	if (sqr > img->width) sqw = img->width - sqx;
	if (sqy < 0){
		sqh += sqy;
		sqy = 0;
	}
	if (sqb > img->height) sqh = img->height - sqy;
	//if the square fits, color it
	//color first line
	if (img->hasAlpha){
		memcpy(colorF, color, 3);
		colorF[3] = 255;
		posStart = ColorLine(img->bA, colorF, sqw, sqy, sqx, lineLen, 4);
	}
	else {
		posStart = ColorLine(img->bA, color, sqw, sqy, sqx, lineLen, 3);
	}
	//copy first line onto the rest
	pos = posStart;
	for (i = 1; i < sqh; i++){
		pos += lineLen;
		memcpy(&img->bA[pos], &img->bA[posStart], lineLen);
	}
	return 0;
}

static inline int ColorLine(UBYTE* bA, UBYTE* color, int width, int top, int left, int lineLen, int pxLen){
	//this function colors a line of the specified width, with the specified coordinates, with the specified color
	int i, pos, posStart;
	pos = posStart = (top * lineLen) + (left * pxLen);
	for (i = 0; i < width; i++){
		memcpy(&bA[pos], color, pxLen);
		pos += pxLen;
	}
	return posStart;
}

void PadImage(ImageData* imgPad, ImageData* img, int pad, UBYTE* paddingColor){
	//this function padds the provided image and pads it by the requested value with the requested color
	int i, j, pos, posS, pxLen, lineLen, lineLenP;
	UBYTE paddingColorF[4];
	imgPad->width = img->width + (pad * 2);
	imgPad->height = img->height + (pad * 2);
	pxLen = 3 + (imgPad->hasAlpha = img->hasAlpha);
	lineLen = img->width * pxLen;
	lineLenP = imgPad->width * pxLen;
	//copy the padding color on every pixel of the first line of the padded image
	if (imgPad->hasAlpha){
		memcpy(paddingColorF, paddingColor, 3);
		paddingColorF[3] = 255; //padding colors are always fully opaque
		ColorLine(imgPad->bA, paddingColorF, imgPad->width, 0, 0, lineLenP, 4);
	}
	else {
		ColorLine(imgPad->bA, paddingColor, imgPad->width, 0, 0, lineLenP, 3);
	}
	//copy the first line onto all other lines to cover the entire padded image in padding color
	pos = lineLenP;
	for (i = 1; i < imgPad->height; i++){
		memcpy(&imgPad->bA[pos], imgPad->bA, lineLenP);
		pos += lineLenP;
	}
	//now copy the source image onto the padded image at the center
	pos = (lineLenP + pxLen) * pad; //we start at the first pixel that isn't padding
	posS = 0;
	for (i = 0; i < img->height; i++){
		memcpy(&imgPad->bA[pos], &img->bA[posS], lineLen);
		pos += lineLenP;
		posS += lineLen;
	}
}

void CopyArea(ImageData* imgCpy, ImageData* img, Area* area){
	//this function copies the defined area in the provided image to another image object
	int i, pos, posC, pxLen, lineLen, lineLenC;
	imgCpy->width = 1 + area->right - area->left;
	imgCpy->height = 1 + area->bottom - area->top;
	pxLen = 3 + (imgCpy->hasAlpha = img->hasAlpha);
	lineLen = img->width * pxLen;
	lineLenC = imgCpy->width * pxLen;
	//copy each line of the source onto the copy, within the width of the area
	pos = (area->top * lineLen) + (area->left * pxLen);
	posC = 0;
    for (i = 0; i < imgCpy->height; i++){
		memcpy(&imgCpy->bA[posC], &img->bA[pos], lineLenC);
		pos += lineLen;
		posC += lineLenC;
    }
}

void EraseSegments(ImageData* img, int maxWidth, int maxHeight, UBYTE* backgroundColor){
	//this function looks for all horizontal segments wider than the mex width and vertical segments higher than the max height and erases them
	char *relArr;
	int i, pos, imgSize, pxLen, segHCount, segVCount;
	float imgSizeF;
	Segment *segHArr, *segVArr;
	imgSize = img->width * img->height;
	pxLen = 3 + img->hasAlpha;
	relArr = (char *) malloc(imgSize);
	imgSizeF = (float) imgSize;
	segHArr = (Segment *) malloc(sizeof(Segment) * ((int) (imgSizeF / ((float) maxWidth)) + 1));
	segVArr = (Segment *) malloc(sizeof(Segment) * ((int) (imgSizeF / ((float) maxHeight)) + 1));
	//build the relevant pixel array
	pos = 0;
	for (i = 0; i < imgSize; i++){
		relArr[i] = ((img->bA[pos] != backgroundColor[0]) && (img->bA[pos + 1] != backgroundColor[1]) && (img->bA[pos + 2] != backgroundColor[2]));
		pos += pxLen;
	}
	//go through the array and find all horizontal segments greater than the maximum width
	segHCount = FindSegments(relArr, segHArr, img->height, img->width, img->width, 1, maxWidth);
	//go through the array and find all vertical segments greater than the maximum height
	segVCount = FindSegments(relArr, segVArr, img->width, img->height, 1, img->width, maxHeight);
	//clear horizontal segments
	ClearSegments(img, segHArr, segHCount, pxLen, pxLen, backgroundColor);
	//clear vertical segments
	ClearSegments(img, segVArr, segVCount, pxLen, (img->width * pxLen), backgroundColor);
	//cleanup
	free((void *) relArr);
	free((void *) segHArr);
	free((void *) segVArr);
}

static inline int FindSegments(char* relArr, Segment* segArr, int dimP, int dimS, int moveP, int moveS, int maxLen){
	//this function finds segments larger than the given size in the image
	int i, j, posS, pos, segLen, segStart, segCount;
	posS = segStart = segCount = 0;
	for (i = 0; i < dimP; i++){
		pos = posS;
		segLen = 0;
		for (j = 0; j < dimS; j++){
			if (relArr[pos]){
				//if this is a relevant pixel, add it to the segment
				if (!segLen) segStart = pos; //if there is no started segment, we start it here
				segLen++;
			}
			else if (segLen){
				//if this is not a relevant pixel, and there is a segment started, create a new segment
				if (segLen > maxLen) segArr[segCount++] = (Segment){.start = segStart, .length = segLen};
				//in any case reset the segment length to 0
				segLen = 0;
			}
			pos += moveS;
		}
		//at the end of the line or column, check if there is a valid segment and add it to the array if it is
		if (segLen > maxLen) segArr[segCount++] = (Segment){.start = segStart, .length = segLen};
		posS += moveP;
	}
	return segCount;
}

static inline void ClearSegments(ImageData* img, Segment* segArr, int segCount, int pxLen, int move, UBYTE* backgroundColor){
	//this function replaces targeted segments with supplied background color
	int i, j, k, pos;
	for (i = 0; i < segCount; i++){
		pos = segArr[i].start * pxLen;
		for (j = 0; j < segArr[i].length; j++){
			for (k = 0; k < 3; k++){
				img->bA[pos + k] = backgroundColor[k];
			}
			pos += move;
		}
	}
}

void RemoveEmptyLines(ImageData* imgRm, ImageData* img, int maxLines, UBYTE* backgroundColor){
	//this function removes the lines in the provided image that are empty (have only pixels of the background color) after the specified maximum allowed
    int i, j, pos, posL, posR, pxLen, lineLen, lineCount, isFull;
	//go through each line to check if the line is empty
	imgRm->width = img->width;
	pxLen = 3 + (imgRm->hasAlpha = img->hasAlpha);
	lineLen = img->width * pxLen;
	posL = posR = lineCount = imgRm->height = 0;
	for (i = 0; i < img->height; i++){
		pos = posL;
		for (j = 0; j < img->width; j++){
			//check for each pîxel in the line if the pixel color differs in any channel from the background color
			if (isFull = ((img->bA[pos] != backgroundColor[0]) || (img->bA[pos + 1] != backgroundColor[1]) || (img->bA[pos + 2] != backgroundColor[2]))) break; //if it does, the line is not empty, move on
			pos += pxLen;
		}
		//if the current line is not empty, we must copy lines
		if (isFull){
			if ((lineCount > 1) && (lineCount <= maxLines)){
				//if the number of empty lines is less than the maximum allowed, copy them as well as the current line
				pos = posL - ((lineCount - 1) * lineLen);
			}
			else {
				//otherwise copy only the current line
				lineCount = 1;
				pos = posL;
			}
			for (j = 0; j < lineCount; j++){
				memcpy(&imgRm->bA[posR], &img->bA[pos], lineLen);
				posR += lineLen;
				pos += lineLen;
				imgRm->height++;
			}
			lineCount = 0;
		}
		lineCount++;
		posL += lineLen;
	}
}

float PixelMatch(ImageData* imgSml, ImageData* imgBig, int ignoreAlpha){
	//this function checks the small and big images and returns the percentage of pixels that match for the best match
	int i, pos, posB, colB, pxCount, lineJump, maxPosH, posCount, matchScore, matchScoreBest;
	uint32_t *pxArrSml, *pxArrBig;
	//check that the small image is smaller in width and height than the big image
	if ((imgSml->width > imgBig->width) || (imgSml->height > imgBig->height)) return 0;
	//to speed up the comparisons, build an array of 32 bit integers representing the pixels for both images
	pxCount = (imgSml->width * imgSml->height);
	pxArrSml = (uint32_t *) malloc(sizeof(uint32_t) * pxCount);
	BuildPixelArray(imgSml, pxArrSml, ignoreAlpha);
	pxArrBig = (uint32_t *) malloc(sizeof(uint32_t) * (imgBig->width * imgBig->height));
	BuildPixelArray(imgBig, pxArrBig, ignoreAlpha);
	//get the number of possible positions that the small image can fit into the large one
	lineJump = imgBig->width - imgSml->width;
	maxPosH = lineJump + 1;
	posCount = maxPosH * (imgBig->height - imgSml->height + 1);
	posB = colB = matchScoreBest = 0;
	for (i = 0; i < posCount; i++){
		matchScore = CheckMatch(pxArrBig, pxArrSml, pxCount, posB + colB, imgSml->height, imgSml->width, matchScoreBest, lineJump);
		//check if it's a better match and assign it if it is
		if (matchScore > matchScoreBest) matchScoreBest = matchScore;
		//change the match position
		colB++;
		if (colB >= maxPosH){
			//reset the column and change the position if the small image position column is above the maximum possible horizontal positions
			colB = 0;
			posB += imgBig->width;
		}
	}
	//cleanup and return the match value
	free((void *) pxArrSml);
	free((void *) pxArrBig);
	return ((float) matchScoreBest / (float) pxCount);
}

static inline void BuildPixelArray(ImageData* img, uint32_t *pxArr, int ignoreAlpha){
	//this function build the 32 bit pixel array associated with the given image
	int i, pos, pxLen, pxCount;
	if (img->hasAlpha && !ignoreAlpha){
		//if the image has an alpha channel (and therefore is already 32 bit), and the alpha channel is not ignored, simply copy it
		memcpy((char *) pxArr, img->bA, (img->width * img->height * 4));
	}
	else {
		//if the image does not have an alpha channel, or it is ignored in the comparison, copy the red, green, and blue channels to the pixel array and set the transparency channel to zero
		pxLen = 3 + img->hasAlpha;
		pxCount = img->width * img->height;
		memset((char *) pxArr, 0, (pxCount * pxLen)); //set all bytes to zero
		pos = 0;
		for (i = 0; i < pxCount; i++){
			//copy the red, green, and blue channels
			memcpy((char *) pxArr, &img->bA[pos], 3);
			pos += pxLen;
			pxArr++;
		}
	}
}

static inline int CheckMatch(uint32_t *pxArrA, uint32_t *pxArrB, int matchScore, int posStart, int height, int width, int threshold, int lineJump){
	//this function checks how often two pixel arrays match and returns if it goes below a threshold
	int i, j, posA, posB;
	posA = posStart;
	posB = 0;
	for (i = 0; i < height; i++){
		for (j = 0; j < width; j++){
			//remove one from the match score for each pixel that doesn't match
			matchScore -= (pxArrA[posA++] != pxArrB[posB++]);
		}
		if (matchScore < threshold) break; //if the match score is below the threshold, exit
		posA += lineJump; //move to the next line in the big image
	}
	return matchScore;
}

int GetImagePosition(ImageData* imgSml, ImageData* imgBig, MatchData* matchData, int ignoreAlpha, float precision, int bestMatch, int merge){
	//this function tries to find the small image in the big image with the given precision (percentage of matching pixels)
	int i, j, posB, colB, lineB, pxCount, lineJump, maxPosH, posCount, matchScore, threshold, matchCount, mergeMatchCount, widthAdj, heightAdj, isMatch;
	MatchData *mergeMatchData;
	float pxCountF;
	uint32_t *pxArrSml, *pxArrBig;
	//check that the small image is smaller in width and height than the big image
	if ((imgSml->width > imgBig->width) || (imgSml->height > imgBig->height)) return 0;
	//to speed up the search, build an array of 32 bit inetegers representing the pixels for both images
	pxCount = (imgSml->width * imgSml->height);
	pxCountF = (float) pxCount;
	pxArrSml = (uint32_t *) malloc(sizeof(uint32_t) * pxCount);
	BuildPixelArray(imgSml, pxArrSml, ignoreAlpha);
	pxArrBig = (uint32_t *) malloc(sizeof(uint32_t) * (imgBig->width * imgBig->height));
	BuildPixelArray(imgBig, pxArrBig, ignoreAlpha);
	//whether we add the matches up or remove the misses from the maximum is dependent on the required precision and whether this is a best only match
	lineJump = imgBig->width - imgSml->width;
	maxPosH = lineJump + 1;
	posCount = maxPosH * (imgBig->height - imgSml->height + 1);
	matchCount = posB = colB = lineB = 0;
	if (bestMatch){
		//if this is a best match search, the threshold is the currenct best match score
		threshold = 0;
		for (i = 0; i < posCount; i++){
			matchScore = CheckMatch(pxArrBig, pxArrSml, pxCount, posB + colB, imgSml->height, imgSml->width, threshold, lineJump);
			//check if it's a better match
			if (matchScore > threshold){
				//if it is record it, and record the position and match percentage
				threshold = matchScore;
				matchCount = 1;
				matchData[0] = (MatchData){.a.top = lineB, .a.bottom = 0, .a.left = colB, .a.right = 0, .matchP = (float) matchScore / pxCountF};
			}
			//change the search position
			MoveSearchColumn(&colB, &lineB, &posB, maxPosH, imgBig->width);
		}
	}
	else {
		//if this is a search for all matches above the threshold
		threshold = (int) ((float) pxCount * precision);
		widthAdj = imgSml->width - 1;
		heightAdj = imgSml->height - 1;
		//the search goes faster if we start from the maximum match and remove the misses
		for (i = 0; i < posCount; i++){
			matchScore = CheckMatch(pxArrBig, pxArrSml, pxCount, posB + colB, imgSml->height, imgSml->width, threshold, lineJump);
			//check if it's above the threshold, and, if it is, record it and the position and match percentage
			if (matchScore > threshold) matchData[matchCount++] = (MatchData){.a.top = lineB, .a.bottom = lineB + heightAdj, .a.left = colB, .a.right = colB + widthAdj, .matchP = (float) matchScore / pxCountF};
			//change the search position
			MoveSearchColumn(&colB, &lineB, &posB, maxPosH, imgBig->width);
		}
		//if the merge signal is on and there is at least two matches, merge all matches that overlap
		if ((matchCount > 1) && merge){
			mergeMatchData = (MatchData *) malloc(sizeof(MatchData) * matchCount);
			mergeMatchCount = 0;
			//for each match check whether it occupies the same space an existing merged match
			for (i = 0; i < matchCount; i++){
				isMatch = 0;
				for (j = 0; j < mergeMatchCount; j++){
					//check whether the match occupies the same space as the current merged match
					if ((((matchData[i].a.left >= mergeMatchData[j].a.left) && (matchData[i].a.left <= mergeMatchData[j].a.right)) || ((matchData[i].a.right >= mergeMatchData[j].a.left) && (matchData[i].a.right <= mergeMatchData[j].a.right))) && (((matchData[i].a.top >= mergeMatchData[j].a.top) && (matchData[i].a.top <= mergeMatchData[j].a.bottom)) || ((matchData[i].a.bottom >= mergeMatchData[j].a.top) && (matchData[i].a.bottom <= mergeMatchData[j].a.bottom)))){
						//if it does, compare their match percentages
						if (matchData[i].matchP > mergeMatchData[j].matchP){
							//if the match percentage for the current match is greater than the merged match, replace the merged match
							mergeMatchData[j] = matchData[i];
						}
						else if (matchData[i].matchP == mergeMatchData[j].matchP){
							//if the match percentage is equal, check whether the current match is closer to the top left of the screen
							if ((matchData[i].a.left <= mergeMatchData[j].a.left) && (matchData[i].a.top <= mergeMatchData[j].a.top)) mergeMatchData[j] = matchData[i];
						}
						//move on to the next match
						isMatch = 1;
						break;
					}
				}
				//add it to the merged match data if it did not shared space with an existing merged match
				if (!isMatch) mergeMatchData[mergeMatchCount++] = matchData[i];
			}
			//copy the merge match data into the match data array
			matchCount = mergeMatchCount;
			memcpy((char *) matchData, (char *) mergeMatchData, sizeof(MatchData) * mergeMatchCount);
			//clean up
			free((void *) mergeMatchData);
		}
	}
	//clean up and return the match count
	free((void *) pxArrSml);
	free((void *) pxArrBig);
	return matchCount;
}

static inline void MoveSearchColumn(int* col, int* line, int* pos, int maxPosH, int posChange){
	//this function moves the search column and checks whether it is beyond the maximum horizontal position
	(*col)++;
	if (*col >= maxPosH){
		//reset the column and change the position if the column is above the maximum possible horizontal positions
		*col = 0;
		(*line)++;
		*pos += posChange;
	}
}

void GetRelevantArea(ImageData* img, Area* area, UBYTE* backgroundColor){
	//this function is used to get the dimensions of the area containing relevant pixels (different from the background color)
	//go through every pixel and try to find the top bottom left and right pixels that differ from the background color
	int i, j, pos, pxLen;
	*area = (Area){.top = img->height, .bottom = -1, .left = img->width, .right = -1};
	pxLen = 3 + img->hasAlpha;
	pos = 0;
	for (i = 0; i < img->height; i++){
		for (j = 0; j < img->width; j++){
			//check if the pixel is different from the background color (the alpha channelis not checked)
			if ((img->bA[pos] != backgroundColor[0]) || (img->bA[pos + 1] != backgroundColor[1]) || (img->bA[pos + 2] != backgroundColor[2])){
				//if the pixel is relevant, we see if it lies beyond the borders of the curent relevant area, and expand it if it does
				if (i < area->top) area->top = i;
				if (i > area->bottom) area->bottom = i;
				if (j < area->left) area->left = j;
				if (j > area->right) area->right = j;
			}
			pos += pxLen;
		}
	}
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