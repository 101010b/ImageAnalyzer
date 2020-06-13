#pragma once

#include "vectormatrix.h"

namespace image_add {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::Drawing::Imaging;
	using namespace System::Xml;
	using namespace System::IO;

	public ref class imagedata {
	public:

		String ^filename;
		int width,height;
		array<Byte> ^data;
		Bitmap ^bmap;

		imagedata(void);
		imagedata(int i); // From Clipboard or other sources
		imagedata(String ^fn);

		~imagedata(void);

		void read_using_windows(String ^fn);
		void read_from_jpeg(String ^fn);

		array<Byte> ^compressZ();
		int decompressZ(array<Byte> ^src);
		array<Byte> ^compressJPG();
		int decompressJPG(array<Byte> ^src);
		void write_to_xml(XmlTextWriter ^x);
		void read_from_xml(XmlTextReader ^x);

		int getcolor(int x, int y, int *r, int *g, int *b);



		void write_to_picmap(array<Byte> ^d, int sw, int BWidth, int Bheight,
			double minx, double miny, double maxx, double maxy, int centercross, int expand);
		void write_zoom(Graphics ^g, double minx, double miny, double maxx, double maxy, 
			int centercross, int expand);
		void write_to_graphics(Graphics ^g, double minx, double miny, double maxx, double maxy, 
			int centercross, int expand);
	};

	public ref class profile {
	public:
		vector start;
		vector stop;
		int pts;
		array<double> ^X;
		array<double> ^R;
		array<double> ^G;
		array<double> ^B;
		imagedata ^img;

		profile(imagedata ^id, vector s, vector e);
		~profile();

		void calcprofile();
		void shift(vector news, vector newe);

		double findcolor(double r, double g, double b);

	};


}
