//*** Image Mod Header ***//

#pragma once
#include <stdint.h>
#ifdef BUILD_DLL
	#define DLL_EXPORT __declspec(dllexport)
#else
	#define DLL_EXPORT
#endif
//***** function prototypes *****//
DLL_EXPORT void ColorReduce(uint8_t *pixels, int pixelCount, int hasAlpha, int blackWhite);
DLL_EXPORT void SplitColor(uint8_t *pixels, int pixelCount, int hasAlpha, uint8_t *baseColor, int *threshold, int colorCount, int isBackground);
DLL_EXPORT void ColorReplace(uint8_t *pixels, int pixelCount, int hasAlpha, int ignoreAlpha, uint8_t *oldColor, uint8_t *newColor);
DLL_EXPORT int FillSquareColor(uint8_t *pixels, int width, int height, int hasAlpha, int sqx, int sqy, int sqw, int sqh, uint8_t *color);
DLL_EXPORT void PadImage(uint8_t *pImgPx, uint8_t *imgPx, int width, int height, int hasAlpha, int pad, uint8_t *paddingColor);
DLL_EXPORT void CropImage(uint8_t *imgPx, uint8_t *imgCropPx, uint32_t *rectDim, int width, int hasAlpha);
DLL_EXPORT void EraseLongSegments(uint8_t *imgPx, int width, int height, int hasAlpha, int maxWidth, int maxHeight, uint8_t *backgroundColor);
DLL_EXPORT int RemoveEmptyLines(uint8_t *imgPx, uint8_t *imgRPx, int width, int height, int hasAlpha, int maxLines, uint8_t *backgroundColor);
DLL_EXPORT float PixelMatch(uint8_t *smlImgPx, uint8_t *bigImgPx, int widthS, int heightS, int hasAlphaS, int widthB, int heightB, int hasAlphaB, int ignoreAlpha);
DLL_EXPORT int GetImagePosition(uint8_t *smlImgPx, uint8_t *bigImgPx, int *matchData, int widthS, int heightS, int hasAlphaS, int widthB, int heightB, int hasAlphaB, int ignoreAlpha, float precision, int bestMatch, int merge);
DLL_EXPORT void GetRelevantRectangle(uint8_t *imgPx, uint32_t *rectDim, int width, int height, int hasAlpha, uint8_t *backgroundColor);
DLL_EXPORT int GetElementList(uint8_t *imgPx, int *elDim, int width, int height, int hasAlpha, uint8_t *backgroundColor, int dimH, int dimV);