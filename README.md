# ImageAnalyzer
A simple tool to analyze and/or annotate images

It can be used to
* Measure lengths or other geometric features in an image file, e.g. a microscopic photo, a SEM photo... 
* Extract Data from a bitmap or a scanned figure

Well known solutions for the same purpose
* Engauge Digitizer: http://markummitchell.github.io/engauge-digitizer/
* ImageJ: https://imagej.net/Welcome

Features
* Import images or insert them from the clipboard easily
* Calibrate them using a 2-reference-point or rectangular/arbitrary 3-reference-point coordinate system
* Measure
	* lengths (lines + polygons)
	* angles
	* areas
	* radius
	* point coordinates
* Analyze a color profile colors along a cut line
	* gray value
	* colors
	* extract values as defined by a colorbar somewhere else in the image
* Export measured values to a CSV file or directly to the clipboard, e.g. for further processing tools
* Export Image including measurements as WindowsMetaFile or bitmap

This project uses submodules/libraries which will be required for compilation:
* libjpeg for processing and loading jpeg files, sourced from https://github.com/winlibs/libjpeg
* zlib for lossless compression of image files and other data, sourced from https://github.com/madler/zlib

To clone them together with this repository, use 

git clone --recurse-submodules https://github.com/101010b/ImageAnalyzer.git

Compilation works fine with VisualStudio 2017, Community Edition
