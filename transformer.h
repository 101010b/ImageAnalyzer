#pragma once

#include <math.h>
#include "vectormatrix.h"

namespace image_add {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::Drawing::Imaging;
	
	public enum class AngleMode {
		Radians,		// Display in rad
		Degrees			// Display in degree
	};
	
	public enum class ColorMode {
		RGBByte,
		RGBFloat,
		HTML
	};

	public ref class formatter {
	public:
		String ^form;
		String ^unit;
		AngleMode amode;		

		String ^cform;
		String ^cunit;
		ColorMode cmode;

		formatter() { form="F3"; unit=""; cform="F3"; cunit=""; amode=AngleMode::Degrees; };
		~formatter() { delete form; delete unit; delete cform; delete cunit; };

		void setformat(String ^s) { form=gcnew String(s); }
		void setunit(String ^s) { unit=gcnew String(s); }
		void setangle(AngleMode am) { amode=am; }
		void setcformat(String ^s) { cform=gcnew String(s); }
		void setcunit(String ^s) { cunit=gcnew String(s); }
		void setcmode(ColorMode cm) { cmode=cm; }

		String ^format(double v);
		String ^format(vector v);
		String ^format_nounit(double v);
		String ^format_nounit(vector v);
		String ^format_angle(double v);
		String ^format_area(double v);

		String ^cformat(double v);
		String ^cformat_nounit(double v);

		String ^colorformat(double cr,double cg, double cb);
		String ^colorformat(int cr,int cg, int cb);
	};

	public ref class transformer {
	public:
		double scale;
		vector screen;
		vector center;

		transformer(void);
		transformer(vector sc);

		~transformer(void);

		void resize(vector sc);
		void rescale(double sc);
		void recenter(vector sc);

		vector todata(vector s);
		vector toscreen(vector d);
	};

}