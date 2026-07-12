//*** Image Mod Header ***//

#pragma once
#include <stdint.h>
//***** type definitions *****//
#define UBYTE unsigned char

typedef struct sImageData{
	UBYTE* bA; //byte array containing the image data
	int width;
	int height;
	int hasAlpha;
} ImageData;

typedef struct sArea{
	int top;
	int bottom;
	int left;
	int right;
} Area;

typedef struct sMatchData{
	Area a;
	float matchP; //match percentage
} MatchData;

//***** function prototypes *****//
void ColorReduce(ImageData* img, int blackWhite);
void SplitColor(ImageData* img, UBYTE* baseColor, int* threshold, int colorCount, int isBackground);
void ColorReplace(ImageData* img, int ignoreAlpha, UBYTE* oldColor, UBYTE* newColor);
int FillSquareColor(ImageData* img, int sqx, int sqy, int sqw, int sqh, UBYTE* color);
void PadImage(ImageData* imgPad, ImageData* img, int pad, UBYTE* paddingColor);
void CopyArea(ImageData* imgCpy, ImageData* img, Area* area);
void EraseSegments(ImageData* img, int maxWidth, int maxHeight, UBYTE* backgroundColor);
void RemoveEmptyLines(ImageData* imgRm, ImageData* img, int maxLines, UBYTE* backgroundColor);
float PixelMatch(ImageData* imgSml, ImageData* imgBig, int ignoreAlpha);
int GetImagePosition(ImageData* imgSml, ImageData* imgBig, MatchData* matchData, int ignoreAlpha, float precision, int bestMatch, int merge);
void GetRelevantRectangle(uint8_t *imgPx, uint32_t *rectDim, int width, int height, int hasAlpha, uint8_t *backgroundColor);
int GetElementList(uint8_t *imgPx, int *elDim, int width, int height, int hasAlpha, uint8_t *backgroundColor, int dimH, int dimV);