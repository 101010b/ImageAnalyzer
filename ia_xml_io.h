#pragma once

#include <math.h>
#include "vectormatrix.h"

namespace image_add {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Collections::Generic;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::Drawing::Imaging;
	using namespace System::Xml;
	using namespace System::IO;


	public ref class ia_xml_write : public XmlTextWriter
	{
	public:
		ia_xml_write(String ^fn);

		void write_value(String ^name, double d);
		void write_value(String ^name, int d);
		void write_value(String ^name, String ^v);
		void write_value(String ^name, vector v);
		void write_value(String ^name, Color c);
		void write_value(String ^name, Font ^f);
		void write_dimension(String ^name, int w, int h);

	};

	public ref class ia_xml_read : public XmlTextReader
	{
	public:
		ia_xml_read(String ^fn);

		Font ^xml_read_font();
		vector xml_read_vector();
		Color xml_read_color();
		void read_xml_dimension(int *w, int *h);

	};


};

