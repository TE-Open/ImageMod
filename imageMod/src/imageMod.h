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

typedef struct sColorItem{
	UBYTE red;
	UBYTE green;
	UBYTE blue;
	UBYTE alpha;
} ColorItem;

//***** function prototypes *****//
void SimpleColorReduce(ImageData* img, int blackWhite);
void ColorReduce(ImageData* img, ColorItem* colorArr, int colorCount);
void SplitColor(ImageData* img, ColorItem* baseColor, int* threshold, int colorCount, int isBackground);
void ColorReplace(ImageData* img, int ignoreAlpha, ColorItem* oldColor, ColorItem* newColor);
int FillSquareColor(ImageData* img, int sqx, int sqy, int sqw, int sqh, ColorItem* color);
void PadImage(ImageData* imgPad, ImageData* img, int pad, ColorItem* paddingColor);
void CopyArea(ImageData* imgCpy, ImageData* img, Area* area);
void EraseSegments(ImageData* img, int maxWidth, int maxHeight, ColorItem* backgroundColor);
void RemoveEmptyLines(ImageData* imgRm, ImageData* img, int maxLines, ColorItem* backgroundColor);
float PixelMatch(ImageData* imgSml, ImageData* imgBig, int ignoreAlpha, float minMatch);
int GetImagePosition(ImageData* imgSml, ImageData* imgBig, MatchData* matchData, int ignoreAlpha, float precision, int bestMatch, int merge, int colorCheckCount);
int GetImageColors(ImageData* img, ColorItem* colorArr, int maxColor);
int CheckColorPresence(ImageData* img, ColorItem* colorArr, int colorCount, int ignoreAlpha);
int GetColorPixelCount(ImageData* img, ColorItem* color, int ignoreAlpha);
void GetRelevantArea(ImageData* img, Area* area, ColorItem* backgroundColor);
int GetElementList(ImageData* img, Area* elList, ColorItem* backgroundColor, int dimH, int dimV);