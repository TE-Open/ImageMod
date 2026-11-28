from collections.abc import Callable as Callable
from ctypes import Array, CDLL, Structure, _Pointer, c_ubyte

class ImageData(Structure):
    bA: _Pointer[c_ubyte] | Array[c_ubyte]
    width: int
    height: int
    hasAlpha: int
    def getByteCount(self) -> int: ...

class Area(Structure):
    top: int
    bottom: int
    left: int
    right: int
    def getOffset(self, x: int, y: int) -> Area: ...

class MatchData(Structure):
    a: Area
    matchP: float
    @staticmethod
    def getArrayIP(imgDSml: ImageData, imgDBig: ImageData) -> Array[MatchData]: ...

class ColorItem(Structure):
    red: int
    green: int
    blue: int
    alpha: int
    def __eq__(self, color): ...

class ImageModSL:
    SL: CDLL
    simpleColorReduce: Callable[[pointID, int], None]
    colorReduce: Callable[[pointID, pointCI, int], None]
    splitColor: Callable[[pointID, pointCI, int], None]
    colorReplace: Callable[[pointID, int, pointCI, pointCI], None]
    fillSquareColor: Callable[[pointID, int, int, int, int, pointCI], int]
    padImage: Callable[[pointID, pointID, int, pointCI], None]
    copyArea: Callable[[pointID, pointID, pointAR], None]
    eraseSegments: Callable[[pointID, int, int, pointCI], None]
    removeEmptyLines: Callable[[pointID, pointID, int, pointCI], None]
    pixelMatch: Callable[[pointID, pointID, int, float], float]
    getImagePosition: Callable[[pointID, pointID, pointMD, int, float, int, int, int], int]
    getImageColors: Callable[[pointID, pointCI, int], int]
    getImageColorPixels: Callable[[pointID, pointCI, pointI, int], int]
    checkColorPresence: Callable[[pointID, pointCI, int, int], int]
    getColorPixelCount: Callable[[pointID, pointCI, int], int]
    getRelevantArea: Callable[[pointID, pointAR, pointCI], None]
    getElementList: Callable[[pointID, pointAR, pointCI, int, int], int]
    def __init__(self, path: str = '') -> None: ...
