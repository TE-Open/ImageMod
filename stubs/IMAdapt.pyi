from collections.abc import Callable as Callable
from ctypes import Array, CDLL, Structure, c_float, c_int

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
    simpleColorReduce: Callable[[pointID, u_int], None]
    colorReduce: Callable[[pointID, pointCI, u_int], None]
    splitColor: Callable[[pointID, pointCI, u_int], None]
    colorReplace: Callable[[pointID, u_int, pointCI, pointCI], None]
    fillSquareColor: Callable[[pointID, u_int, u_int, u_int, u_int, pointCI], c_int]
    padImage: Callable[[pointID, pointID, u_int, pointCI], None]
    copyArea: Callable[[pointID, pointID, pointAR], None]
    eraseSegments: Callable[[pointID, u_int, u_int, pointCI], None]
    removeEmptyLines: Callable[[pointID, pointID, u_int, pointCI], None]
    pixelMatch: Callable[[pointID, pointID, u_int, u_float], c_float]
    getImagePosition: Callable[[pointID, pointID, pointMD, u_int, u_float, u_int, u_int, u_int], c_int]
    getImageColors: Callable[[pointID, pointCI, u_int], c_int]
    getImageColorPixels: Callable[[pointID, pointCI, pointI, u_int], c_int]
    checkColorPresence: Callable[[pointID, pointCI, u_int, u_int], c_int]
    getColorPixelCount: Callable[[pointID, pointCI, u_int], c_int]
    getRelevantArea: Callable[[pointID, pointAR, pointCI], None]
    getElementList: Callable[[pointID, pointAR, pointCI, u_int, u_int], c_int]
    def __init__(self, path: str = '') -> None: ...
