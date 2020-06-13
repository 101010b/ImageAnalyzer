#pragma once

#include <math.h>
#include "vectormatrix.h"
#include "transformer.h"
#include "ia_xml_io.h"
#include "imagedata.h"

#ifndef M_PI
	#define M_PI       3.14159265358979323846
#endif


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

	//void write_double(XmlTextWriter ^x,String ^name,double d);
	//void write_int(XmlTextWriter ^x,String ^name,int d);
	//void write_string(XmlTextWriter ^x,String ^name,String ^v);
	//void write_vector(XmlTextWriter ^x,String ^name, vector v);
	//void write_color(XmlTextWriter ^x,String ^name,Color c);

	// predefine
	ref class coordinate;
	ref class cframe;
	ref class ccolorbar;

	// Root Sys
	public ref class csys {
	public:
		formatter ^f;
		transformer ^t;
		coordinate ^c;
		cframe ^frm;
		ccolorbar ^cbar;
		imagedata ^img;
		csys(formatter ^fmt, transformer ^trf);
		csys(formatter ^fmt, transformer ^trf, coordinate ^cs, cframe ^f, ccolorbar ^cb);

		vector image_from_data(vector v);
		vector data_from_image(vector v);
		vector screen_from_real(vector v);
		vector screen_from_data(vector v);
		vector data_from_real(vector v);
		vector data_from_screen(vector v);
		vector real_from_data(vector v);
		vector real_from_screen(vector v);
	};

	[DefaultPropertyAttribute("General")]

	// base class for all data within the drawing
	public ref class cmeasurement {
	public:
		String ^name;
		String ^type;
		int id;

		csys ^c;

		//List <cmeasurement^> ^childs; // independant child objects

		int selected;
		cmeasurement ^root;

		Font ^fnt;
		Color fntcol;
		Color fntfrm;
		int fntfrmuse;
		Color linecol;

		int readonly;

		// Constructors/Destructors
		void cmeasurement_default();
		cmeasurement(String ^mtype, csys ^cs);
		cmeasurement(String ^mtype, String ^mname, csys ^cs);
		cmeasurement(String ^mtype, csys ^cs, cmeasurement ^rt);
		~cmeasurement(void);
		virtual void update_from_template(cmeasurement ^templ);
		
		// Draw Helper Functions
		void drawtxtbox(Graphics ^g, double minx, double miny, double maxx, double maxy, int ax, int ay, String ^txt);
		void drawtxt(Graphics ^g, vector pos, vector along, int ax, int ay, String ^txt);
		void drawtickline(Graphics ^g, vector AR, vector BR, Pen ^pen, int showticks, String ^txt);
		int findsepoints(Graphics ^g, vector A, vector B, double *s, double *e);
		void drawline(Graphics ^g, vector A, vector B, Pen ^p);
		void drawinfline(Graphics ^g, vector A, vector B, Pen ^p);
		double getlinedist(vector A, vector B, vector S);
		double getinlinedist(vector A, vector B, vector S);
		double find10step(double l,double maxw);


//		[CategoryAttribute("General")]
//		// Properties
//		virtual property String ^Type {
//			String ^get(void) { return gcnew String(type); }
//			// void set(String ^s) {}
//		};
		[CategoryAttribute("General")]
		virtual property String ^Name { 
			String ^get(void) { return gcnew String(name); }
			void set(String ^s) { name=gcnew String(s); }
		};
		[CategoryAttribute("General")]
		virtual property String ^Description {
			String ^get(void) { return getdesc(); }
			//void set(String ^s) {}
		};

		[CategoryAttribute("Appearance")]
		property Font ^DisplayFont {
			Font ^get(void) { return fnt; }
			void set(Font ^f) { delete fnt; fnt=f; }
		}
		[CategoryAttribute("Appearance")]
		property Color DisplayFontColor {
			Color get(void) { return fntcol; }
			void set(Color f) { fntcol=f; }
		}
		[CategoryAttribute("Appearance")]
		property Color DisplayFontFrameColor {
			Color get(void) { return fntfrm; }
			void set(Color f) { fntfrm=f; }
		}
		[CategoryAttribute("Appearance")]
		property bool DisplayFontFrame {
			bool get(void) { return (fntfrmuse)?true:false; }
			void set(bool f) { fntfrmuse=(f)?1:0; }
		}
		[CategoryAttribute("Appearance")]
		property Color LineColor {
			Color get(void) { return linecol; }
			void set(Color f) { linecol=f; }
		}


	// Property Helper
	virtual String ^getdesc(void);

	// Text Exporter
	virtual String ^exportCSV(void);

		// Public interface
	public:
		virtual void draw(Graphics ^g);
		virtual void select(void);
		virtual void unselect(void);
		
		// Checks whether a click hits the object or part of it
		// returns a list if the touched objects or a nullptr
		virtual List<cmeasurement^>^ touching(vector datai);

		// Checks whether the object is inside a rectangle defined by the two screen vectors
		virtual List<cmeasurement^>^ isinside(vector data1, vector data2);

		// Object was dragged
		virtual void moveto(vector data_from, vector data_to);

		virtual void storestate();
		virtual void restorestate();

		// Callback
		virtual void refchangenotify(cmeasurement ^c);

		virtual void write_to_xml(ia_xml_write ^x);
		virtual int read_from_xml(ia_xml_read ^x);

	};

	public enum class FrameStyles {
		None,
		Single,
		Double,
		DoubleWithSections
	};

	public enum class FieldStyles {
		None,
		Top,
		Bottom
	};

	public ref class cframe : cmeasurement {
	public:
		FrameStyles frms;
		int frmw;

		FieldStyles flds;

		int fullpath;
		int showimgfileinfo;
		//int calinfo;
		int showcmt;
		int showimg;
		int showcal;
		int showfn;
		int showscale;
		int showcoord;

		Color BackCol;
		double alpha;

		String ^comment;
		String ^imgfilename;
		String ^imgfileinfo;
		String ^filename;
		String ^calname;
		
		void cframe_default();
		cframe(csys ^c, ia_xml_read ^x);
		cframe(csys ^c);

		virtual String ^getdesc(void) override;
		virtual String ^exportCSV(void) override;

	public:
	[CategoryAttribute("General")]
	property FrameStyles FrameStyle {
		FrameStyles get(void) { return frms; }
		void set(FrameStyles s) { frms = s; }
	};

	[CategoryAttribute("General")]
	property FieldStyles FieldStyle {
		FieldStyles get(void) { return flds; }
		void set(FieldStyles s) { flds = s; }
	};
		
	[CategoryAttribute("General")]
	property int FrameWidth {
		int get(void) { return frmw; }
		void set(int f) { if (f < 10) f=10; if (f > 100) f=100; frmw=f; }
	}

	[CategoryAttribute("Display")]
	property String ^Comment {
		String^ get(void) { return gcnew String(comment); }
		void set(String ^s) { comment=gcnew String(s); }
	}
	[CategoryAttribute("Display")]
	property bool ShowComment {
		bool get(void) { return (showcmt)?true:false; }
		void set(bool b) { showcmt=(b)?1:0; }
	};
	[CategoryAttribute("Display")]
	property bool ShowImageFileName {
		bool get(void) { return (showimg)?true:false; }
		void set(bool b) { showimg=(b)?1:0; }
	};
	[CategoryAttribute("Display")]
	property bool ShowFileName {
		bool get(void) { return (showfn)?true:false; }
		void set(bool b) { showfn=(b)?1:0; }
	};
	[CategoryAttribute("Display")]
	property bool ShowCalibrationName {
		bool get(void) { return (showcal)?true:false; }
		void set(bool b) { showcal=(b)?1:0; }
	};
	[CategoryAttribute("Display")]
	property bool ShowImageInfo {
		bool get(void) { return (showimgfileinfo)?true:false; }
		void set(bool b) { showimgfileinfo=(b)?1:0; }
	};
	[CategoryAttribute("Display")]
	property bool ShowCornerCoordinates {
		bool get(void) { return (showcoord)?true:false; }
		void set(bool b) { showcoord=(b)?1:0; }
	};
	[CategoryAttribute("Display")]
	property bool ShowScaleBar {
		bool get(void) { return (showscale)?true:false; }
		void set(bool b) { showscale=(b)?1:0; }
	};


	[CategoryAttribute("Appearance")]
	property Color BackgroundColor {
		Color get(void) { return BackCol; }
		void set(Color sc) { BackCol=sc; }
	};
	[CategoryAttribute("Appearance")]
	property int Transparency {
		int get(void) { return (int)floor(alpha*100+0.5); }
		void set(int alp) { if (alp<0) alp=0; if (alp>100) alp=100;alpha=(double)alp/100.0; }
	};


	public:
		virtual void draw(Graphics ^g) override;

		virtual List<cmeasurement^>^ touching(vector datai) override;
		virtual List<cmeasurement^>^ isinside(vector data1, vector data2) override;
		virtual void moveto(vector data_from, vector data_to) override;
		virtual void storestate() override;
		virtual void restorestate() override;

		virtual void write_to_xml(ia_xml_write ^x) override;
		virtual int read_from_xml(ia_xml_read ^x) override;

	};


	public enum class PointStyles {
		None,
		Dot,
		Plus,
		LargePlus,
		X,
		LargeX,
		Circle,
		LargeCircle,
		Square,
		LargeSquare
	};


	public ref class crefpoint : cmeasurement {
	public:
		vector data;
		vector store_data;
		//Color PointCol;
		cmeasurement ^callback;
		PointStyles pts;

		void crefpoint_default();
		crefpoint(csys ^c, ia_xml_read ^x);
		crefpoint(vector dtv, csys ^c);

		void setcallback(cmeasurement ^cbk);

//	[CategoryAttribute("Appearance")]
//	property Color PointColor {
//		Color get(void) { return PointCol; }
//		void set(Color sc) { PointCol=sc; }
//	};

	virtual String ^getdesc(void) override;

	public:
		virtual void draw(Graphics ^g) override;

		virtual List<cmeasurement^>^ touching(vector datai) override;
		virtual List<cmeasurement^>^ isinside(vector data1, vector data2) override;
		virtual void moveto(vector data_from, vector data_to) override;
		virtual void storestate() override;
		virtual void restorestate() override;

		virtual void write_to_xml(ia_xml_write ^x) override;
		virtual int read_from_xml(ia_xml_read ^x) override;

	};


	public enum class HAlign {
		Left,
		Center,
		Right
	};

	public enum class VAlign {
		Top,
		Center,
		Bottom
	};

	public enum class Shape {
		Line,
		Arrow,
		Rectangle,
		RoundedRectangle,
		Ellipse
	};

	public enum class LineMode {
		None,
		Solid,
		Dash,
		DashDot,
		Dot
	};

	public ref class cannotation : cmeasurement {
	public:
		crefpoint ^p1;
		crefpoint ^p2;

		Color bgcol;
		double bgalp;
		int bgfill;
		
		LineMode lm;
		double lw;

		HAlign ha;
		VAlign va;
		Shape shp;

		int textonline;

		double rratio;
		
		String ^text;
				
		void cannotation_default();
		cannotation(csys ^c, ia_xml_read ^x);
		cannotation(vector pt1, vector pt2, csys ^c);
		virtual void update_from_template(cmeasurement ^templ) override;

	[CategoryAttribute("Appearance")]
	property Color BackgroundColor {
		Color get(void) { return bgcol; }
		void set(Color sc) { bgcol=sc; }
	};
	[CategoryAttribute("Appearance")]
	property int BackgroundTransparency {
		int get(void) { return (int)floor(bgalp*100+0.5); }
		void set(int sc) { if (sc < 0) bgalp=0; else if (sc > 100) bgalp=1; else bgalp=(double)sc / 100.0; }
	};
	[CategoryAttribute("Appearance")]
	property bool Fill {
		bool get(void) { return (bgfill)?true:false; }
		void set(bool sc) { bgfill=(sc)?1:0; }
	};
	[CategoryAttribute("Appearance")]
	property LineMode LineStyle {
		LineMode get(void) { return lm; }
		void set(LineMode sc) { lm=sc; }
	};
	[CategoryAttribute("Appearance")]
	property double LineWidth {
		double get(void) { return lw; }
		void set(double sc) { lw=sc; }
	};
	[CategoryAttribute("Appearance")]
	property Shape FrameShape {
		Shape get(void) { return shp; }
		void set(Shape sc) { shp=sc; }
	};
	[CategoryAttribute("Appearance")]
	property int RectangleRounding {
		int get(void) { return floor(rratio*100+0.5); }
		void set(int sc) { if (sc <= 0) rratio=0; else if (sc >= 100) rratio=1; else rratio=(double)sc/100.0; }
	};

	[CategoryAttribute("Text")]
	property HAlign HorizontalAlign {
		HAlign get(void) { return ha; }
		void set(HAlign sc) { ha=sc; }
	};
	[CategoryAttribute("Text")]
	property VAlign VerticalAlign {
		VAlign get(void) { return va; }
		void set(VAlign sc) { va=sc; }
	};
	[CategoryAttribute("Text")]
	property bool TextOnLine {
		bool get(void) { return (textonline)?true:false; }
		void set(bool sc) { textonline=(sc)?1:0; }
	};
	[CategoryAttribute("Text")]
	property String ^Text {
		String ^get(void) { return gcnew String(text); }
		void set(String ^sc) { text=gcnew String(sc); }
	};

	virtual String ^getdesc(void) override;

	public:
		virtual void draw(Graphics ^g) override;

		virtual List<cmeasurement^>^ touching(vector datai) override;
		virtual List<cmeasurement^>^ isinside(vector data1, vector data2) override;
		virtual void moveto(vector data_from, vector data_to) override;
		virtual void storestate() override;
		virtual void restorestate() override;

		virtual void write_to_xml(ia_xml_write ^x) override;
		virtual int read_from_xml(ia_xml_read ^x) override;

	};


	public enum class PointDisplayMode {
		PointOnly,
		Coordinates,
		Color,
		ColorValue,
		CoordinatesAndColor,
		CoordinatesAndColorValue };

	public ref class cpoint : cmeasurement {
	public:
		int isref; 
		coordinate ^refsys;
		// isref = 0: Just a passive point --> real will be calculated

		PointDisplayMode dmode;
		// int showcoord;
		//Color PointCol;
		//Font ^fnt;
		PointStyles pts;
		
		vector data;
		vector real;

		vector store_data;

		void cpoint_default(); 
		cpoint(csys ^c, ia_xml_read ^x);
		cpoint(vector dtv, csys ^c);
		cpoint(vector dtv, vector re, csys ^c, coordinate ^cr);
		virtual void update_from_template(cmeasurement ^templ) override;

		void update_data(void);

	[CategoryAttribute("Position")]
	property double X {
		double get() { return real.x; }
		void set(double x) { real.x=x; update_data(); }
	};

	[CategoryAttribute("Position")]
	property double Y {
		double get() { return real.y; }
		void set(double y) { real.y=y; update_data(); }
	};

	[CategoryAttribute("Position")]
	property double Radius { double get() { return real.len; } };

	[CategoryAttribute("Position")]
	property double Phi { double get() { return atan2(real.y,real.x)*180/M_PI; } };



	[CategoryAttribute("Appearance")]
	property PointStyles PointStyle {
		PointStyles get(void) { return pts; }
		void set(PointStyles s) { pts = s; }
	};

	[CategoryAttribute("Appearance")]
	property PointDisplayMode DisplayMode {
		PointDisplayMode get(void) { return dmode; }
		void set(PointDisplayMode b) { dmode=b; }
	};

	virtual String ^getdesc(void) override;
	virtual String ^exportCSV(void) override;

	public:
		virtual void draw(Graphics ^g) override;

		virtual List<cmeasurement^>^ touching(vector datai) override;
		virtual List<cmeasurement^>^ isinside(vector data1, vector data2) override;
		virtual void moveto(vector data_from, vector data_to) override;
		virtual void storestate() override;
		virtual void restorestate() override;

		virtual void write_to_xml(ia_xml_write ^x) override;
		virtual int read_from_xml(ia_xml_read ^x) override;

	};

	public ref class clinear : cmeasurement {
	public:
		cpoint ^p1;
		cpoint ^p2;
		double offset;

		crefpoint ^pp;

		int drawrefpoints;

//		Color FillBGCol;
//		int fillbackground;
		VAlign va;

		int showticks;

		void clinear_default();
		clinear(csys ^c, ia_xml_read ^x);
		clinear(vector data1, vector data2, csys ^cs);
		virtual void update_from_template(cmeasurement ^templ) override;

	[CategoryAttribute("Position")]
	property double StartX { double get() { return p1->real.x; };}
	[CategoryAttribute("Position")]
	property double StartY { double get() { return p1->real.y; };}
	[CategoryAttribute("Position")]
	property double EndX { double get() { return p2->real.x; };}
	[CategoryAttribute("Position")]
	property double EndY { double get() { return p2->real.y; };}
	[CategoryAttribute("Delta")]
	property double DeltaX { double get() { return p2->real.x-p1->real.x; };}
	[CategoryAttribute("Delta")]
	property double DeltaY { double get() { return p2->real.y-p1->real.y; };}
	[CategoryAttribute("Delta")]
	property double Length { double get() { vector v=p1->real-p2->real; return v.len; };}
	[CategoryAttribute("Angle")]
	property double Phi { double get() { return atan2(p2->real.y-p1->real.y,p2->real.x-p1->real.x)*180/M_PI; };}
	[CategoryAttribute("Line Fit Y=m*X+a")]
	property double LineFitY_m { double get() { return (p2->real.y-p1->real.y)/(p2->real.x-p1->real.x); };}
	[CategoryAttribute("Line Fit Y=m*X+a")]
	property double LineFitY_a { double get() { return p1->real.y-(p2->real.y-p1->real.y)/(p2->real.x-p1->real.x)*p1->real.x; };}
	[CategoryAttribute("Line Fit X=n*Y+b")]
	property double LineFitX_n { double get() { return (p2->real.x-p1->real.x)/(p2->real.y-p1->real.y); };}
	[CategoryAttribute("Line Fit X=n*Y+b")]
	property double LineFitX_b { double get() { return p1->real.x-(p2->real.x-p1->real.x)/(p2->real.y-p1->real.y)*p1->real.y; };}


	[CategoryAttribute("Appearance")]
	property bool ShowTicks {
		bool get() { return (showticks)?true:false; };
		void set(bool sr) { showticks=(sr)?1:0; }
	}
	[CategoryAttribute("Appearance")]
	property VAlign VerticalAlignment {
		VAlign get() { return va; };
		void set(VAlign sr) { va=sr; }
	}



	virtual String ^getdesc(void) override;
	virtual String ^exportCSV(void) override;

	public:
		virtual void draw(Graphics ^g) override;
		virtual List<cmeasurement^>^ touching(vector datai) override;
		virtual List<cmeasurement^>^ isinside(vector data1, vector data2) override;
		virtual void moveto(vector data_from, vector data_to) override;
		virtual void storestate() override;
		virtual void restorestate() override;
		virtual void refchangenotify(cmeasurement ^csrc) override;
		void calc_ref_point();

		virtual void select(void) override;
		virtual void unselect(void) override;

		virtual void write_to_xml(ia_xml_write ^x) override;
		virtual int read_from_xml(ia_xml_read ^x) override;

	};



	public enum class ColorBarMode {
		Standard
	};

	public ref class ccolorbar : cmeasurement {
	public:
		cpoint ^p1;
		cpoint ^p2;
		double o1, v1;
		double o2, v2;
		ColorBarMode cbmode;
		crefpoint ^cp1;
		crefpoint ^cp2;
		int show;


		profile ^prf;
		double filtsize;

		void ccolorbar_default();
		ccolorbar(csys ^c, ia_xml_read ^x);
		ccolorbar(vector data1, vector data2, csys ^cs);
		virtual void update_from_template(cmeasurement ^templ) override;

	[CategoryAttribute("Position")]
	property double StartX { double get() { return p1->real.x; };}
	[CategoryAttribute("Position")]
	property double StartY { double get() { return p1->real.y; };}
	[CategoryAttribute("Position")]
	property double EndX { double get() { return p2->real.x; };}
	[CategoryAttribute("Position")]
	property double EndY { double get() { return p2->real.y; };}
	[CategoryAttribute("Delta")]
	property double DeltaX { double get() { return p2->real.x-p1->real.x; };}
	[CategoryAttribute("Delta")]
	property double DeltaY { double get() { return p2->real.y-p1->real.y; };}
	[CategoryAttribute("Delta")]
	property double Length { double get() { vector v=p1->real-p2->real; return v.len; };}
	[CategoryAttribute("Angle")]
	property double Phi { double get() { return atan2(p2->real.y-p1->real.y,p2->real.x-p1->real.x)*180/M_PI; };}
	[CategoryAttribute("Line Fit Y=m*X+a")]
	property double LineFitY_m { double get() { return (p2->real.y-p1->real.y)/(p2->real.x-p1->real.x); };}
	[CategoryAttribute("Line Fit Y=m*X+a")]
	property double LineFitY_a { double get() { return p1->real.y-(p2->real.y-p1->real.y)/(p2->real.x-p1->real.x)*p1->real.x; };}
	[CategoryAttribute("Line Fit X=n*Y+b")]
	property double LineFitX_n { double get() { return (p2->real.x-p1->real.x)/(p2->real.y-p1->real.y); };}
	[CategoryAttribute("Line Fit X=n*Y+b")]
	property double LineFitX_b { double get() { return p1->real.x-(p2->real.x-p1->real.x)/(p2->real.y-p1->real.y)*p1->real.y; };}

	[CategoryAttribute("CoolorBar")]
	property double Offset1 { double get() { return o1; };}
	[CategoryAttribute("CoolorBar")]
	property double Offset2 { double get() { return o2; };}
	[CategoryAttribute("CoolorBar")]
	property double Value1 { double get() { return v1; }; void set(double v) { v1=v; } }
	[CategoryAttribute("CoolorBar")]
	property double Value2 { double get() { return v2; }; void set(double v) { v2=v; } }

	[CategoryAttribute("Appearance")]
	property ColorBarMode Mode  {
		ColorBarMode get(void) { return cbmode; }
		void set(ColorBarMode b) { cbmode=b; }
	};

	virtual String ^getdesc(void) override;
	virtual String ^exportCSV(void) override;

	public:
		virtual void draw(Graphics ^g) override;
		virtual List<cmeasurement^>^ touching(vector datai) override;
		virtual List<cmeasurement^>^ isinside(vector data1, vector data2) override;
		virtual void moveto(vector data_from, vector data_to) override;
		virtual void storestate() override;
		virtual void restorestate() override;
		virtual void refchangenotify(cmeasurement ^csrc) override;
		//void calc_ref_point();
		
		void calcprofile();
		void filterprofile(array<double> ^e);

		//array<double> ^getprofile();
		virtual void select(void) override;
		virtual void unselect(void) override;

		virtual void write_to_xml(ia_xml_write ^x) override;
		virtual int read_from_xml(ia_xml_read ^x) override;

	};

	public enum class ProfileSource {
		WeightedRGBSum,
		Red,
		Green,
		Blue,
		Cyan,
		Magenta,
		Yellow,
		Hue,
		Saturation,
		Lightness,
		ColorVal
	};

	public enum class ProfileFilter {
		Off,
		Derivative,
		HighPass,
		LowPass,
		Median,
		Integrated
	};

	public enum class ProfilePost {
		Linear,
		Log,
		Normalize
	};

	public enum class ProfileCursors {
		None,
		ManualX,
		ManualY,
		ManualYCoupled,
		Threshold,
		LocalMax,
		LocalMin
	};

	public ref class cprofile : cmeasurement {
	public:
		cpoint ^p1;
		cpoint ^p2;
		double offset;

		crefpoint ^pp;
		crefpoint ^cp1;
		crefpoint ^cp2;

		VAlign va;

		int showprofile;
		int showlength;
		int showrgb;
		double scaler,scaleg,scaleb;
		int fillprofile;
		Color prcolor;
		Color prfillcol;
		double pralpha;

		ProfileSource prsrc;
		ProfileFilter prflt;
		ProfilePost prpost;
		double filtsize;

		ProfileCursors prcur;
		double c1x,c2x,c1y,c2y;

		profile ^prf;

		void cprofile_default();
		cprofile(csys ^c, ia_xml_read ^x);
		cprofile(vector data1, vector data2, csys ^cs);
		virtual void update_from_template(cmeasurement ^templ) override;

	[CategoryAttribute("Position")]
	property double StartX { double get() { return p1->real.x; };}
	[CategoryAttribute("Position")]
	property double StartY { double get() { return p1->real.y; };}
	[CategoryAttribute("Position")]
	property double EndX { double get() { return p2->real.x; };}
	[CategoryAttribute("Position")]
	property double EndY { double get() { return p2->real.y; };}
	[CategoryAttribute("Delta")]
	property double DeltaX { double get() { return p2->real.x-p1->real.x; };}
	[CategoryAttribute("Delta")]
	property double DeltaY { double get() { return p2->real.y-p1->real.y; };}
	[CategoryAttribute("Delta")]
	property double Length { double get() { vector v=p1->real-p2->real; return v.len; };}
	[CategoryAttribute("Angle")]
	property double Phi { double get() { return atan2(p2->real.y-p1->real.y,p2->real.x-p1->real.x)*180/M_PI; };}
	[CategoryAttribute("Line Fit Y=m*X+a")]
	property double LineFitY_m { double get() { return (p2->real.y-p1->real.y)/(p2->real.x-p1->real.x); };}
	[CategoryAttribute("Line Fit Y=m*X+a")]
	property double LineFitY_a { double get() { return p1->real.y-(p2->real.y-p1->real.y)/(p2->real.x-p1->real.x)*p1->real.x; };}
	[CategoryAttribute("Line Fit X=n*Y+b")]
	property double LineFitX_n { double get() { return (p2->real.x-p1->real.x)/(p2->real.y-p1->real.y); };}
	[CategoryAttribute("Line Fit X=n*Y+b")]
	property double LineFitX_b { double get() { return p1->real.x-(p2->real.x-p1->real.x)/(p2->real.y-p1->real.y)*p1->real.y; };}


	[CategoryAttribute("Appearance")]
	property bool ShowProfile {
		bool get() { return (showprofile)?true:false; };
		void set(bool sr) { showprofile=(sr)?1:0; }
	}
	[CategoryAttribute("Appearance")]
	property bool ShowLength {
		bool get() { return (showlength)?true:false; };
		void set(bool sr) { showlength=(sr)?1:0; }
	}
	[CategoryAttribute("Appearance")]
	property Color ProfileColor {
		Color get(void) { return prcolor; }
		void set(Color b) { prcolor=b; }
	};
	[CategoryAttribute("Appearance")]
	property bool ProfileFill {
		bool get(void) { return (fillprofile)?true:false; }
		void set(bool b) { fillprofile=(b)?1:0; }
	};
	[CategoryAttribute("Appearance")]
	property Color ProfileFillColor {
		Color get(void) { return prfillcol; }
		void set(Color b) { prfillcol=b; }
	};
	[CategoryAttribute("Appearance")]
	property int Transparency {
		int get(void) { return (int)floor(pralpha*100+0.5); }
		void set(int alp) { if (alp<0) alp=0; if (alp>100) alp=100;pralpha=(double)alp/100.0; }
	};
	[CategoryAttribute("Appearance")]
	property VAlign VerticalAlignment {
		VAlign get() { return va; };
		void set(VAlign sr) { va=sr; }
	}
	[CategoryAttribute("Profile")]
	property bool ShowFullRGB {
		bool get() { return (showrgb)?true:false; };
		void set(bool sr) { showrgb=(sr)?1:0; }
	}
	[CategoryAttribute("Profile")]
	property double ColorScale_R {
		double get() { return scaler; };
		void set(double sr) { scaler=sr; }
	}
	[CategoryAttribute("Profile")]
	property double ColorScale_G {
		double get() { return scaleg; };
		void set(double sr) { scaleg=sr; }
	}
	[CategoryAttribute("Profile")]
	property double ColorScale_B {
		double get() { return scaleb; };
		void set(double sr) { scaleb=sr; }
	}
	[CategoryAttribute("Profile")]
	property ProfileSource ProfileMode {
		ProfileSource get() { return prsrc; };
		void set(ProfileSource sr) { prsrc=sr; }
	}
	[CategoryAttribute("Profile")]
	property ProfileFilter ProfileProcess {
		ProfileFilter get() { return prflt; };
		void set(ProfileFilter sr) { prflt=sr; }
	}
	[CategoryAttribute("Profile")]
	property ProfilePost ProfileFinalize {
		ProfilePost get() { return prpost; };
		void set(ProfilePost sr) { prpost=sr; }
	}
	[CategoryAttribute("Profile")]
	property double FilterSize {
		double get() { return filtsize; };
		void set(double sr) { filtsize=sr; }
	}
	[CategoryAttribute("Cursors")]
	property ProfileCursors Cursors {
		ProfileCursors get() { return prcur; };
		void set(ProfileCursors sr) { prcur=sr; }
	}
	[CategoryAttribute("Cursors")]
	property double C1X {
		double get() { return c1x; };
	}
	[CategoryAttribute("Cursors")]
	property double C1Y {
		double get() { return c1y; };
	}
	[CategoryAttribute("Cursors")]
	property double C2X {
		double get() { return c2x; };
	}
	[CategoryAttribute("Cursors")]
	property double C2Y {
		double get() { return c2y; };
	}


	virtual String ^getdesc(void) override;
	virtual String ^exportCSV(void) override;

	public:
		virtual void draw(Graphics ^g) override;
		virtual List<cmeasurement^>^ touching(vector datai) override;
		virtual List<cmeasurement^>^ isinside(vector data1, vector data2) override;
		virtual void moveto(vector data_from, vector data_to) override;
		virtual void storestate() override;
		virtual void restorestate() override;
		virtual void refchangenotify(cmeasurement ^csrc) override;
		void calc_ref_point();
		void calcprofile();
		void postprofile(array<double> ^e);
		void filterprofile(array<double> ^e);
		array<double> ^getprofile();
		virtual void select(void) override;
		virtual void unselect(void) override;

		virtual void write_to_xml(ia_xml_write ^x) override;
		virtual int read_from_xml(ia_xml_read ^x) override;

	};



	public ref class cangle : cmeasurement {
	public:
		cpoint ^p1;
		cpoint ^p2;
		cpoint ^p3;

		double displayrad,displayrad_store;
		double phi;

		crefpoint ^pp;

		int drawrefpoints;
		int drawarrow;
		int noneg;

		void cangle_default();
		cangle(csys ^c, ia_xml_read ^x);
		cangle(vector data1, vector data2, vector data3, csys ^cs);
		virtual void update_from_template(cmeasurement ^templ) override;

	[CategoryAttribute("Point 1")]
	property double P1_X { double get() { return p1->real.x; };}
	[CategoryAttribute("Point 1")]
	property double P1_Y { double get() { return p1->real.y; };}

	[CategoryAttribute("Point 2")]
	property double P2_X { double get() { return p2->real.x; };}
	[CategoryAttribute("Point 2")]
	property double P2_Y { double get() { return p2->real.y; };}
	[CategoryAttribute("Point 2")]
	property double P2_PHI { double get() { vector a=p2->real-p1->real; return a.angle*180.0/M_PI; };}

	[CategoryAttribute("Point 3")]
	property double P3_X { double get() { return p3->real.x; };}
	[CategoryAttribute("Point 3")]
	property double P3_Y { double get() { return p3->real.y; };}
	[CategoryAttribute("Point 3")]
	property double P3_PHI { double get() { vector a=p3->real-p1->real; return a.angle*180.0/M_PI; };}

	[CategoryAttribute("Delta")]
	property double P23_Distance { double get() { vector v=p3->real-p2->real;return v.len; };}
	[CategoryAttribute("Delta")]
	property double P23_Phi { double get() { return angulardiff(p3->real-p1->real,p2->real-p1->real)*180.0/M_PI; };}

	[CategoryAttribute("Appearance")]
	property bool ShowReferencePoints {
		bool get(void) { return (drawrefpoints)?true:false; }
		void set(bool b) { drawrefpoints=(b)?1:0; }
	};
	[CategoryAttribute("Appearance")]
	property bool ShowArrow {
		bool get(void) { return (drawarrow)?true:false; }
		void set(bool b) { drawarrow=(b)?1:0; }
	};
	[CategoryAttribute("Appearance")]
	property bool NoNegativeAngle {
		bool get(void) { return (noneg)?true:false; }
		void set(bool b) { noneg=(b)?1:0; }
	};

	virtual String ^getdesc(void) override;
	virtual String ^exportCSV(void) override;

	public:
		void calc_ref_point();
		virtual void draw(Graphics ^g) override;
		virtual List<cmeasurement^>^ touching(vector datai) override;
		virtual List<cmeasurement^>^ isinside(vector data1, vector data2) override;
		virtual void moveto(vector data_from, vector data_to) override;
		virtual void storestate() override;
		virtual void restorestate() override;
		virtual void refchangenotify(cmeasurement ^csrc) override;

		virtual void select(void) override;
		virtual void unselect(void) override;

		virtual void write_to_xml(ia_xml_write ^x) override;
		virtual int read_from_xml(ia_xml_read ^x) override;
	};




	public ref class carea : cmeasurement {
	public:
		List<cpoint^> ^plist;

		vector cog;
		double area;
		double length;
		double perimeter;

		Color fillcol;
		double fillalp;

		int drawrefpoints;
		int closed;
		int fill;
		int showlength;
		int showarea;

		void carea_default();
		carea(csys ^c, ia_xml_read ^x);
		carea(List<vector^> ^pl, csys ^cs);
		virtual void update_from_template(cmeasurement ^templ) override;

	[CategoryAttribute("Data")]
	property double Area { double get() { return area; };}
	[CategoryAttribute("Data")]
	property double PathLength { double get() { return length; };}
	[CategoryAttribute("Data")]
	property double Perimeter { double get() { return perimeter; };}
	[CategoryAttribute("CenterOfGravity")]
	property double CoGX { double get() { return cog.x; };}
	[CategoryAttribute("CenterOfGravity")]
	property double CoGY { double get() { return cog.y; };}

	[CategoryAttribute("Appearance")]
	property bool ShowReferencePoints {
		bool get(void) { return (drawrefpoints)?true:false; }
		void set(bool b) { drawrefpoints=(b)?1:0; }
	};
	[CategoryAttribute("Appearance")]
	property bool ShowArea {
		bool get(void) { return (showarea)?true:false; }
		void set(bool b) { showarea=(b)?1:0; }
	};
	[CategoryAttribute("Appearance")]
	property bool ShowLength {
		bool get(void) { return (showlength)?true:false; }
		void set(bool b) { showlength=(b)?1:0; }
	};
	[CategoryAttribute("Appearance")]
	property bool ClosePolygon {
		bool get(void) { return (closed)?true:false; }
		void set(bool b) { closed=(b)?1:0; }
	};
	[CategoryAttribute("Appearance")]
	property bool Fill {
		bool get(void) { return (fill)?true:false; }
		void set(bool b) { fill=(b)?1:0; }
	};
	[CategoryAttribute("Appearance")]
	property Color FillColor {
		Color get(void) { return fillcol; }
		void set(Color b) { fillcol=b; }
	};
	[CategoryAttribute("Appearance")]
	property int Transparency {
		int get(void) { return (int)floor(fillalp*100+0.5); }
		void set(int alp) { if (alp<0) alp=0; if (alp>100) alp=100;fillalp=(double)alp/100.0; }
	};

	virtual String ^getdesc(void) override;
	virtual String ^exportCSV(void) override;

	public:
		void calc_area();
		virtual void draw(Graphics ^g) override;
		virtual List<cmeasurement^>^ touching(vector datai) override;
		virtual List<cmeasurement^>^ isinside(vector data1, vector data2) override;
		virtual void moveto(vector data_from, vector data_to) override;
		virtual void storestate() override;
		virtual void restorestate() override;
		virtual void refchangenotify(cmeasurement ^csrc) override;

		virtual void select(void) override;
		virtual void unselect(void) override;

		virtual void write_to_xml(ia_xml_write ^x) override;
		virtual int read_from_xml(ia_xml_read ^x) override;
	};


	public ref class ccircle3p : cmeasurement {
	public:
		cpoint ^p1;
		cpoint ^p2;
		cpoint ^p3;
		cpoint ^center;

		double radphi;
		double rad;

		crefpoint ^pp;

		int drawrefpoints;
		int showradline;
		int showdiameter;
		int showfullcircle;

		//double offset;

		//Color LineCol;
		//Font ^fnt;

		void ccircle3p_default();
		ccircle3p(csys ^c, ia_xml_read ^x);
		ccircle3p(vector data1, vector data2, csys ^cs);
		ccircle3p(vector data1, vector data2, vector data3, csys ^cs);
		virtual void update_from_template(cmeasurement ^templ) override;

	[CategoryAttribute("Point 1")]
	property double P1_X { double get() { return p1->real.x; };}
	[CategoryAttribute("Point 1")]
	property double P1_Y { double get() { return p1->real.y; };}
	[CategoryAttribute("Point 1")]
	property double P1_PHI { double get() { vector a=p1->real-center->real; return a.angle*180.0/M_PI; };}

	[CategoryAttribute("Point 2")]
	property double P2_X { double get() { return p2->real.x; };}
	[CategoryAttribute("Point 2")]
	property double P2_Y { double get() { return p2->real.y; };}
	[CategoryAttribute("Point 2")]
	property double P2_PHI { double get() { vector a=p2->real-center->real; return a.angle*180.0/M_PI; };}

	[CategoryAttribute("Point 3")]
	property double P3_X { double get() { return p3->real.x; };}
	[CategoryAttribute("Point 3")]
	property double P3_Y { double get() { return p3->real.y; };}
	[CategoryAttribute("Point 3")]
	property double P3_PHI { double get() { vector a=p3->real-center->real; return a.angle*180.0/M_PI; };}

	[CategoryAttribute("Center")]
	property double CTR_X { double get() { if (rad >= 0) return center->real.x; else return 0; };}
	[CategoryAttribute("Center")]
	property double CTR_Y { double get() { if (rad >= 0) return center->real.y; else return 0; };}


	[CategoryAttribute("Delta 1-->2")]
	property double P12_Distance { double get() { vector v=p2->real-p1->real;return v.len; };}
	[CategoryAttribute("Delta 1-->2")]
	property double P12_Phi { double get() { return angulardiff(p2->real-center->real,p1->real-center->real)*180.0/M_PI; };}
	[CategoryAttribute("Delta 1-->2")]
	property double P12_Radial { double get() { return angulardiff(p2->real-center->real,p1->real-center->real)*rad; };}

	[CategoryAttribute("Delta 2-->3")]
	property double P23_Distance { double get() { vector v=p3->real-p2->real;return v.len; };}
	[CategoryAttribute("Delta 2-->3")]
	property double P23_Phi { double get() { return angulardiff(p3->real-center->real,p2->real-center->real)*180.0/M_PI; };}
	[CategoryAttribute("Delta 2-->3")]
	property double P23_Radial { double get() { return angulardiff(p3->real-center->real,p2->real-center->real)*rad; };}
	
	[CategoryAttribute("Delta 1-->3")]
	property double P13_Distance { double get() { vector v=p3->real-p1->real;return v.len; };}
	[CategoryAttribute("Delta 1-->3")]
	property double P13_Phi { double get() { return angulardiff(p3->real-center->real,p1->real-center->real)*180.0/M_PI; };}
	[CategoryAttribute("Delta 1-->3")]
	property double P13_Radial { double get() { return angulardiff(p3->real-center->real,p1->real-center->real)*rad; };}

//	[CategoryAttribute("Appearance")]
//	property Color LineColor {
//		Color get(void) { return LineCol; }
//		void set(Color sc) { LineCol=sc; }
//	}

	[CategoryAttribute("Appearance")]
	property bool ShowRadialLine {
		bool get(void) { return (showradline)?true:false; }
		void set(bool srl) { showradline=(srl)?1:0; }
	}

	[CategoryAttribute("Appearance")]
	property bool ShowDiameter {
		bool get(void) { return (showdiameter) ? true : false; }
		void set(bool val) { showdiameter = (val) ? 1 : 0; }
	}

	[CategoryAttribute("Appearance")]
	property bool ShowFullCircle {
		bool get(void) { return (showfullcircle) ? true : false; }
		void set(bool val) { showfullcircle = (val) ? 1 : 0; }
	}

	[CategoryAttribute("Appearance")]
	property double RadiusPoint {
		double get(void) { return radphi*180.0/M_PI; }
		void set(double sr) { radphi=sr*M_PI/180.0; }
	}

	[CategoryAttribute("Circle")]
	property double Radius {
		double get(void) { return rad; }
	}



//	[CategoryAttribute("Appearance")]
//	property Font ^DisplayFont {
//		Font ^get(void) { return fnt; }
//		void set(Font ^f) { delete fnt; fnt=f; }
//	}

	virtual String ^getdesc(void) override;
	virtual String ^exportCSV(void) override;

	public:
		void setcenter();
		void calc_ref_point();
		virtual void draw(Graphics ^g) override;
		virtual List<cmeasurement^>^ touching(vector datai) override;
		virtual List<cmeasurement^>^ isinside(vector data1, vector data2) override;
		virtual void moveto(vector data_from, vector data_to) override;
		virtual void storestate() override;
		virtual void restorestate() override;
		virtual void refchangenotify(cmeasurement ^csrc) override;

		virtual void select(void) override;
		virtual void unselect(void) override;

		virtual void write_to_xml(ia_xml_write ^x) override;
		virtual int read_from_xml(ia_xml_read ^x) override;
	};

	public enum class CoordMode {
		TwoPoint,			// Classic Two Point
		TwoPointYScale,		// Two Point, Scaling Factor for Y
		TwoPointSimTilt,	// Two Point, Simulate tilted image (typical SEM pictures)
		ThreePoint,			// Three Point
		ThreePointForce90	// Three Point, forced 90degree system
	};

	public ref class coordinate : cmeasurement {
	public:
		cpoint ^p1;
		cpoint ^p2;
		cpoint ^p3;

		vector C;
		matrix M;
		double xcross;
		double ycross;
		int xcrossauto;
		int ycrossauto;

		CoordMode cmode;
		double yscale;
		double tiltangle;

		// int twopoint;
		// int force90;

		int drawrefpoints;
		int drawcoord;
		int drawmesh;

//		Color LineCol;
//		Font ^fnt;
		Color bkg;

		void coordinate_default();
		coordinate(csys ^c, ia_xml_read ^x);
		coordinate(csys ^c, int tp);

		int istwopoint(void);

		String ^getUnit(void);
		void setUnit(String ^s);
		String ^getFormat(void);
		void setFormat(String ^s);
		String ^getCUnit(void);
		void setCUnit(String ^s);
		String ^getCFormat(void);
		void setCFormat(String ^s);

	[CategoryAttribute("Mode")]
	property CoordMode SystemMode {
		CoordMode get(void) { return cmode; }
		void set(CoordMode nm) { changemode(nm); }
	};
	[CategoryAttribute("Mode")]
	property double YScale {
		double get(void) { return yscale; }
		void set(double v) { yscale=v;calctrf(); }
	};
	[CategoryAttribute("Mode")]
	property double ImageTiltAngle {
		double get(void) { return tiltangle; }
		void set(double v) { tiltangle=v;calctrf(); }
	};	
	
	//	[CategoryAttribute("Mode")]
//	property bool TwoPoint {
//		bool get(void) { return (twopoint)?true:false; }
//		void set(bool b) { twopointmode((b)?1:0); }
//	};
//	[CategoryAttribute("Mode")]
//	property bool ThreePoint90 {
//		bool get(void) { return (force90)?true:false; }
//		void set(bool b) { force90mode((b)?1:0); }
//	};

/*	property bool XLog {
		bool get(void) { return false; }
		void set(bool b) { }
	}

	property bool YLog {
		bool get(void) { return false; }
		void set(bool b) { }
	}*/

	[CategoryAttribute("System")]
	property String ^Unit {
		String ^get(void) { return getUnit(); }
		void set(String ^s) { setUnit(s); }
	}

	[CategoryAttribute("System")]
	property String ^Format {
		String ^get(void) { return getFormat(); }
		void set(String ^s) { setFormat(s); }
	}

	[CategoryAttribute("System")]
	property String ^ColorBarUnit {
		String ^get(void) { return getCUnit(); }
		void set(String ^s) { setCUnit(s); }
	}

	[CategoryAttribute("System")]
	property String ^ColorBarFormat {
		String ^get(void) { return getCFormat(); }
		void set(String ^s) { setCFormat(s); }
	}

	[CategoryAttribute("System")]
	property ColorMode ColorDisplayFormat {
		ColorMode get(void) { return c->f->cmode; }
		void set(ColorMode s) { c->f->cmode=s; }
	}

//	[CategoryAttribute("Appearance")]
//	property Color LineColor {
//		Color get(void) { return LineCol; }
//		void set(Color sc) { LineCol=sc; }
//	}
	[CategoryAttribute("Appearance")]
	property Color BackgroundColor {
		Color get(void) { return bkg; }
		void set(Color sc) { bkg=sc; }
	}

	[CategoryAttribute("Appearance")]
	property bool ShowRefPoints {
		bool get(void) { return (drawrefpoints)?true:false; }
		void set(bool sr) { drawrefpoints=(sr)?1:0; }
	}

	[CategoryAttribute("Appearance")]
	property bool ShowMesh {
		bool get(void) { return (drawmesh)?true:false; }
		void set(bool sr) { drawmesh=(sr)?1:0; }
	}

//	[CategoryAttribute("Appearance")]
//	property Font ^DisplayFont {
//		Font ^get(void) { return fnt; }
//		void set(Font ^f) { delete fnt; fnt=f; }
//	}
	

	[CategoryAttribute("X-Axis position")]
	property bool AutoXPosition {
		bool get(void) { return (ycrossauto)?true:false; }
		void set(bool sr) { ycrossauto=(sr)?1:0; }
	}
	[CategoryAttribute("X-Axis position")]
	property double XPosition {
		double get(void) { return ycross; }
		void set(double sr) { ycross=sr; }
	}
	[CategoryAttribute("Y-Axis position")]
	property bool AutoYPosition {
		bool get(void) { return (xcrossauto)?true:false; }
		void set(bool sr) { xcrossauto=(sr)?1:0; }
	}
	[CategoryAttribute("Y-Axis position")]
	property double YPosition {
		double get(void) { return xcross; }
		void set(double sr) { xcross=sr; }
	}

	virtual String ^getdesc(void) override;

	public:
		vector getreal(vector data);
		vector getdata(vector real);
		void calctrf(void);
		void changemode(CoordMode newmode);
//		void twopointmode(int m);
//		void force90mode(int m);

		virtual void draw(Graphics ^g) override;

		virtual List<cmeasurement^>^ touching(vector datai) override;
		virtual List<cmeasurement^>^ isinside(vector data1, vector data2) override;
		virtual void moveto(vector data_from, vector data_to) override;
		virtual void storestate() override;
		virtual void restorestate() override;

		virtual void select(void) override;
		virtual void unselect(void) override;

		virtual void write_to_xml(ia_xml_write ^x) override;
		virtual int read_from_xml(ia_xml_read ^x) override;

	};

}
