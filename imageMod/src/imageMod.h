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

//***** function prototypes *****//
void ColorReduce(ImageData* img, int blackWhite);
void SplitColor(ImageData* img, UBYTE* baseColor, int* threshold, int colorCount, int isBackground);
void ColorReplace(ImageData* img, int ignoreAlpha, UBYTE* oldColor, UBYTE* newColor);
int FillSquareColor(uint8_t *pixels, int width, int height, int hasAlpha, int sqx, int sqy, int sqw, int sqh, uint8_t *color);
void PadImage(uint8_t *pImgPx, uint8_t *imgPx, int width, int height, int hasAlpha, int pad, uint8_t *paddingColor);
void CropImage(uint8_t *imgPx, uint8_t *imgCropPx, uint32_t *rectDim, int width, int hasAlpha);
void EraseLongSegments(uint8_t *imgPx, int width, int height, int hasAlpha, int maxWidth, int maxHeight, uint8_t *backgroundColor);
int RemoveEmptyLines(uint8_t *imgPx, uint8_t *imgRPx, int width, int height, int hasAlpha, int maxLines, uint8_t *backgroundColor);
float PixelMatch(uint8_t *smlImgPx, uint8_t *bigImgPx, int widthS, int heightS, int hasAlphaS, int widthB, int heightB, int hasAlphaB, int ignoreAlpha);
int GetImagePosition(uint8_t *smlImgPx, uint8_t *bigImgPx, int *matchData, int widthS, int heightS, int hasAlphaS, int widthB, int heightB, int hasAlphaB, int ignoreAlpha, float precision, int bestMatch, int merge);
void GetRelevantRectangle(uint8_t *imgPx, uint32_t *rectDim, int width, int height, int hasAlpha, uint8_t *backgroundColor);
int GetElementList(uint8_t *imgPx, int *elDim, int width, int height, int hasAlpha, uint8_t *backgroundColor, int dimH, int dimV);