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

int GetElementList(ImageData* img, Area* elList, UBYTE* backgroundColor, int dimH, int dimV){
	//this function goes through each pixel in the image and creates elements to group them together
	int i, j, k, l, posP, pos, posC, pxLen, pixelCount, pxFound, elCount, len, maxH, maxV;
	char *relPx, *zeroPx;
	Area searchArea;
	//build the relevant pixel array (pixels with a color different than the background)
	pxLen = 3 + img->hasAlpha;
	pixelCount = img->width * img->height;
	relPx = malloc(pixelCount);
	pos = 0;
	for (i = 0; i < pixelCount; i++){
		relPx[i] = ((img->bA[pos] != backgroundColor[0]) || (img->bA[pos + 1] != backgroundColor[1]) || (img->bA[pos + 2] != backgroundColor[2]));
		pos += pxLen;
	}
	//go through the relevant pixel array and create elements for each sets of pixels separated by less than the defined pad values
	elCount = 0;
	maxH = img->width - 1;
	maxV = img->height - 1;
	zeroPx = malloc(img->width); //this is a comparison array for horizontal pixel checks
	memset(zeroPx, 0, img->width);
	posP = 0;
	for (i = 0; i < img->height; i++){
		for (j = 0; j < img->width; j++){
			//if the current pixel is relevant, create a new element
			if (relPx[posP++]){
				//store the initial dimensions of the element (one pixel)
				elList[elCount] = (Area) {.top = i, .bottom = i, .left = j, .right = j};
				//loop until no pixel can be found in the search area
				pxFound = 1;
				searchArea = (Area) {.top = elList[elCount].top - dimV, .bottom = elList[elCount].bottom + dimV , .left = elList[elCount].left - dimH, .right = elList[elCount].right + dimH};
				if (searchArea.top < 0) searchArea.top = 0;
				if (searchArea.bottom > maxV) searchArea.bottom = maxV;
				if (searchArea.left < 0) searchArea.left = 0;
				if (searchArea.right > maxH) searchArea.right = maxH;
				while (pxFound){
					//we search for relevant pixels by looking at the areas beneath and to the side of the current element in a clockwise direction
					pxFound = 0;
					//search the right area
					posC = pos = (searchArea.top * img->width) + (searchArea.right);
					for (k = searchArea.right; k > elList[elCount].right; k--){
						for (l = searchArea.top; l <= searchArea.bottom; l++){
							if (relPx[pos]){
								//if there was a relevant pixel found in the area, set the current element right to it, update the search area
								elList[elCount].right = k;
								searchArea.right = k + dimH;
								if (searchArea.right > maxH) searchArea.right = maxH;
								pxFound = 1;
								goto IM_GEL_RIGHT_END;
							}
							pos += img->width;
						}
						posC--;
						pos = posC;
					}
					IM_GEL_RIGHT_END:
					//search the bottom area
					len = searchArea.right - searchArea.left + 1;
					pos = (searchArea.bottom * img->width) + searchArea.left;
					for (k = searchArea.bottom; k > elList[elCount].bottom; k--){
						if (memcmp(&relPx[pos], zeroPx, len)){
							//if the current line is not all zeros, set the current element bottom to it, set the new search area, update the search area
							elList[elCount].bottom = k;
							searchArea.bottom = k + dimV;
							if (searchArea.bottom > maxV) searchArea.bottom = maxV;
							pxFound = 1;
							break;
						}
						pos -= img->width;
					}
					//search the left area if active
					posC = pos = (searchArea.top * img->width) + (searchArea.left);
					for (k = searchArea.left; k < elList[elCount].left; k++){
						for (l = searchArea.top; l <= searchArea.bottom; l++){
							if (relPx[pos]){
								//if there was a relevant pixel found in the area, set the current element left to it, update the search area
								elList[elCount].left = k;
								searchArea.left = k - dimH;
								if (searchArea.left < 0) searchArea.left = 0;
								pxFound = 1;
								goto IM_GEL_LEFT_END;
							}
							pos += img->width;
						}
						posC++;
						pos = posC;
					}
					IM_GEL_LEFT_END:
				}
				//set all pixels in the element to zero as they are no longer relevant
				len = elList[elCount].right - elList[elCount].left + 1;
				pos = (elList[elCount].top * img->width) + elList[elCount].left;
				for (k = elList[elCount].top; k <= elList[elCount].bottom; k++){
					memset(&relPx[pos], 0, len);
					pos += img->width;
				}
				//increment element count
				elCount++;
			}
		}
	}
	//cleanup
	free((void *) relPx);
	free((void *) zeroPx);
	//return the element count
	return elCount;
}