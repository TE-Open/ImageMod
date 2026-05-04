import os
import pathlib
import platform
from ctypes import c_int, c_char, c_float, c_ubyte, c_uint32, c_int64, POINTER, CDLL

class ImageModSL:
	#this class is used to access the functions in the shared library
	def __init__(self, path = None):
		#check the operating system and get the path for the shared library
		system = platform.system()
		if system == "Windows":
			slext = "dll"
		elif system == "Linux":
			slext = "so"
		else:
			print("Must run on windows or Linux system\n")
			self.isValid = False
			return
		libname = "ImageMod." + slext
		baseDir = pathlib.Path(__file__).parent.absolute()
		if path is None: path = os.path.join(os.path.join(os.path.join(baseDir, "SL"), "ImageMod"), libname)
		path2 = os.path.join(baseDir, libname)
		#try to load the shared library from the either path
		self.isValid = False
		try:
			self.SL = CDLL(path)
			self.isValid = True
		except Exception as e:
			print("Library not found at " + path + "\nLooking in " + path2)
			self.SL = CDLL(path2)
			self.isValid = True
		#if the shared library was successfully loaded, we specify the return types and arguments for the various functions
		if self.isValid:
			#load the color reduce function
			self.colorReduce = self.SL.ColorReduce
			self.colorReduce.argtypes = [POINTER(c_ubyte), c_int, c_int, c_int]
			#load the split color function
			self.splitColor = self.SL.SplitColor
			self.splitColor.argtypes = [POINTER(c_ubyte), c_int, c_int, POINTER(c_ubyte), POINTER(c_int), c_int, c_int]
			#load the color replace function
			self.colorReplace = self.SL.ColorReplace
			self.colorReplace.argtypes = [POINTER(c_ubyte), c_int, c_int, c_int, POINTER(c_ubyte), POINTER(c_ubyte)]
			#load the fill square color function
			self.fillSquareColor = self.SL.FillSquareColor
			self.fillSquareColor.argtypes = [POINTER(c_ubyte), c_int, c_int, c_int, c_int, c_int, c_int, c_int, POINTER(c_ubyte)]
			self.fillSquareColor.restype = c_int
			#load the build padded image function
			self.padImage = self.SL.PadImage
			self.padImage.argtypes = [POINTER(c_ubyte), POINTER(c_ubyte), c_int, c_int, c_int, c_int, POINTER(c_ubyte)]
			#load the crop image function
			self.cropImage = self.SL.CropImage
			self.cropImage.argtypes = [POINTER(c_ubyte), POINTER(c_ubyte), POINTER(c_uint32), c_int, c_int]
			#load the erase long segments function
			self.eraseLongSegments = self.SL.EraseLongSegments
			self.eraseLongSegments.argtypes = [POINTER(c_ubyte), c_int, c_int, c_int, c_int, c_int, POINTER(c_ubyte)]
			#load the remove empty lines function
			self.removeEmptyLines = self.SL.RemoveEmptyLines
			self.removeEmptyLines.argtypes = [POINTER(c_ubyte), POINTER(c_ubyte), c_int, c_int, c_int, c_int, POINTER(c_ubyte)]
			self.removeEmptyLines.restype = c_int
			#load the pixel match function
			self.pixelMatch = self.SL.PixelMatch
			self.pixelMatch.argtypes = [POINTER(c_ubyte), POINTER(c_ubyte), c_int, c_int, c_int, c_int, c_int, c_int, c_int]
			self.pixelMatch.restype = c_float
			#load the get image position function
			self.getImagePosition = self.SL.GetImagePosition
			self.getImagePosition.argtypes = [POINTER(c_ubyte), POINTER(c_ubyte), POINTER(c_int), c_int, c_int, c_int, c_int, c_int, c_int, c_int, c_float, c_int, c_int]
			self.getImagePosition.restype = c_int
			#load the relevant rectangle function
			self.getRelevantRectangle = self.SL.GetRelevantRectangle
			self.getRelevantRectangle.argtypes = [POINTER(c_ubyte), POINTER(c_uint32), c_int, c_int, c_int, POINTER(c_ubyte)]
			#load the get element list function
			self.getElementList = self.SL.GetElementList
			self.getElementList.argtypes = [POINTER(c_ubyte), POINTER(c_int), c_int, c_int, c_int, POINTER(c_ubyte), c_int, c_int]
			self.getElementList.restype = c_int
