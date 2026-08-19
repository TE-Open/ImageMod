import os
import pathlib
import platform

from ctypes import Structure, c_int, c_ubyte, c_float, POINTER, CDLL

class ImageData(Structure):
	#this class represents an ImageData structure in the shared library
	_fields_ = [("bA", POINTER(c_ubyte)), ("width", c_int), ("height", c_int), ("hasAlpha", c_int)]

	def __repr__(self):
		return f"{self.__class__.__name__}(width={self.width}, height={self.height}, hasAlpha={self.hasAlpha})"

	def getByteCount(self):
		#this function returns the total byte count for the current ImageData object
		return (self.width * self.height * (3 + self.hasAlpha))

class Area(Structure):
	#this class represents an Area structure in the shared library
	_fields_ = [("top", c_int), ("bottom", c_int), ("left", c_int), ("right", c_int)]

	def __repr__(self):
		return f"{self.__class__.__name__}(top={self.top}, bottom={self.bottom}, left={self.left}, right={self.right})"

	def getOffset(self, x, y):
		#this function returns an area that is a copy of itself offset by the given horizontal and vertical distances (x and y)
		return Area(top = self.top + y, bottom = self.bottom + y, left = self.left + x, right = self.right + x)

class MatchData(Structure):
	#this class represents an MatchData structure in the shared library
	_fields_ = [("a", Area), ("matchP", c_float)]

	def __repr__(self):
		return f"{self.__class__.__name__}(top={self.a.top}, bottom={self.a.bottom}, left={self.a.left}, right={self.a.right}, matchP={self.matchP})"

	def getArrayIP(imgDSml, imgDBig):
		#this function returns an array of MatchData objects large enough to use in the ImageMod library GetImagePosition function given the supplied ImageData objects
		return (MatchData * (3 * (1 + int(imgDBig.width / imgDSml.width)) * (1 + int(imgDBig.height / imgDSml.height))))()

class ColorItem(Structure):
	#this class represents an ColorItem structure in the shared library
	_fields_ = [("red", c_ubyte), ("green", c_ubyte), ("blue", c_ubyte), ("alpha", c_ubyte)]

	def __repr__(self):
		return f"{self.__class__.__name__}(red={self.red}, green={self.green}, blue={self.blue}, alpha={self.alpha})"

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
			self.colorReduce.argtypes = [POINTER(ImageData), c_int]
			self.colorReduce.restype = None
			#load the split color function
			self.splitColor = self.SL.SplitColor
			self.splitColor.argtypes = [POINTER(ImageData), POINTER(ColorItem), POINTER(c_int), c_int, c_int]
			self.splitColor.restype = None
			#load the color replace function
			self.colorReplace = self.SL.ColorReplace
			self.colorReplace.argtypes = [POINTER(ImageData), c_int, POINTER(ColorItem), POINTER(ColorItem)]
			self.colorReplace.restype = None
			#load the fill square color function
			self.fillSquareColor = self.SL.FillSquareColor
			self.fillSquareColor.argtypes = [POINTER(ImageData), c_int, c_int, c_int, c_int, POINTER(ColorItem)]
			self.fillSquareColor.restype = c_int
			#load the pad image function
			self.padImage = self.SL.PadImage
			self.padImage.argtypes = [POINTER(ImageData), POINTER(ImageData), c_int, POINTER(ColorItem)]
			self.padImage.restype = None
			#load the copy area function
			self.copyArea = self.SL.CopyArea
			self.copyArea.argtypes = [POINTER(ImageData), POINTER(ImageData), POINTER(Area)]
			self.copyArea.restype = None
			#load the erase segments function
			self.eraseSegments = self.SL.EraseSegments
			self.eraseSegments.argtypes = [POINTER(ImageData), c_int, c_int, POINTER(ColorItem)]
			self.eraseSegments.restype = None
			#load the remove empty lines function
			self.removeEmptyLines = self.SL.RemoveEmptyLines
			self.removeEmptyLines.argtypes = [POINTER(ImageData), POINTER(ImageData), c_int, POINTER(ColorItem)]
			self.removeEmptyLines.restype = None
			#load the pixel match function
			self.pixelMatch = self.SL.PixelMatch
			self.pixelMatch.argtypes = [POINTER(ImageData), POINTER(ImageData), c_int, c_float]
			self.pixelMatch.restype = c_float
			#load the get image position function
			self.getImagePosition = self.SL.GetImagePosition
			self.getImagePosition.argtypes = [POINTER(ImageData), POINTER(ImageData), POINTER(MatchData), c_int, c_float, c_int, c_int, c_int]
			self.getImagePosition.restype = c_int
			#load the get image colors function
			self.getImageColors = self.SL.GetImageColors
			self.getImageColors.argtypes = [POINTER(ImageData), POINTER(ColorItem), c_int]
			self.getImageColors.restype = c_int
			#load the check color presence function
			self.checkColorPresence = self.SL.CheckColorPresence
			self.checkColorPresence.argtypes = [POINTER(ImageData), POINTER(ColorItem), c_int, c_int]
			self.checkColorPresence.restype = c_int
			#load the relevant area function
			self.getRelevantArea = self.SL.GetRelevantArea
			self.getRelevantArea.argtypes = [POINTER(ImageData), POINTER(Area), POINTER(ColorItem)]
			self.getRelevantArea.restype = None
			#load the get element list function
			self.getElementList = self.SL.GetElementList
			self.getElementList.argtypes = [POINTER(ImageData), POINTER(Area), POINTER(ColorItem), c_int, c_int]
			self.getElementList.restype = c_int
