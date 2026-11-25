from collections.abc import Callable as Callable
from ctypes import Array, CDLL, Structure, _Pointer, c_float, c_int

class ImageData(Structure):
    def getByteCount(self) -> int: ...

class Area(Structure):
    def getOffset(self, x: int, y: int) -> Area: ...

class MatchData(Structure):
    @staticmethod
    def getArrayIP(imgDSml: ImageData, imgDBig: ImageData) -> Array[MatchData]: ...

class ColorItem(Structure):
    def __eq__(self, color): ...

class ImageModSL:
    SL: CDLL
    simpleColorReduce: Callable[[_Pointer[ImageData], c_int], None]
    colorReduce: Callable[[_Pointer[ImageData], _Pointer[ColorItem], c_int], None]
    splitColor: Callable[[_Pointer[ImageData], _Pointer[ColorItem], c_int], None]
    colorReplace: Callable[[_Pointer[ImageData], c_int, _Pointer[ColorItem], _Pointer[ColorItem]], None]
    fillSquareColor: Callable[[_Pointer[ImageData], c_int, c_int, c_int, c_int, _Pointer[ColorItem]], c_int]
    padImage: Callable[[_Pointer[ImageData], _Pointer[ImageData], c_int, _Pointer[ColorItem]], None]
    copyArea: Callable[[_Pointer[ImageData], _Pointer[ImageData], _Pointer[Area]], None]
    eraseSegments: Callable[[_Pointer[ImageData], c_int, c_int, _Pointer[ColorItem]], None]
    removeEmptyLines: Callable[[_Pointer[ImageData], _Pointer[ImageData], c_int, _Pointer[ColorItem]], None]
    pixelMatch: Callable[[_Pointer[ImageData], _Pointer[ImageData], c_int, c_float], c_float]
    getImagePosition: Callable[[_Pointer[ImageData], _Pointer[ImageData], _Pointer[MatchData], c_int, c_float, c_int, c_int, c_int], c_int]
    getImageColors: Callable[[_Pointer[ImageData], _Pointer[ColorItem], c_int], c_int]
    getImageColorPixels: Callable[[_Pointer[ImageData], _Pointer[ColorItem], _Pointer[c_int], c_int], c_int]
    checkColorPresence: Callable[[_Pointer[ImageData], _Pointer[ColorItem], c_int, c_int], c_int]
    getColorPixelCount: Callable[[_Pointer[ImageData], _Pointer[ColorItem], c_int], c_int]
    getRelevantArea: Callable[[_Pointer[ImageData], _Pointer[Area], _Pointer[ColorItem]], None]
    getElementList: Callable[[_Pointer[ImageData], _Pointer[Area], _Pointer[ColorItem], c_int, c_int], c_int]
    def __init__(self, path: str = '') -> None: ...
