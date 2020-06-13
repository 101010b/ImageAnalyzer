# ImageAnalyzer
A simple tool to analyze images geometrically

WORK IN PROGRESS!!! CONVERTING FROM ANOTHER REPOSITORY SYSTEM TO GITHUB!!! WILL NOT WORK YET

Features
* Import images or insert them from the clipboard
* Calibrate them using 2-point or rectangular/arbitrarty 3-point coordinate system
* Measure
	* lengths (lines + polygons)
	* angles
	* areas
	* radius
	* point coordinates
* Analyze Colors along a profile cut
	* gray value
	* colors
	* value matching by an arbitrary colorbar
* Export measured values to CSV file or directly to the clipboard, e.g. for further processing in Excel or similar tools
* Export Bitmap Image including measurements

This Project contains submodules which will be required for compiulation
* libjpeg for processing and loading jpeg files from  https://github.com/winlibs/libjpeg.git
* zlib for lossless compression of image files and other data from https://github.com/madler/zlib.git

To clone them together with this repository, use 

git clone --recurse-submodules https://github.com/101010b/ImageAnalyzer.git


Compilation works fine with VisualStudio 2017, Community Edition

