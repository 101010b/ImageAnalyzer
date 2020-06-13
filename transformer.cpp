#include "stdafx.h"
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <memory.h>
#include <math.h>
// #include "image_add_win.h"
//#include "imagedata.h"
//#include "measurements.h"
#include "vectormatrix.h"
#include "transformer.h"

using namespace image_add;

transformer::transformer(void) {
	scale=1.0F;
	screen=vector(100,100);
	center=vector(0,0);
}

transformer::transformer(vector sc) {
	transformer();
	screen=sc;
}

transformer::~transformer(void) {
}

void transformer::resize(vector sc) {
	screen=sc;
}

void transformer::rescale(double sc) {
	scale=sc;
}

void transformer::recenter(vector sc) {
	center=sc;
}

vector transformer::todata(vector s) {
	return (s-screen/2-center)/scale;
}

vector transformer::toscreen(vector d) {
	return (d*scale)+center+screen/2;
}

String ^formatter::format(double v) {
	if (unit->Length < 1) {
		return gcnew String(String::Format("{0:" + form + "}",v));
	} else {
		return gcnew String(String::Format("{0:" + form + "} " + unit,v));
	}
}

String ^formatter::format(vector v) {
	if (unit->Length < 1) {
		return gcnew String(String::Format("{0:" + form + "},{1:" + form + "}",v.x,v.y));
	} else {
		return gcnew String(String::Format("{0:" + form + "},{1:" + form + "} " + unit,v.x,v.y));
	}
}

String ^formatter::cformat(double v) {
	if (cunit->Length < 1) {
		return gcnew String(String::Format("{0:" + cform + "}",v));
	} else {
		return gcnew String(String::Format("{0:" + cform + "} " + cunit,v));
	}
}

String ^formatter::format_nounit(double v) {
	return gcnew String(String::Format("{0:" + form + "}",v));
}

String ^formatter::format_nounit(vector v) {
	return gcnew String(String::Format("{0:" + form + "},{1:" + form + "}",v.x,v.y));
}

String ^formatter::cformat_nounit(double v) {
	return gcnew String(String::Format("{0:" + cform + "}",v));
}


String ^formatter::colorformat(double cr,double cg, double cb) {
	int ir, ig, ib;
	if (cr < 0.0) cr=0.0; if (cr > 1.0) cr=1.0;
	if (cg < 0.0) cg=0.0; if (cg > 1.0) cg=1.0;
	if (cb < 0.0) cb=0.0; if (cb > 1.0) cb=1.0;
	switch (cmode) {
	case ColorMode::RGBByte:
		ir=floor(cr*255+0.5);
		ig=floor(cg*255+0.5);
		ib=floor(cb*255+0.5);
		return gcnew String(String::Format("{0},{1},{2}",ir,ig,ib));
	case ColorMode::RGBFloat:
		return gcnew String(String::Format("{0:F3},{1:F3},{2:F3}",cr,cg,cb));
	case ColorMode::HTML:
		ir=floor(cr*255+0.5);
		ig=floor(cg*255+0.5);
		ib=floor(cb*255+0.5);
		return gcnew String(String::Format("#{0,2:X2}{1,2:X2}{2,2:X2}",ir,ig,ib));
	}
	return nullptr;
}

String ^formatter::colorformat(int ir,int ig, int ib) {
	double cr, cg, cb;
	if (ir < 0) ir=0;if (ir > 255) ir=255;
	if (ig < 0) ig=0;if (ig > 255) ig=255;
	if (ib < 0) ib=0;if (ib > 255) ib=255;
	switch (cmode) {
	case ColorMode::RGBByte:
		return gcnew String(String::Format("{0},{1},{2}",ir,ig,ib));
	case ColorMode::RGBFloat:
		cr=(double)ir / 255.0;
		cg=(double)ig / 255.0;
		cb=(double)ib / 255.0;
		return gcnew String(String::Format("{0:F3},{1:F3},{2:F3}",cr,cg,cb));
	case ColorMode::HTML:
		return gcnew String(String::Format("#{0,2:X2}{1,2:X2}{2,2:X2}",ir,ig,ib));
	}
	return nullptr;
}

String ^formatter::format_angle(double v) {
	switch (amode) {
	case AngleMode::Radians:
		return gcnew String(String::Format("{0:F3}rad",v));
		break;
	case AngleMode::Degrees:
		return gcnew String(String::Format("{0:F1}°",v*180.0/M_PI));
		break;
	}
	return gcnew String(String::Format("{0:F3}",v));
}

String ^formatter::format_area(double v) {
	if (unit->Length < 1) {
		return gcnew String(String::Format("{0:" + form + "}",v));
	} else {
		return gcnew String(String::Format("{0:" + form + "} " + unit + "²",v));
	}
}

// EOF