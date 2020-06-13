#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <math.h>
// #include "image_add_win.h"
#include "imagedata.h"
#include "measurements.h"
#include "ia_xml_io.h"

#ifndef M_PI
	#define M_PI       3.14159265358979323846
#endif

using namespace image_add;


// ****************************************************************** I N T E R N A L **** U T I L I T I E S ***************************************************
#define MIN(A,B) (((A)<(B))?(A):(B))
#define MAX(A,B) (((A)<(B))?(B):(A))
#define MAX3(A,B,C) MAX(MAX((A),(B)),(C))
#define MIN3(A,B,C) MIN(MIN((A),(B)),(C))

void getHSLfromRGB(double R, double G, double B, double *H, double *S, double *I) {
	double cmin=MIN3(R,G,B);
	double cmax=MAX3(R,G,B);
	double dmax=cmax-cmin;
	*I=(cmax+cmin)/2;
	if (dmax == 0) {
		*H=0;
		*S=0;
	} else {
		if (*I < 0.5) 
			*S = dmax/(cmax+cmin);
		else
			*S = dmax/(2.0 - cmax - cmin);
		double dr=((cmax-R)/6+dmax/2)/dmax;
		double dg=((cmax-G)/6+dmax/2)/dmax;
		double db=((cmax-B)/6+dmax/2)/dmax;
		if (R == cmax) {
			*H=db-dg;
		} else if (G == cmax) {
			*H=1.0/3.0+dr-db;
		} else {
			*H=2.0/3.0+dg-dr;
		}
		if (*H < 0) 
			*H+=1.0;
		if (*H > 1.0)
			*H-=1.0;
	}
}

void filter(array<double> ^e,array<double> ^f,int renorm) {
	if ((e==nullptr) || (f == nullptr)) return;

	int N=e->Length;
	int Q=f->Length;
	array<double> ^e2=gcnew array<double>(N);
	double fsum=0;
	for (int i=0;i<N;i++) e2[i]=e[i];
	for (int i=0;i<Q;i++) fsum+=f[i];
	if (renorm) {
		for (int i=0;i<N;i++) {
			double csum=0;
			double esum=0;
			for (int q=0;q<Q;q++) {
				int m=i+q-Q/2;
				if ((m >= 0) && (m < N)) {
					csum+=f[q];
					esum+=f[q]*e2[m];
				}
			}
			if ((fsum != 0) && (csum != 0)) 
				esum*=fsum/csum;
			e[i]=esum;
		}
	} else {
		for (int i=0;i<N;i++) {
			double esum=0;
			for (int q=0;q<Q;q++) {
				int m=i+q-Q/2;
				if (m < 0) esum+=f[q]*e2[0];
				else if (m >= N) esum+=f[q]*e2[N-1];
				else esum+=f[q]*e2[m];
			}
			e[i]=esum;
		}
	}
	delete e2;
}

void filter_median(array<double> ^e,int filtsize) {
	if (e == nullptr) return;
	if (filtsize < 2) return;
	array<int> ^order=gcnew array<int>(filtsize);
	array<double> ^e2=gcnew array<double>(e->Length);
	for (int i=0;i<e->Length;i++) e2[i]=e[i];
	for (int i=0;i<e->Length;i++) {
		int fnd=0;
		for (int q=0;q<filtsize;q++) {
			int m=i+q-(q-1)/2;
			if ((m >= 0) && (m < e->Length)) {
				order[fnd]=m;
				fnd++;
			}
		}
		if (fnd < 2) 
			e[i]=e2[i];
		else {
			for (int q=0;q<fnd-1;q++) 
				for (int r=q+1;r<fnd;r++) {
					if (e2[order[q]] > e2[order[r]]) {
						int t=order[q];
						order[q]=order[r];
						order[r]=t;
					}
				}
			e[i]=e2[order[fnd/2]];
		}
	}
	delete order;
	delete e2;
}

array<double> ^makefilter(ProfileFilter pf, double filtersize) {
	array<double> ^f;
	int fs;
	double s;
	if (filtersize < 1) return nullptr;
	fs=ceil(filtersize*2);
	switch (pf) {
	case ProfileFilter::Off: 
	case ProfileFilter::Derivative:
	case ProfileFilter::Median:
	case ProfileFilter::Integrated:
		return nullptr;
	case ProfileFilter::HighPass:
		f=gcnew array<double>(fs);
		for (int i=0;i<fs;i++) {
			double x=((double)i-((double)fs-1.0)/2.0)/(filtersize/2);
			f[i]=exp(-x*x*3)*2/3*x;
		}
		return f;
	case ProfileFilter::LowPass:
		f=gcnew array<double>(fs);
		s=0;
		for (int i=0;i<fs;i++) {
			double x=((double)i-((double)fs-1.0)/2.0)/(filtersize/2);
			f[i]=exp(-x*x*3);
			s+=f[i];
		}
		// Normalize
		for (int i=0;i<fs;i++) f[i]/=s;
		return f;
	}
	return nullptr;
}


// ************************************************ C O O R D I N A T E S Y S T E M ***********************************************************


csys::csys(formatter ^fmt, transformer ^trf, coordinate ^cs, cframe ^fr, ccolorbar ^cb) {
	f=fmt;
	t=trf;
	c=cs;
	frm=fr;
	cbar=cb;
}

csys::csys(formatter ^fmt, transformer ^trf) {
	f=fmt;
	t=trf;
	c=nullptr;
	frm=nullptr;
	cbar=nullptr;
}

vector csys::screen_from_real(vector v) {
	return t->toscreen(c->getdata(v)); 
}

vector csys::screen_from_data(vector v) { 
	return t->toscreen(v); 
}

vector csys::data_from_real(vector v) { 
	return c->getdata(v); 
}

vector csys::data_from_screen(vector v) { 
	return t->todata(v); 
}

vector csys::real_from_data(vector v) { 
	return c->getreal(v); 
}

vector csys::real_from_screen(vector v) { 
	return c->getreal(t->todata(v)); 
}

vector csys::image_from_data(vector v) {
	if (img == nullptr) return v;
	return v+vector(img->width,img->height)/2;
}

vector csys::data_from_image(vector v) {
	if (img == nullptr) return v;
	return v-vector(img->width,img->height)/2;
}


//********************************************* C M E A S U R E M E N T ***********************************************************

void cmeasurement::cmeasurement_default() {
	name="";
	type="";
	id=0;
	c=nullptr;
	root=nullptr;
	selected=0;
	readonly=0;

	fnt=gcnew System::Drawing::Font(FontFamily::GenericSansSerif,8.0F,FontStyle::Regular);
	fntcol=Color::FromArgb(255,255,255);
	fntfrm=Color::FromArgb(0,0,0);
	fntfrmuse=1;
	linecol=Color::FromArgb(0,192,0);
}

cmeasurement::cmeasurement(String ^mtype, String ^mname, csys ^cs) {
	cmeasurement_default();
	name=mname;
	type=mtype;
	c=cs;
}

cmeasurement::cmeasurement(String ^mtype, csys ^cs) {
	cmeasurement_default();
	type=mtype;
	c=cs;
}

cmeasurement::cmeasurement(String ^mtype, csys ^cs, cmeasurement ^rt) {
	cmeasurement_default();
	type=mtype;
	c=cs;
	root=rt;
}

void cmeasurement::update_from_template(cmeasurement ^templ) {
	if (templ == nullptr) return;
	fnt = gcnew Font(templ->fnt,templ->fnt->Style);
	fntcol=templ->fntcol;
	fntfrm=templ->fntfrm;
	fntfrmuse=templ->fntfrmuse;
	linecol=templ->linecol;
}

cmeasurement::~cmeasurement() {
}

void cmeasurement::select(void) {
	if (!selected) {
		selected=1;
	}
}
void cmeasurement::unselect(void) {
	if (selected) {
		selected=0;
	}
}

String ^cmeasurement::getdesc(void) {
	return gcnew String("Measurement");
}

String ^cmeasurement::exportCSV(void) {
	return nullptr;
}

void cmeasurement::draw(Graphics ^g) {
}

List<cmeasurement^>^ cmeasurement::touching(vector datai) {
	return nullptr;
}

List<cmeasurement^>^ cmeasurement::isinside(vector data1, vector data2) {
	return nullptr;
}

void cmeasurement::moveto(vector data_from, vector data_to) {
}

void cmeasurement::refchangenotify(cmeasurement ^c) {
}

void cmeasurement::storestate() {
}

void cmeasurement::restorestate() {
}

void cmeasurement::write_to_xml(ia_xml_write ^x) {
	x->write_value("NAME",name);
	x->write_value("ID",id);
	x->write_value("FNTCOL",fntcol);
	x->write_value("FNTFRM",fntfrm);
	x->write_value("FNTFRMUSE",fntfrmuse);
	x->write_value("LINECOL",linecol);
	x->write_value("FONT",fnt);
}

int cmeasurement::read_from_xml(ia_xml_read ^x) {
	if (String::Compare(x->Name,"NAME")==0) {
		if (x->IsEmptyElement) { name="";x->Read(); }
		else name=x->ReadElementContentAsString();
		return 1;
	}
	if (String::Compare(x->Name,"FNTCOL")==0) {
		x->Read();fntcol=x->xml_read_color();
		return 1;
	}
	if (String::Compare(x->Name,"ID")==0) {
		id=x->ReadElementContentAsInt();
		return 1;
	}
	if (String::Compare(x->Name,"FNTFRM")==0) {
		x->Read();fntfrm=x->xml_read_color();
		return 1;
	}
	if (String::Compare(x->Name,"FNTFRMUSE")==0) {
		fntfrmuse=x->ReadElementContentAsInt();
		return 1;
	}
	if (String::Compare(x->Name,"LINECOL")==0) {
		x->Read();linecol=x->xml_read_color();
		return 1;
	}
	if (String::Compare(x->Name,"FONT")==0) {
		x->Read();
		Font ^ftmp=x->xml_read_font();
		if (ftmp != nullptr) fnt=ftmp;
		return 1;
	}
	return 0;
}

void cmeasurement::drawtxt(Graphics ^g, vector pos, vector along, int ax, int ay, String ^txt) {
	double xofs;
	double yofs;
	SizeF size;
	double phi;
	vector vx,vy;
	RectangleF bb;

	if (pos.len > 1e6) return;
	if (txt == nullptr) return;

	txt=txt->Replace('|','\n');

	Drawing2D::GraphicsPath ^gp=gcnew Drawing2D::GraphicsPath();
	
	StringFormat ^sf=gcnew StringFormat();
	switch (ax) {
	case -1: // Left
		sf->Alignment=StringAlignment::Near;
		break;
	case 0: // Center
		sf->Alignment=StringAlignment::Center;
		break;
	case 1: // Right
		sf->Alignment=StringAlignment::Far;
		break;
	}
	gp->AddString(txt,fnt->FontFamily,(int) fnt->Style,fnt->Height,PointF(0,0),sf);
	delete sf;

	bb=gp->GetBounds();

	bb.Inflate(SizeF(2,2));

	size.Width=bb.Width;
	size.Height=bb.Height;

	if (along.len <= 0) along=vector(1,0);
	phi=atan2(along.y,along.x)*180/M_PI;

	vx=along/along.len;
	vy=vector(-vx.y,vx.x);

	if ((phi > 90) || (phi < -90)) {
		if (phi > 90) phi=phi-180; else phi=phi+180;
		ax=-ax;
		ay=-ay;
		vx=vx*(-1);
		vy=vector(-vx.y,vx.x);
	}

	switch (ax) {
	case -1 : xofs=bb.Left;break;
	case 0: xofs=(bb.Right+bb.Left)/2;break;
	case 1: xofs=bb.Right;break;
	}
	switch (ay) {
	case -1: yofs=bb.Top;break;
	case 0: yofs=(bb.Bottom+bb.Top)/2;break;
	case 1: yofs=bb.Bottom;break;
	}
	Drawing2D::Matrix mt;
	mt.Translate(-xofs,-yofs);
	gp->Transform(%mt);

	g->TranslateTransform(pos.x,pos.y);
	g->RotateTransform(phi);

	if (fntfrmuse) {
		Pen ^bg = gcnew Pen(fntfrm,2.0f);
		//g->FillPath(bg,gp); 
		g->DrawPath(bg,gp);
		delete bg;
	}
	SolidBrush ^fb=gcnew SolidBrush(fntcol);
	g->FillPath(fb,gp);
	delete fb;

	g->ResetTransform();
	delete gp;
}

void cmeasurement::drawtxtbox(Graphics ^g, double minx, double miny, double maxx, double maxy, int ax, int ay, String ^txt) {
	double xofs,xpos;
	double yofs,ypos;
	SizeF size;
	RectangleF bb;

	if (txt == nullptr) return;
	txt=txt->Replace('|','\n');

	StringFormat ^sf=gcnew StringFormat();
	switch (ax) {
	case -1: // Left
		sf->Alignment=StringAlignment::Near;
		break;
	case 0: // Center
		sf->Alignment=StringAlignment::Center;
		break;
	case 1: // Right
		sf->Alignment=StringAlignment::Far;
		break;
	}

	Drawing2D::GraphicsPath ^gp=gcnew Drawing2D::GraphicsPath();
	gp->AddString(txt,fnt->FontFamily,(int) fnt->Style,fnt->Height,PointF(0,0),sf);
	bb=gp->GetBounds();

	bb.Inflate(SizeF(2,2));

	size.Width=bb.Width;
	size.Height=bb.Height;

	switch (ax) {
	case -1 : 
		xofs=bb.Left;
		xpos=minx;
		break;
	case 0: 
		xofs=(bb.Right+bb.Left)/2;
		xpos=(maxx+minx)/2;
		break;
	case 1: 
		xofs=bb.Right;
		xpos=maxx;
		break;
	}
	switch (ay) {
	case -1: 
		yofs=bb.Top;
		ypos=miny;
		break;
	case 0: 
		yofs=(bb.Bottom+bb.Top)/2;
		ypos=(maxy+miny)/2;
		break;
	case 1: 
		yofs=bb.Bottom;
		ypos=maxy;
		break;
	}

	Drawing2D::Matrix mt;
	mt.Translate(-xofs,-yofs);
	gp->Transform(%mt);

	g->TranslateTransform(xpos,ypos);

	if (fntfrmuse) {
		Pen ^bg = gcnew Pen(fntfrm,2.0f);
		//g->FillPath(bg,gp); 
		g->DrawPath(bg,gp);
		delete bg;
	}
	SolidBrush ^fb=gcnew SolidBrush(fntcol);
	g->FillPath(fb,gp);
	delete fb;

	g->ResetTransform();
	delete gp;
}

int cmeasurement::findsepoints(Graphics ^g, vector A, vector B, double *s, double *e) {
	vector V=B-A;
	double tmin,tmax;
	int fndp=0;
//	double sw=(double)g->VisibleClipBounds.Width;
//	double sh=(double)g->VisibleClipBounds.Height;
	double sw=c->t->screen.x;
	double sh=c->t->screen.y;

	// intersect with vertical limits
	if (V.x != 0) {
		double t=-A.x/V.x;
		double y=A.y+V.y*t;
		if ((y >= 0) && (y <= sh)) {
			if (fndp==0) tmin=tmax=t; 
			if (t < tmin) tmin=t;
			if (t > tmax) tmax=t;
			fndp++;
		}
		t=(sw-A.x)/V.x;
		y=A.y+V.y*t;
		if ((y >= 0) && (y <= sh)) {
			if (fndp==0) tmin=tmax=t; 
			if (t < tmin) tmin=t;
			if (t > tmax) tmax=t;
			fndp++;
		}
	} 
	if (V.y != 0) {
		double t=-A.y/V.y;
		double x=A.x+V.x*t;
		if ((x >= 0) && (x <= sw)) {
			if (fndp==0) tmin=tmax=t; 
			if (t < tmin) tmin=t;
			if (t > tmax) tmax=t;
			fndp++;
		}
		t=(sh-A.y)/V.y;
		x=A.x+V.x*t;
		if ((x >= 0) && (x <= sw)) {
			if (fndp==0) tmin=tmax=t; 
			if (t < tmin) tmin=t;
			if (t > tmax) tmax=t;
			fndp++;
		}
	}
	if (fndp < 2) return 0;
	if (tmin==tmax) return 0;
	*s=tmin;
	*e=tmax;
	return 1;
}

void cmeasurement::drawinfline(Graphics ^g, vector A, vector B, Pen ^p) {
	vector V=B-A;
	double t;
	float x0,y0,x1,y1;

	if (fabs(V.x) > fabs(V.y)) {
		t=(-10-A.x)/V.x;
		x0=-10;
		y0=A.y+V.y*t;
		
		//t=((double)g->VisibleClipBounds.Width+10-A.x)/V.x;
		t=(c->t->screen.x+10-A.x)/V.x;
		// x1=g->VisibleClipBounds.Width+10;
		x1=c->t->screen.x+10;
		y1=A.y+V.y*t;
	} else {
		t=(-10-A.y)/V.y;
		x0=A.x+V.x*t;
		y0=-10;
		
		//t=((double)g->VisibleClipBounds.Height+10-A.y)/V.y;
		t=(c->t->screen.y+10-A.y)/V.y;
		x1=A.x+V.x*t;
		//y1=g->VisibleClipBounds.Height+10;
		y1=c->t->screen.y+10;
	}

	g->DrawLine(p,x0,y0,x1,y1);
}

void cmeasurement::drawline(Graphics ^g, vector A, vector B, Pen ^p) {
	if ((A.len > 1e5) || (B.len > 1e5)) return;
	g->DrawLine(p,(float)A.x,(float)A.y,(float)B.x,(float)B.y);
}

double cmeasurement::getlinedist(vector A, vector B, vector S) {
	vector v=B-A;
	vector w=S-A;
	double l=v.len;
	if (l == 0) return 1e99;
	return (v^w)/l;
}

double cmeasurement::getinlinedist(vector A, vector B, vector S) {
	vector v=B-A;
	vector w=S-A;
	vector r=S-B;
	double l=v.len;
	if (l == 0) return 1e99;

	double h=(v^w)/l;
	double m=(v*w)/l;
	if (m < 0) return w.len;
	if (m > l) return r.len;
	return h;
}

double cmeasurement::find10step(double l,double maxw) {
	double ll=floor(log10(maxw/l));
	return pow(10,ll);	
}

void cmeasurement::drawtickline(Graphics ^g, vector AR, vector BR, Pen ^pen, int showticks, String ^txt) {

	vector A=c->t->toscreen(c->c->getdata(AR));
	vector B=c->t->toscreen(c->c->getdata(BR));
	vector vx=B-A;
	vector vy=vector(-vx.y,vx.x);
	vector vr=AR-BR;
	double lr=vr.len;
	double lx=vx.len;

	double step10x=find10step(lx/lr,c->t->screen.len/2);
	double tickxlen=5/lx;	
	double t1,t2;
	
	drawline(g,A,B,pen);

	if (findsepoints(g,A,B,&t1,&t2)) {
		double dt1,dt2;dt1=t1;dt2=t2;
		if (dt1 < 0) dt1=0;
		if (dt2 > 1) dt2=1;
		double sp=floor(t1*lr/step10x);
		double ep=ceil(t2*lr/step10x);
		vector v=A+vx*(dt1+dt2)/2;
		//vector rv=AR-BR;
		//drawtxt(Graphics ^g, Font ^f, SolidBrush ^b, SolidBrush ^bg, vector pos, vector along, int ax, int ay, String ^txt) 
		drawtxt(g,v,vx,0,1,txt);
		// drawtxt(g,fnt,b,v,vx,0,1,txt);

		drawline(g,A-vy*2*tickxlen,A+vy*2*tickxlen,pen);
		drawline(g,B-vy*2*tickxlen,B+vy*2*tickxlen,pen);
			
		if (showticks) {
			for (int i=sp;i<=ep;i++) {
				double tickpos=(double)i*step10x/lr;
				if ((tickpos >= dt1) && (tickpos <= dt2))
					drawline(g,A+vx*tickpos-vy*tickxlen,A+vx*tickpos+vy*tickxlen,pen);
				for (int j=1;j<10;j++) {
					double tickpos=(double)(i+0.1*j)*step10x/lr;
					double ticklen=0.3;
					if (j==5) ticklen=0.6;
					if ((tickpos >= dt1) && (tickpos <= dt2))
						drawline(g,A+vx*tickpos-vy*tickxlen*ticklen,A+vx*tickpos+vy*tickxlen*ticklen,pen);
				}
			}
		}
	}
}



//************************************************* C F R A M E ***********************************************************

void cframe::cframe_default() {
	frms=FrameStyles::DoubleWithSections;
	frmw=15;
	flds=FieldStyles::Bottom;
	fullpath=1;
	showimgfileinfo=1;
	showcmt=0;
	showimg=1;
	showcal=1;
	showfn=1;
	showscale=1;
	showcoord=0;
	BackCol=Color::LightSteelBlue;
	alpha=0.7;
	fntcol=Color::FromArgb(0,0,0);
	linecol=Color::FromArgb(65,105,225);
	fntfrmuse=0;

	comment="";
	imgfilename=nullptr;
	imgfileinfo=nullptr;
	filename=nullptr;
	calname=nullptr;
}

cframe::cframe(csys ^c):cmeasurement(gcnew String("Frame"),c) {
	cframe_default();
}

cframe::cframe(csys ^c, ia_xml_read ^x):cmeasurement(gcnew String("Frame"),c) {
	cframe_default();
	readonly=1;
	c->frm=this;

	imgfilename=nullptr;
	imgfileinfo=nullptr;
	filename=nullptr;
	calname=nullptr;

	while (1) {
		switch (x->NodeType) {
			case XmlNodeType::Element:
				if (!read_from_xml(x)) {
					// Unknown element --> Skip over
					if (x->IsEmptyElement)	x->Read(); else x->Skip();
				}
				break;
			case XmlNodeType::EndElement:
				x->Read();
				return;
			default:
				throw gcnew System::Exception("Bad XML Formatted File");
		}
	}
}

void cframe::write_to_xml(ia_xml_write ^x) {
	cmeasurement::write_to_xml(x);

	x->write_value("FRAMESTYLE",(int)frms);
	x->write_value("FRAMEWIDTH",frmw);
	x->write_value("FIELDSTYLE",(int)flds);
	//x->write_value("FULLPATH",fullpath);
	x->write_value("SHOWIMGFILEINFO",showimgfileinfo);
	x->write_value("SHOWCOMMENT",showcmt);
	x->write_value("SHOWIMGFILENAME",showimg);
	x->write_value("SHOWCALFILENAME",showcal);
	x->write_value("SHOWFILENAME",showfn);
	x->write_value("SHOWSCALE",showscale);
	x->write_value("SHOWCORNERCOORDINATES",showcoord);
	x->write_value("BACKGROUNDCOLOR",BackCol);
	x->write_value("ALPHA",alpha);
	x->write_value("COMMENT",comment);
	x->write_value("IMAGEFN",imgfilename);
	x->write_value("IMAGEFINFO",imgfileinfo);
	x->write_value("FILENAME",filename);
	x->write_value("CALNAME",calname);
}

int cframe::read_from_xml(ia_xml_read ^x) {
	if (cmeasurement::read_from_xml(x)) return 1;

	if (String::Compare(x->Name,"FRAMESTYLE")==0) {
		frms=(FrameStyles)x->ReadElementContentAsInt();
	} else if (String::Compare(x->Name,"FRAMEWIDTH")==0) {
		frmw=x->ReadElementContentAsInt();
	} else if (String::Compare(x->Name,"FIELDSTYLE")==0) {
		flds=(FieldStyles)x->ReadElementContentAsInt();
	//} else if (String::Compare(x->Name,"FULLPATH")==0) {
	//	fullpath=x->ReadElementContentAsInt();
	} else if (String::Compare(x->Name,"SHOWIMGFILEINFO")==0) {
		showimgfileinfo=x->ReadElementContentAsInt();
	} else if (String::Compare(x->Name,"SHOWCOMMENT")==0) {
		showcmt=x->ReadElementContentAsInt();
	} else if (String::Compare(x->Name,"SHOWIMGFILENAME")==0) {
		showimg=x->ReadElementContentAsInt();
	} else if (String::Compare(x->Name,"SHOWCALFILENAME")==0) {
		showcal=x->ReadElementContentAsInt();
	} else if (String::Compare(x->Name,"SHOWFILENAME")==0) {
		showfn=x->ReadElementContentAsInt();
	} else if (String::Compare(x->Name,"SHOWSCALE")==0) {
		showscale=x->ReadElementContentAsInt();
	} else if (String::Compare(x->Name,"SHOWSCALE")==0) {
		showscale=x->ReadElementContentAsInt();
	} else if (String::Compare(x->Name,"SHOWCORNERCOORDINATES")==0) {
		showcoord=x->ReadElementContentAsInt();
	} else if (String::Compare(x->Name,"BACKGROUNDCOLOR")==0) {
		x->Read();BackCol=x->xml_read_color();
	} else if (String::Compare(x->Name,"ALPHA")==0) {
		alpha=x->ReadElementContentAsDouble();
	} else if (String::Compare(x->Name,"COMMENT")==0) {
		if (x->IsEmptyElement) { comment="";x->Read(); } else comment=x->ReadElementContentAsString();
	} else if (String::Compare(x->Name,"IMAGEFN")==0) {
		if (x->IsEmptyElement) { imgfilename="";x->Read(); } else imgfilename=x->ReadElementContentAsString();
	} else if (String::Compare(x->Name,"IMAGEFINFO")==0) {
		if (x->IsEmptyElement) { imgfileinfo="";x->Read(); } else imgfileinfo=x->ReadElementContentAsString();
	} else if (String::Compare(x->Name,"FILENAME")==0) {
		if (x->IsEmptyElement) { filename="";x->Read(); } else filename=x->ReadElementContentAsString();
	} else if (String::Compare(x->Name,"CALNAME")==0) {
		if (x->IsEmptyElement) { calname="";x->Read(); } else calname=x->ReadElementContentAsString();
	} else return 0;
	return 1;
}

String ^cframe::getdesc(void) {
	if (name->Length > 0)
		return gcnew String("Frame " + name);
	else
		return gcnew String("Frame");
}

String ^cframe::exportCSV(void) {
	return gcnew String(
		"Overview\n" + 
		"File\t" + filename + "\n" +
		"ImageFile\t" + imgfilename + "\n" +
		"ImageInfo\t" + imgfileinfo + "\n" +
		String::Format("\t{0}\t{1}\n",c->img->width,c->img->height) +
		"Calibration\t" + calname + "\n" +
		"Comment\t" + comment + "\n" +
		"\n");
}

double findscale(double l) {
	double mag=floor(log10(l));
	double ml=pow(10,mag);
	if (ml * 5 <= l) return 5*ml;
	if (ml * 2 <= l) return 2*ml;
	return ml;
}

void cframe::draw(Graphics ^g) {
	vector screen=c->t->screen;
	vector uls=vector(0,0);vector urs=vector(screen.x-1,0);
	vector lrs=vector(screen.x-1,screen.y-1);vector lls=vector(0,screen.y-1);
	vector uld=c->t->todata(uls);vector urd=c->t->todata(urs);
	vector lrd=c->t->todata(lrs);vector lld=c->t->todata(lls);
	vector ulr=c->c->getreal(uld);vector urr=c->c->getreal(urd);
	vector lrr=c->c->getreal(lrd);vector llr=c->c->getreal(lld);

	int fw=frmw;

	Pen ^drawpen;
	//SolidBrush ^fntc;

	if (selected) {
		drawpen=gcnew Pen( Color::FromArgb(255-linecol.R,255-linecol.G,255-linecol.B),2.0f );
	} else {
		drawpen=gcnew Pen( linecol,1.0f );
	}

	SolidBrush ^bgbrush=gcnew SolidBrush(Color::FromArgb(alpha*255,BackCol.R,BackCol.G,BackCol.B));

	//g->SmoothingMode = System::Drawing::Drawing2D::SmoothingMode::AntiAlias;

	switch (frms) {
	case FrameStyles::None:
		fw=0;
		break;
	case FrameStyles::Single:
		g->SmoothingMode = System::Drawing::Drawing2D::SmoothingMode::None;
		g->DrawRectangle(drawpen,(float)uls.x,(float)uls.y,(float)lrs.x-uls.x,(float)lrs.y-uls.y);
		g->SmoothingMode = System::Drawing::Drawing2D::SmoothingMode::AntiAlias;
		fw=0;
		break;
	case FrameStyles::Double:
		g->SmoothingMode = System::Drawing::Drawing2D::SmoothingMode::None;
		g->FillRectangle(bgbrush,(float)uls.x,(float)uls.y,(float)fw,(float)lrs.y-uls.y);
		g->FillRectangle(bgbrush,(float)urs.x-fw,(float)uls.y,(float)fw,(float)lrs.y-uls.y);
		g->FillRectangle(bgbrush,(float)uls.x+fw,(float)uls.y,(float)lrs.x-uls.x-2*fw,(float)fw);
		g->FillRectangle(bgbrush,(float)uls.x+fw,(float)lls.y-fw,(float)lrs.x-uls.x-2*fw,(float)fw);
		g->DrawRectangle(drawpen,(float)uls.x,(float)uls.y,(float)lrs.x-uls.x,(float)lrs.y-uls.y);
		g->DrawRectangle(drawpen,(float)uls.x+fw,(float)uls.y+fw,(float)lrs.x-uls.x-2*fw,(float)lrs.y-uls.y-2*fw);
		g->SmoothingMode = System::Drawing::Drawing2D::SmoothingMode::AntiAlias;
		break;
	case FrameStyles::DoubleWithSections:
		g->SmoothingMode = System::Drawing::Drawing2D::SmoothingMode::None;
		g->FillRectangle(bgbrush,(float)uls.x,(float)uls.y,(float)fw,(float)lrs.y-uls.y);
		g->FillRectangle(bgbrush,(float)urs.x-fw,(float)uls.y,(float)fw,(float)lrs.y-uls.y);
		g->FillRectangle(bgbrush,(float)uls.x+fw,(float)uls.y,(float)lrs.x-uls.x-2*fw,(float)fw);
		g->FillRectangle(bgbrush,(float)uls.x+fw,(float)lls.y-fw,(float)lrs.x-uls.x-2*fw,(float)fw);
		g->DrawRectangle(drawpen,(float)uls.x,(float)uls.y,(float)lrs.x-uls.x,(float)lrs.y-uls.y);
		g->DrawRectangle(drawpen,(float)uls.x+fw,(float)uls.y+fw,(float)lrs.x-uls.x-2*fw,(float)lrs.y-uls.y-2*fw);
		g->SmoothingMode = System::Drawing::Drawing2D::SmoothingMode::AntiAlias;

		int nw=floor(screen.x/50);
		int nh=floor(screen.y/50);
		String ^s=gcnew String("A");
		
		for (int i=0;i<nw;i++) {
			double x=fw+(screen.x-2*fw)*(i+1)/nw;
			char c[2];
			c[0]='A'+i;
			c[1]=0;
			g->SmoothingMode = System::Drawing::Drawing2D::SmoothingMode::None;
			if (i < nw-1) {
				g->DrawLine(drawpen,(float)uls.x+x,(float)uls.y,(float)uls.x+x,(float)uls.y+fw);
				g->DrawLine(drawpen,(float)lls.x+x,(float)lls.y-fw,(float)lls.x+x,(float)lls.y);
			}
			g->SmoothingMode = System::Drawing::Drawing2D::SmoothingMode::AntiAlias;
			drawtxt(g, vector(uls.x+fw+(screen.x-2*fw)*(i+0.5)/nw,uls.y+fw/2), 
				vector(1,0), 0, 0, gcnew String(c));
			drawtxt(g, vector(lls.x+fw+(screen.x-2*fw)*(i+0.5)/nw,lls.y-fw/2), 
				vector(1,0), 0, 0, gcnew String(c));
		}
		for (int i=0;i<nh;i++) {
			double y=fw+(screen.y-2*fw)*(i+1)/nh;
			String ^s=String::Format("{0}",nh-i);
			g->SmoothingMode = System::Drawing::Drawing2D::SmoothingMode::None;
			if (i < nh-1) {
				g->DrawLine(drawpen,(float)uls.x,(float)uls.y+y,(float)uls.x+fw,(float)uls.y+y);
				g->DrawLine(drawpen,(float)urs.x-fw,(float)urs.y+y,(float)urs.x,(float)urs.y+y);
			}
			g->SmoothingMode = System::Drawing::Drawing2D::SmoothingMode::AntiAlias;
			drawtxt(g, vector(uls.x+fw/2,uls.y+fw+(screen.y-2*fw)*(i+0.5)/nh), 
				vector(0,-1), 0, 0, s);
			drawtxt(g, vector(urs.x-fw/2,uls.y+fw+(screen.y-2*fw)*(i+0.5)/nh), 
				vector(0,-1), 0, 0, s);
		}
		break;
	}

	if (showcoord) {
		drawtxt(g,uls+vector(1,1),vector(1,0),-1,-1,c->f->format(ulr));
		drawtxt(g,urs+vector(-1,1),vector(1,0),1,-1,c->f->format(urr));
		drawtxt(g,lrs+vector(-1,-1),vector(1,0),1,1,c->f->format(lrr));
		drawtxt(g,lls+vector(1,-1),vector(1,0),-1,1,c->f->format(llr));
	}

	switch (flds) {
	case FieldStyles::None:
		break;
	case FieldStyles::Top:
	case FieldStyles::Bottom:
		int lines=0;
		if (showcmt) lines++;
		if (showimg) lines++;
		if (showcal) lines++;
		if (showfn) lines++;
		if (showimgfileinfo) lines++;
		if (showscale && lines < 2) lines=2;
		if (lines > 0) {
			int y1,y2;
			int fh=fnt->GetHeight();
			int yh=(lines+1)*fh; 
			if (flds==FieldStyles::Top) 
				{ y1=uls.y+fw;y2=y1+yh; } 
			else
				{ y2=lls.y-fw;y1=y2-yh; } 
			g->SmoothingMode = System::Drawing::Drawing2D::SmoothingMode::None;
			g->FillRectangle(bgbrush,(float)uls.x+fw,(float)y1,(float)lrs.x-uls.x-2*fw,(float)y2-y1);
			g->DrawRectangle(drawpen,(float)uls.x+fw,(float)y1,(float)lrs.x-uls.x-2*fw,(float)y2-y1);
			g->SmoothingMode = System::Drawing::Drawing2D::SmoothingMode::AntiAlias;
			int line=0;
			if (showcmt) {
				drawtxt(g, vector(lls.x+fw+fh,y1+fh+fh*line),vector(1,0),-1,0,
					comment);
				line++; 
			}
			if (showfn) {
				drawtxt(g, vector(lls.x+fw+fh,y1+fh+fh*line),vector(1,0),-1,0,
					gcnew String("File = " + filename));
				line++; 
			}
			if (showimg) {
				drawtxt(g, vector(lls.x+fw+fh,y1+fh+fh*line),vector(1,0),-1,0,
					gcnew String("Image File = " + imgfilename));
				line++; 
			}
			if (showimgfileinfo) {
				drawtxt(g, vector(lls.x+fw+fh,y1+fh+fh*line),vector(1,0),-1,0,
					gcnew String("   " + imgfileinfo));
				line++; 
			}
			if (showcal) {
				drawtxt(g, vector(lls.x+fw+fh,y1+fh+fh*line),vector(1,0),-1,0,
					gcnew String("Calibration = " + calname));
				line++; 
			}

			if (showscale) {
				vector sr0=vector(0,0);
				vector srx=vector(1,0);
				vector sry=vector(0,1);
				vector rr0=c->c->getreal(c->t->todata(sr0));
				vector rrx=c->c->getreal(c->t->todata(srx));
				vector rry=c->c->getreal(c->t->todata(sry));
				vector lx=rrx-rr0;
				vector ly=rry-rr0;
				double l=lx.len;
				double ratio=ly.len/l;
				if (fabs(ratio-1.0)>0.01) {
					// Asymmetric
					// Not possible...
				} else {
					double noml=screen.x/4*l;
					double sf=findscale(noml);
					double screenlen=sf/l;
					double reallen=sf;


					g->DrawRectangle(drawpen,lrs.x-fw-fh-screenlen,y1+fh/2,screenlen,fh*1.5);
					drawtxt(g,vector(lrs.x-fw-fh-screenlen/2,y1+fh/2+0.75*fh),
						vector(1,0),0,0,c->f->format(reallen));
				}

			}
		}
		break;
	}

	delete drawpen;
	delete bgbrush;
}


List<cmeasurement^> ^ cframe::touching(vector datai) {
	vector s=c->t->toscreen(datai);
	int fw=0;
	switch (frms) {
	case FrameStyles::None: fw=1;break;
	case FrameStyles::Single: fw=3; break;
	case FrameStyles::Double:
	case FrameStyles::DoubleWithSections: fw=frmw+3; break;
	}

	if ((s.x <= fw) || (s.y <= fw) || 
		(s.x >= c->t->screen.x-fw) || (s.y >= c->t->screen.y-fw)) {
		List<cmeasurement^> ^l=gcnew List<cmeasurement^>();
		l->Add(this);
		return l;
	}
	return nullptr;
}

List<cmeasurement^> ^cframe::isinside(vector data1, vector data2) {
	return nullptr;
}

void cframe::moveto(vector data_from, vector data_to) {
}

void cframe::storestate() {
}

void cframe::restorestate() {
}

//************************************************* C R E F P O I N T ***********************************************************

void crefpoint::crefpoint_default() {
	data=vector(0,0);
	pts=PointStyles::Square;
}

crefpoint::crefpoint(vector dtv, csys ^c):cmeasurement(gcnew String("RefPoint"),c) {
	crefpoint_default();
	data=dtv;
	callback=nullptr;
}

crefpoint::crefpoint(csys ^c, ia_xml_read ^x):cmeasurement(gcnew String("RefPoint"),c)  {
	int datafound,rf;
	datafound=0;

	crefpoint_default();

	callback=nullptr;
	while (1) {
		switch (x->NodeType) {
			case XmlNodeType::Element:
				if (rf=read_from_xml(x)) {
					if (rf & 0x02) datafound=1;
				} else {
					// Unknown element --> Skip over
					if (x->IsEmptyElement)	x->Read(); else x->Skip();
				}
				break;
			case XmlNodeType::EndElement:
				if (!datafound) 
					throw gcnew System::Exception("Bad XML Formatted File <REFPOINT> tag");
				x->Read();
				return;
			default:
				throw gcnew System::Exception("Bad XML Formatted File");
		}
	}
}

void crefpoint::write_to_xml(ia_xml_write ^x) {
	cmeasurement::write_to_xml(x);
	x->write_value("DATAVECTOR",data);
	x->write_value("POINTSTYLE",(int)pts);
}

int crefpoint::read_from_xml(ia_xml_read ^x) {
	if (cmeasurement::read_from_xml(x)) return 1;

	if (String::Compare(x->Name,"DATAVECTOR")==0) {
		x->Read();data=x->xml_read_vector();
		return 0x02;
	} else if (String::Compare(x->Name,"POINTSTYLE")==0) {
		pts=(PointStyles)x->ReadElementContentAsInt();
		return 1;
	} else return 0;
	return 1;
}

void crefpoint::setcallback(cmeasurement ^cbk) {
	callback=cbk;
}

String ^crefpoint::getdesc(void) {
	if (name->Length > 0)
		return gcnew String(name + " = " + c->f->format(c->c->getreal(data)));
	else
		return gcnew String(c->f->format(c->c->getreal(data)));
}

void crefpoint::draw(Graphics ^g) {
	vector m=c->t->toscreen(data);
	Pen ^drawpen;

	if (m.len > 1e3) return;

	drawpen=gcnew Pen( linecol, 1.0f);

	switch (pts) {
	case PointStyles::None:
		break;
	case PointStyles::Dot: 
		g->DrawArc(drawpen,(float)m.x-2.0,(float)m.y-2.0,(float)4.0,(float)4.0,0.0,360.0);
		break;
	case PointStyles::Plus: 
		g->DrawLine(drawpen,(float)m.x,(float)m.y-5,(float)m.x,(float)m.y+5);
		g->DrawLine(drawpen,(float)m.x-5,(float)m.y,(float)m.x+5,(float)m.y);
		break;
	case PointStyles::LargePlus:
		g->DrawLine(drawpen,(float)m.x,(float)m.y-10,(float)m.x,(float)m.y+10);
		g->DrawLine(drawpen,(float)m.x-10,(float)m.y,(float)m.x+10,(float)m.y);
		break;
	case PointStyles::X:
		g->DrawLine(drawpen,(float)m.x-5,(float)m.y-5,(float)m.x+5,(float)m.y+5);
		g->DrawLine(drawpen,(float)m.x-5,(float)m.y+5,(float)m.x+5,(float)m.y-5);
		break;
	case PointStyles::LargeX:
		g->DrawLine(drawpen,(float)m.x-10,(float)m.y-10,(float)m.x+10,(float)m.y+10);
		g->DrawLine(drawpen,(float)m.x-10,(float)m.y+10,(float)m.x+10,(float)m.y-10);
		break;
	case PointStyles::Circle:
		g->DrawArc(drawpen,(float)m.x-5.0,(float)m.y-5.0,(float)10.0,(float)10.0,0.0,360.0);
		break;
	case PointStyles::LargeCircle:
		g->DrawArc(drawpen,(float)m.x-10.0,(float)m.y-10.0,(float)20.0,(float)20.0,0.0,360.0);
		break;
	case PointStyles::Square:
		g->DrawRectangle(drawpen,(float)m.x-5.0,(float)m.y-5.0,(float)10.0,(float)10.0);
		break;
	case PointStyles::LargeSquare:
		g->DrawRectangle(drawpen,(float)m.x-10.0,(float)m.y-10.0,(float)20.0,(float)20.0);
		break;
	}

	// g->DrawRectangle(drawpen,(float)m.x-5.0,(float)m.y-5.0,(float)11.0,(float)11.0);
	delete drawpen;

}

List<cmeasurement^> ^ crefpoint::touching(vector datai) {
	vector s=c->t->toscreen(data);
	vector screen=c->t->toscreen(datai);
	if ((s.x >= screen.x - 3) && (s.x <= screen.x + 3) && 
		(s.y >= screen.y - 3) && (s.y <= screen.y + 3)) {
		List<cmeasurement^> ^l=gcnew List<cmeasurement^>();
		l->Add(this);
		return l;
	}
	return nullptr;
}

List<cmeasurement^> ^crefpoint::isinside(vector data1, vector data2) {
	vector m1=data1;//c->t->todata(screen1);
	vector m2=data2;//c->t->todata(screen2);
	if ((m1.x <= m2.x) && ((data.x < m1.x) || (data.x > m2.x))) return nullptr;
	if ((m1.x >= m2.x) && ((data.x > m1.x) || (data.x < m2.x))) return nullptr;
	if ((m1.y <= m2.y) && ((data.y < m1.y) || (data.y > m2.y))) return nullptr;
	if ((m1.y >= m2.y) && ((data.y > m1.y) || (data.y < m2.y))) return nullptr;
	// Its inside
	List<cmeasurement^> ^l=gcnew List<cmeasurement^>();
	l->Add(this);
	return l;
}

void crefpoint::moveto(vector data_from, vector data_to) {
	data=store_data-data_from+data_to;
	if (callback != nullptr)
		callback->refchangenotify(this);
}

void crefpoint::storestate() {
	store_data=data;
}

void crefpoint::restorestate() {
	data=store_data;
}

//************************************************* C A N N O T A T I O N ***********************************************************

void cannotation::cannotation_default() {
	p1=nullptr;
	p2=nullptr;
	bgcol=Color::FromArgb(200,200,255);
	bgalp=0.5;
	bgfill=1;
	lm=LineMode::None;
	lw=2;
	ha=HAlign::Center;
	va=VAlign::Center;
	shp=Shape::Rectangle;
	rratio=0.5;
	text="";
	textonline=0;
	fntfrmuse=0;
	fntcol=Color::FromArgb(0,0,0);
}

cannotation::cannotation(vector pt1, vector pt2, csys ^c):cmeasurement(gcnew String("annotation"),c) {
	cannotation_default();
	p1=gcnew crefpoint(pt1,c);
	p2=gcnew crefpoint(pt2,c);
}

cannotation::cannotation(csys ^c, ia_xml_read ^x):cmeasurement(gcnew String("annotation"),c)  {
	int d1found,d2found,rf;
	d1found=d2found=0;

	cannotation_default();

	while (1) {
		switch (x->NodeType) {
			case XmlNodeType::Element:
				if (rf=read_from_xml(x)) {
					if (rf & 0x02) d1found=1;
					if (rf & 0x04) d2found=1;
				} else {
					// Unknown element --> Skip over
					if (x->IsEmptyElement)	x->Read(); else x->Skip();
				}
				break;
			case XmlNodeType::EndElement:
				if (!d1found || !d2found) 
					throw gcnew System::Exception("Bad XML Formatted File <ANNOTATION> tag");
				x->Read();
				return;
			default:
				throw gcnew System::Exception("Bad XML Formatted File <ANNOTATION> tag");
		}
	}
}

void cannotation::update_from_template(cmeasurement ^templ) {
	if (templ == nullptr) return;
	cmeasurement::update_from_template(templ);
	bgcol=((cannotation^)templ)->bgcol;
	bgalp=((cannotation^)templ)->bgalp;
	bgfill=((cannotation^)templ)->bgfill;
	lm=((cannotation^)templ)->lm;
	lw=((cannotation^)templ)->lw;
	ha=((cannotation^)templ)->ha;
	va=((cannotation^)templ)->va;
	shp=((cannotation^)templ)->shp;
	textonline=((cannotation^)templ)->textonline;
	rratio=((cannotation^)templ)->rratio;
	text=gcnew String(((cannotation^)templ)->text);
}


void cannotation::write_to_xml(ia_xml_write ^x) {
	cmeasurement::write_to_xml(x);
	x->WriteStartElement("P1");
		p1->write_to_xml(x);
	x->WriteEndElement();x->WriteWhitespace("\n");
	x->WriteStartElement("P2");
		p2->write_to_xml(x);
	x->WriteEndElement();x->WriteWhitespace("\n");
	x->write_value("BGCOL",bgcol);
	x->write_value("BGALPHA",bgalp);
	x->write_value("FILLBACKGROUND",bgfill);
	x->write_value("LINEMODE",(int)lm);
	x->write_value("LINEWIDTH",lw);
	x->write_value("HALIGN",(int)ha);
	x->write_value("VALIGN",(int)va);
	x->write_value("TEXTONLINE",textonline);
	x->write_value("SHAPE",(int)shp);
	x->write_value("RRATIO",rratio);
	x->write_value("TEXT",text);
}

int cannotation::read_from_xml(ia_xml_read ^x) {
	if (cmeasurement::read_from_xml(x)) return 1;

	if (String::Compare(x->Name,"P1")==0) {
		x->Read();p1=gcnew crefpoint(c,x);
		return 0x02;
	} else if (String::Compare(x->Name,"P2")==0) {
		x->Read();p2=gcnew crefpoint(c,x);
		return 0x04;
	} else if (String::Compare(x->Name,"BGCOL")==0) {
		x->Read();bgcol=x->xml_read_color();
	} else if (String::Compare(x->Name,"BGALPHA")==0) {
		bgalp=x->ReadElementContentAsDouble();
	} else if (String::Compare(x->Name,"FILLBACKGROUND")==0) {
		bgfill=x->ReadElementContentAsInt();
	} else if (String::Compare(x->Name,"LINEMODE")==0) {
		lm=(image_add::LineMode) x->ReadElementContentAsInt();
	} else if (String::Compare(x->Name,"LINEWIDTH")==0) {
		lw=x->ReadElementContentAsDouble();
	} else if (String::Compare(x->Name,"HALIGN")==0) {
		ha=(image_add::HAlign) x->ReadElementContentAsInt();
	} else if (String::Compare(x->Name,"VALIGN")==0) {
		va=(image_add::VAlign) x->ReadElementContentAsInt();
	} else if (String::Compare(x->Name,"TEXTONLINE")==0) {
		textonline=x->ReadElementContentAsInt();
	} else if (String::Compare(x->Name,"SHAPE")==0) {
		shp=(image_add::Shape) x->ReadElementContentAsInt();
	} else if (String::Compare(x->Name,"RRATIO")==0) {
		rratio=x->ReadElementContentAsDouble();
	} else if (String::Compare(x->Name,"TEXT")==0) {
		if (x->IsEmptyElement) { text="";x->Read(); } else text=x->ReadElementContentAsString();
	} else return 0;
	return 1;
}

String ^cannotation::getdesc(void) {
	if (name->Length > 0)
		return gcnew String("Annotation[" + name + "] = " + text);
	else
		return gcnew String("Annotation = " + text);
}

void dblbezier(Drawing2D::GraphicsPath ^gp, 
	double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4) {
		gp->AddBezier((float)x1,(float)y1,
			(float)x2,(float)y2,
			(float)x3,(float)y3,
			(float)x4,(float)y4);
}

Drawing2D::GraphicsPath ^create_rounded_rect(vector c, vector dim, double rr) {
	Drawing2D::GraphicsPath ^gp=gcnew Drawing2D::GraphicsPath();
	dim.x=fabs(dim.x);
	dim.y=fabs(dim.y);
	double mindim=(dim.x<dim.y)?dim.x:dim.y;
	if (rr < 0) rr=0;
	if (rr > 0.99) rr=0.99;
	double rad=mindim/2*rr;

	dblbezier(gp,
		c.x+dim.x/2,		c.y-dim.y/2+rad,
		c.x+dim.x/2,		c.y-dim.y/2,
		c.x+dim.x/2,		c.y-dim.y/2,
		c.x+dim.x/2-rad,	c.y-dim.y/2);
	dblbezier(gp,
		c.x-dim.x/2+rad,	c.y-dim.y/2,
		c.x-dim.x/2,		c.y-dim.y/2,
		c.x-dim.x/2,		c.y-dim.y/2,
		c.x-dim.x/2,		c.y-dim.y/2+rad);
	dblbezier(gp,
		c.x-dim.x/2,		c.y+dim.y/2-rad,
		c.x-dim.x/2,		c.y+dim.y/2,
		c.x-dim.x/2,		c.y+dim.y/2,
		c.x-dim.x/2+rad,	c.y+dim.y/2);
	dblbezier(gp,
		c.x+dim.x/2-rad,	c.y+dim.y/2,
		c.x+dim.x/2,		c.y+dim.y/2,
		c.x+dim.x/2,		c.y+dim.y/2,
		c.x+dim.x/2,		c.y+dim.y/2-rad);
	gp->CloseFigure();
	return gp;
}

void cannotation::draw(Graphics ^g) {
	vector m1=c->screen_from_data(p1->data);
	vector m2=c->screen_from_data(p2->data);
	vector v,vy;
	Pen ^drawpen;
	double drawwidth;
	Color drawcol;
	Drawing2D::GraphicsPath ^gp=nullptr;

	// if (!selected) return;

	//	drawpen=gcnew Pen( PointCol,1.0f );
	if (selected) {
		drawcol=Color::FromArgb(255-linecol.R,255-linecol.G,255-linecol.B);
		if (lw < 1) drawwidth=1;
		else drawwidth=lw;
	} else {
		drawcol=linecol;
		drawwidth=lw;
	}

	if (lm==LineMode::None) {
		if (!selected)  drawwidth=0;
	}			

	if ((shp == Shape::Line) || (shp == Shape::Arrow)) {
		if (drawwidth < 1) drawwidth=1; // no invisible line for line or Arrow!
	}

	if (selected) {
		p1->draw(g);
		p2->draw(g);
	}

	double minx=(m1.x<m2.x)?m1.x:m2.x;
	double miny=(m1.y<m2.y)?m1.y:m2.y;
	double maxx=(m1.x<m2.x)?m2.x:m1.x;
	double maxy=(m1.y<m2.y)?m2.y:m1.y;
	array<Single>^pdash = {4.0F,4.0F};
	array<Single>^pdashdot = {4.0F,4.0F,1.0F,4.0F};
	array<Single>^pdot = {1.0F,4.0F};

	drawpen=gcnew Pen( drawcol, drawwidth);
	switch (lm) {
	case LineMode::Solid:
		drawpen->DashStyle = Drawing2D::DashStyle::Solid;
		break;
	case LineMode::Dash:
		drawpen->DashStyle = Drawing2D::DashStyle::Custom;
		drawpen->DashPattern=pdash;
//		drawpen->DashStyle = Drawing2D::DashStyle::Dash;
		break;
	case LineMode::DashDot:
		drawpen->DashStyle = Drawing2D::DashStyle::Custom;
		drawpen->DashPattern=pdashdot;
//		drawpen->DashStyle = Drawing2D::DashStyle::DashDot;
		break;
	case LineMode::None:
	case LineMode::Dot:
		drawpen->DashStyle = Drawing2D::DashStyle::Custom;
		drawpen->DashPattern=pdot;
//		drawpen->DashStyle = Drawing2D::DashStyle::Dot;
		break;
	}

	if (((bgfill && (bgalp > 0)) || (drawwidth > 0)) && (shp == Shape::RoundedRectangle)) {
		gp=create_rounded_rect(vector((minx+maxx)/2,(miny+maxy)/2),
				vector(maxx-minx,maxy-miny),rratio);
	}

	if (bgfill && (bgalp > 0)) {
		Brush ^b=gcnew SolidBrush(Color::FromArgb((int)floor(bgalp*255+0.5),bgcol.R,bgcol.G,bgcol.B));
		switch (shp) {
		case Shape::Line:
			break;
		case Shape::Arrow:
			break;
		case Shape::Rectangle:
			g->FillRectangle(b,(float)minx,(float)miny,(float)maxx-minx+1,(float)maxy-miny+1);
			break;
		case Shape::RoundedRectangle:
			g->FillPath(b,gp);
			break;
		case Shape::Ellipse:
			g->FillEllipse(b,(float)minx,(float)miny,(float)maxx-minx+1,(float)maxy-miny+1);
			break;
		}
		delete b;
	}

	if (drawwidth > 0) {
		switch (shp) {
		case Shape::Line:
			drawline(g,m1,m2,drawpen);
			break;
		case Shape::Arrow:
			drawline(g,m1,m2,drawpen);
			v=norm(m2-m1);
			vy=vector(-v.y,v.x);
			drawpen->DashStyle = Drawing2D::DashStyle::Solid;
			drawline(g,m2,m2-8*v+5*vy,drawpen);
			drawline(g,m2,m2-8*v-5*vy,drawpen);
			break;
		case Shape::Rectangle:
			g->DrawRectangle(drawpen,(float)minx,(float)miny,(float)maxx-minx+1,(float)maxy-miny+1);
			break;
		case Shape::RoundedRectangle:
			g->DrawPath(drawpen,gp);
			break;
		case Shape::Ellipse:
			g->DrawEllipse(drawpen,(float)minx,(float)miny,(float)maxx-minx+1,(float)maxy-miny+1);
			break;
		}
	}

	if (gp != nullptr) delete gp;

	if (text->Length > 0) {
		int alx,aly;alx=aly=0;
		switch (ha) {
		case HAlign::Left: alx=-1;break;
		case HAlign::Center: alx=0;break;
		case HAlign::Right: alx=1;break;
		}
		switch (va) {
		case VAlign::Top: aly=-1;break;
		case VAlign::Center: aly=0;break;
		case VAlign::Bottom: aly=1;break;
		}
		if ((shp == Shape::Line) || (shp == Shape::Arrow)) {
			if (textonline) {
				vector ctr=(m1+m2)/2;
				vector vx=norm(m2-m1);
				switch(alx) {
				case -1: drawtxt(g,m1,vx,alx,aly,text);break;
				case 0: drawtxt(g,ctr,vx,alx,aly,text);break;
				case 1: drawtxt(g,m2,vx,alx,aly,text);break;
				}
			} else {
				int phi=(int)floor((atan2(m1.y-m2.y,m1.x-m2.x)+M_PI/8)/(M_PI/4));
				switch (phi) {
				case -4: alx=1;aly=0;break;
				case -3: alx=1;aly=1;break;
				case -2: alx=0;aly=1;break;
				case -1: alx=-1;aly=1;break;
				case 0: alx=-1;aly=0;break;
				case 1: alx=-1;aly=-1;break;
				case 2: alx=0;aly=-1;break;
				case 3: alx=1;aly=-1;break;
				case 4: alx=1;aly=0;break;
				case 5: alx=1;aly=1;break;
				}
				drawtxtbox(g,m1.x,m1.y,m1.x,m1.y,alx,aly,text);
			}
		} else {
			drawtxtbox(g,minx,miny,maxx,maxy,alx,aly,text);
		}
		//delete stext;
	}

	delete drawpen;
}

List<cmeasurement^> ^ cannotation::touching(vector datai) {
	List<cmeasurement^> ^l;
	vector m1=c->screen_from_data(p1->data);
	vector m2=c->screen_from_data(p2->data);
	vector s=c->t->toscreen(datai);
	double minx=(m1.x<m2.x)?m1.x:m2.x;
	double miny=(m1.y<m2.y)?m1.y:m2.y;
	double maxx=(m1.x<m2.x)?m2.x:m1.x;
	double maxy=(m1.y<m2.y)?m2.y:m1.y;

	if (l=p1->touching(datai)) { return l; }
	if (l=p2->touching(datai)) { return l; }

	if ((shp == Shape::Line) || (shp == Shape::Arrow)) {
		double ldx=getinlinedist(m1,m2,s);
		if (fabs(ldx) < 3) {
			l=gcnew List<cmeasurement^>();
			l->Add(this);
			return l;
		}
	} else {
		if ((s.x >= minx - 3) && (s.x <= maxx + 3) && 
			(s.y >= miny - 3) && (s.y <= maxy + 3)) {
			l=gcnew List<cmeasurement^>();
			l->Add(this);
			return l;
		}
	}
	return nullptr;
}

List<cmeasurement^> ^cannotation::isinside(vector data1, vector data2) {
	List<cmeasurement^> ^l;
	List<cmeasurement^> ^lt;
	//vector m1=c->screen_from_data(p1->data);
	//vector m2=c->screen_from_data(p2->data);
	//vector screen=c->t->toscreen(datai);
	//double minx=(m1.x<m2.x)?m1.x:m2.x;
	//double miny=(m1.y<m2.y)?m1.y:m2.y;
	//double maxx=(m1.x<m2.x)?m2.x:m1.x;
	//double maxy=(m1.y<m2.y)?m2.y:m1.y;

	l=gcnew List<cmeasurement^>();

	if (lt=p1->isinside(data1,data2)) { l->AddRange(lt); }
	if (lt=p2->isinside(data1,data2)) { l->AddRange(lt); }

	if (l->Count > 0) {
		if (l->Count == 2) {
			// All inside
			delete l;
			l=gcnew List<cmeasurement^>();
			l->Add(this);
			return l;
		}
		return l;
	}
	delete l;
	return nullptr;
}

void cannotation::moveto(vector data_from, vector data_to) {
	if (selected) {
		p1->moveto(data_from,data_to);
		p2->moveto(data_from,data_to);
	} else {
		if (p1->selected) p1->moveto(data_from,data_to);
		if (p2->selected) p2->moveto(data_from,data_to);
	}
}

void cannotation::storestate() {
	p1->storestate();
	p2->storestate();
}

void cannotation::restorestate() {
	p1->restorestate();
	p2->restorestate();
}

//******************************************************* C P O I N T ***********************************************************

void cpoint::cpoint_default() {
	isref=0;refsys=nullptr;
	data=vector(0,0);
	root=nullptr;
	if ((c!=nullptr) && (c->c != nullptr))
		real=c->c->getreal(data);
	else
		real=vector(0,0);
	dmode=PointDisplayMode::Coordinates;
	pts=PointStyles::Plus;

}

cpoint::cpoint(vector dtv, csys ^c):cmeasurement("Point",c) {
	cpoint_default();
	data=dtv;
	if ((c!=nullptr) && (c->c != nullptr))
		real=c->c->getreal(data);
	else
		real=vector(0,0);
}

cpoint::cpoint(vector dtv, vector re, csys ^c, coordinate ^cr):cmeasurement("Point",c) {
	cpoint_default();
	data=dtv;
	isref=1;refsys=cr;
	real=re;
	root=cr;
}

cpoint::cpoint(csys ^c, ia_xml_read ^x):cmeasurement("Point",c) {
	int datafound,realfound,rv;
	datafound=realfound=0;

	cpoint_default();
	
	while (1) {
		switch (x->NodeType) {
			case XmlNodeType::Element:
				if (rv=read_from_xml(x)) {
					if (rv & 0x02) datafound=1;
					if (rv & 0x04) realfound=1;
				} else {
					// Unknown element --> Skip over
					if (x->IsEmptyElement)	x->Read(); else x->Skip();
				}
				break;
			case XmlNodeType::EndElement:
				if (!datafound || !realfound) 
					throw gcnew System::Exception("Bad XML Formatted File <POINT> tag");
				x->Read();
				return;
			default:
				throw gcnew System::Exception("Bad XML Formatted File");
		}
	}
}

void cpoint::write_to_xml(ia_xml_write ^x) {
	cmeasurement::write_to_xml(x);
	x->write_value("DATAVECTOR",data);
	x->write_value("REALVECTOR",real);
	// x->write_value("SHOWCOORD",showcoord);
	x->write_value("DISPLAYMODE",(int)dmode);
	x->write_value("POINTSTYLE",(int)pts);
}

int cpoint::read_from_xml(ia_xml_read ^x) {
	if (cmeasurement::read_from_xml(x)) return 1;

	if (String::Compare(x->Name,"DATAVECTOR")==0) {
		x->Read();data=x->xml_read_vector();
		return 0x02;
	} else if (String::Compare(x->Name,"REALVECTOR")==0) {
		x->Read();real=x->xml_read_vector();
		return 0x04;
	} else if (String::Compare(x->Name,"SHOWCOORD")==0) {
		// Compatibility
		if (x->ReadElementContentAsInt())
			dmode=PointDisplayMode::Coordinates;
	} else if (String::Compare(x->Name,"DISPLAYMODE")==0) {
		dmode=(image_add::PointDisplayMode) x->ReadElementContentAsInt();
	} else if (String::Compare(x->Name,"POINTSTYLE")==0) {
		pts=(image_add::PointStyles) x->ReadElementContentAsInt();
	} else return 0;
	return 1;
}


void cpoint::update_from_template(cmeasurement ^templ) {
	if (templ == nullptr) return;
	cmeasurement::update_from_template(templ);
	// showcoord=((cpoint^)templ)->showcoord;
	dmode=((cpoint^)templ)->dmode;
	pts=((cpoint^)templ)->pts;
}

String ^cpoint::getdesc(void) {
	if (name->Length > 0)
		return gcnew String(name + " = " + c->f->format(real));
	else
		return gcnew String(c->f->format(real));
}

String ^cpoint::exportCSV(void) {
	if (isref) return String::Format("Point\t{0}\t{1}\t{2}\n",name,real.x,real.y);
	vector r=c->c->getreal(data);
	return String::Format("Point\t{0}\t{1}\t{2}\n",name,r.x,r.y);
}

void cpoint::update_data(void) {
	if (isref) {
		// Move Ref System
		refsys->calctrf();
	} else {
		// Move point
		data=c->c->getdata(real);
	}
}

void cpoint::draw(Graphics ^g) {
	vector m=c->t->toscreen(data);

	Pen ^drawpen;
	if (selected) 
		drawpen=gcnew Pen( Color::FromArgb(255-linecol.R,255-linecol.G,255-linecol.B),2.0f );
	else
		drawpen=gcnew Pen( linecol,1.0f );
	switch (pts) {
	case PointStyles::None:
		break;
	case PointStyles::Dot: 
		g->DrawArc(drawpen,(float)m.x-2.0,(float)m.y-2.0,(float)4.0,(float)4.0,0.0,360.0);
		break;
	case PointStyles::Plus: 
		g->DrawLine(drawpen,(float)m.x,(float)m.y-5,(float)m.x,(float)m.y+5);
		g->DrawLine(drawpen,(float)m.x-5,(float)m.y,(float)m.x+5,(float)m.y);
		break;
	case PointStyles::LargePlus:
		g->DrawLine(drawpen,(float)m.x,(float)m.y-10,(float)m.x,(float)m.y+10);
		g->DrawLine(drawpen,(float)m.x-10,(float)m.y,(float)m.x+10,(float)m.y);
		break;
	case PointStyles::X:
		g->DrawLine(drawpen,(float)m.x-5,(float)m.y-5,(float)m.x+5,(float)m.y+5);
		g->DrawLine(drawpen,(float)m.x-5,(float)m.y+5,(float)m.x+5,(float)m.y-5);
		break;
	case PointStyles::LargeX:
		g->DrawLine(drawpen,(float)m.x-10,(float)m.y-10,(float)m.x+10,(float)m.y+10);
		g->DrawLine(drawpen,(float)m.x-10,(float)m.y+10,(float)m.x+10,(float)m.y-10);
		break;
	case PointStyles::Circle:
		g->DrawArc(drawpen,(float)m.x-5.0,(float)m.y-5.0,(float)10.0,(float)10.0,0.0,360.0);
		break;
	case PointStyles::LargeCircle:
		g->DrawArc(drawpen,(float)m.x-10.0,(float)m.y-10.0,(float)20.0,(float)20.0,0.0,360.0);
		break;
	case PointStyles::Square:
		g->DrawRectangle(drawpen,(float)m.x-5.0,(float)m.y-5.0,(float)10.0,(float)10.0);
		break;
	case PointStyles::LargeSquare:
		g->DrawRectangle(drawpen,(float)m.x-10.0,(float)m.y-10.0,(float)20.0,(float)20.0);
		break;
	}


	delete drawpen;

	SolidBrush ^drawbrush=gcnew SolidBrush(fntcol);

	if (name->Length > 0) {
		drawtxt(g,m,vector(1,0),-1,1,name);
	}

	switch (dmode) {
	case PointDisplayMode::Coordinates: 
	case PointDisplayMode::CoordinatesAndColor: 
	case PointDisplayMode::CoordinatesAndColorValue:
		if (!isref) real=c->c->getreal(data);
		break;
	}

	int colvalid=0;
	int valvalid=0;
	int cr,cg,cb;
	double colval;

	switch (dmode) {
	case PointDisplayMode::Color:
	case PointDisplayMode::CoordinatesAndColor: 
		{ 
			colvalid=1;
			vector ip=c->image_from_data(data);
			int rx=(int)floor(ip.x+0.5);
			int ry=(int)floor(ip.y+0.5);
			if (!c->img->getcolor(rx,ry,&cr,&cg,&cb)) colvalid=0;
		}
		break;
	case PointDisplayMode::ColorValue:
	case PointDisplayMode::CoordinatesAndColorValue:
		{
			colvalid=1;
			vector ip=c->image_from_data(data);
			int rx=(int)floor(ip.x+0.5);
			int ry=(int)floor(ip.y+0.5);
			if (!c->img->getcolor(rx,ry,&cr,&cg,&cb)) colvalid=0;
			
			valvalid=colvalid;
			if (valvalid && (c->cbar == nullptr)) valvalid=0;
			if (valvalid && (c->cbar->prf == nullptr)) valvalid=0;

			if (valvalid) {
				colval=c->cbar->prf->findcolor((double)cr/255.0,(double)cg/255.0,(double)cb/255.0);
				if (colval >= 0) {
					colval=c->cbar->v1+(colval-c->cbar->o1)/(c->cbar->o2-c->cbar->o1)*(c->cbar->v2-c->cbar->v1);
				} else valvalid=0;
			}
		}
		break;
	}

	switch (dmode) {
	case PointDisplayMode::PointOnly: break;
	case PointDisplayMode::Coordinates: 
		drawtxt(g,m,vector(1,0),-1,-1,c->f->format(real));
		break;
	case PointDisplayMode::Color:
		if (colvalid)
			drawtxt(g,m,vector(1,0),-1,1,c->f->colorformat(cr,cg,cb));
		break;
	case PointDisplayMode::ColorValue:
		if (valvalid)
			drawtxt(g,m,vector(1,0),-1,1,c->f->cformat(colval));
		break;
	case PointDisplayMode::CoordinatesAndColor: 
		drawtxt(g,m,vector(1,0),-1,-1,c->f->format(real));
		if (colvalid)
			drawtxt(g,m,vector(1,0),-1,1,c->f->colorformat(cr,cg,cb));
		break;
	case PointDisplayMode::CoordinatesAndColorValue:
		drawtxt(g,m,vector(1,0),-1,-1,c->f->format(real));
		if (valvalid)
			drawtxt(g,m,vector(1,0),-1,1,c->f->cformat(colval));
		break;
	}
	delete drawbrush;
}

List<cmeasurement^> ^ cpoint::touching(vector datai) {
	vector s=c->t->toscreen(data);
	vector screen=c->t->toscreen(datai);
	if ((s.x >= screen.x - 3) && (s.x <= screen.x + 3) && 
		(s.y >= screen.y - 3) && (s.y <= screen.y + 3)) {
		List<cmeasurement^> ^l=gcnew List<cmeasurement^>();
		l->Add(this);
		return l;
	}
	return nullptr;
}

List<cmeasurement^> ^cpoint::isinside(vector data1, vector data2) {
	vector m1=data1;//c->t->todata(screen1);
	vector m2=data2;//c->t->todata(screen2);
	if ((m1.x <= m2.x) && ((data.x < m1.x) || (data.x > m2.x))) return nullptr;
	if ((m1.x >= m2.x) && ((data.x > m1.x) || (data.x < m2.x))) return nullptr;
	if ((m1.y <= m2.y) && ((data.y < m1.y) || (data.y > m2.y))) return nullptr;
	if ((m1.y >= m2.y) && ((data.y > m1.y) || (data.y < m2.y))) return nullptr;
	// Its inside
	List<cmeasurement^> ^l=gcnew List<cmeasurement^>();
	l->Add(this);
	return l;
}

void cpoint::moveto(vector data_from, vector data_to) {
	data=store_data-data_from+data_to;
	if (isref == 0)
		real=c->c->getreal(data);
	else 
		refsys->calctrf();
}

void cpoint::storestate() {
	store_data=data;
}

void cpoint::restorestate() {
	data=store_data;
	if (!isref) real=c->c->getreal(data);
}


//******************************************************* C L I N E A R ***********************************************************

void clinear::clinear_default() {
	p1=nullptr;
	p2=nullptr;
	pp=nullptr;
	offset=0;
	//FillBGCol=Color::FromArgb(127,0,0,0);
	//fillbackground=0;
	drawrefpoints=1;
	showticks=1;
	va=VAlign::Bottom;
}

clinear::clinear(vector data1, vector data2, csys ^c):cmeasurement(gcnew String("Line"),c) {
	clinear_default();
	p1=gcnew cpoint(data1, c);
	p2=gcnew cpoint(data2, c);
	p1->dmode=PointDisplayMode::PointOnly;p1->pts=PointStyles::Dot;
	p2->dmode=PointDisplayMode::PointOnly;p2->pts=PointStyles::Dot;
	pp=gcnew crefpoint((data1+data2)/2,c);
	pp->setcallback(this);
}

clinear::clinear(csys ^c, ia_xml_read ^x):cmeasurement(gcnew String("Line"),c) {
	int foundp1,foundp2,rv;
	foundp1=foundp2=0;

	clinear_default();

	while (1) {
		switch (x->NodeType) {
			case XmlNodeType::Element:
				if (rv = read_from_xml(x)) {
					if (rv & 0x02) foundp1=1;
					if (rv & 0x04) foundp2=1;
				} else {
					// Unknown element --> Skip over
					if (x->IsEmptyElement)	x->Read(); else x->Skip();
				}
				break;
			case XmlNodeType::EndElement:
				if (!foundp1 || !foundp2) 
					throw gcnew System::Exception("Bad XML Formatted File <POINT> tag");
				x->Read();
				pp=gcnew crefpoint((p1->data+p2->data)/2,c);
				pp->setcallback(this);
				return;
			default:
				throw gcnew System::Exception("Bad XML Formatted File");
		}
	}
}
	
void clinear::write_to_xml(ia_xml_write ^x) {
	cmeasurement::write_to_xml(x);
	x->WriteStartElement("P1");
		p1->write_to_xml(x);
	x->WriteEndElement();x->WriteWhitespace("\n");
	x->WriteStartElement("P2");
		p2->write_to_xml(x);
	x->WriteEndElement();x->WriteWhitespace("\n");
	x->write_value("OFFSET",offset);
	//x->write_value("FILLBGCOL",FillBGCol);
	//x->write_value("FILLBACKGROUND",fillbackground);
	x->write_value("DRAWREFPOINTS",drawrefpoints);
	x->write_value("SHOWTICKS",showticks);
	x->write_value("VALIGN",(int)va);
	calc_ref_point();
}

int clinear::read_from_xml(ia_xml_read ^x) {
	if (cmeasurement::read_from_xml(x)) return 1;

	if (String::Compare(x->Name,"P1")==0) {
		x->Read();
		p1=gcnew cpoint(c,x);
		return 0x02;
	} else if (String::Compare(x->Name,"P2")==0) {
		x->Read();
		p2=gcnew cpoint(c,x);
		return 0x04;
	//} else if (String::Compare(x->Name,"FILLBACKGROUND")==0) {
	//	fillbackground=x->ReadElementContentAsInt();
	} else if (String::Compare(x->Name,"OFFSET")==0) {
		offset=x->ReadElementContentAsDouble();
	} else if (String::Compare(x->Name,"DRAWREFPOINTS")==0) {
		drawrefpoints=x->ReadElementContentAsInt();
	} else if (String::Compare(x->Name,"SHOWTICKS")==0) {
		showticks=x->ReadElementContentAsInt();
	} else if (String::Compare(x->Name,"VALIGN")==0) {
		va=(image_add::VAlign)x->ReadElementContentAsInt();
	//} else if (String::Compare(x->Name,"FILLBGCOL")==0) {
	//	x->Read();FillBGCol=x->xml_read_color();
	} else return 0;
	return 1;
}
	
void clinear::update_from_template(cmeasurement ^templ) {
	if (templ == nullptr) return;
	cmeasurement::update_from_template(templ);

	//FillBGCol=((clinear^)templ)->FillBGCol;
	//fillbackground=((clinear^)templ)->fillbackground;
	drawrefpoints=((clinear^)templ)->drawrefpoints;
	showticks=((clinear^)templ)->showticks;
	va=((clinear^)templ)->va;
}

String ^clinear::getdesc(void) {
	vector l=c->c->getreal(p1->data)-c->c->getreal(p2->data);
	if (name->Length > 0) 
		return gcnew String(name + " = " + c->f->format(l.len));
	else
		return gcnew String("L = "+c->f->format(l.len));
}

String ^clinear::exportCSV(void) {
	vector A,B;
	A=c->c->getreal(p1->data);
	B=c->c->getreal(p2->data);
	vector l=B-A;
	return String::Format("Line\t{0}\t{1}\t{2}\t{3}\t{4}\t{5}\n",name,A.x,A.y,B.x,B.y,l.len);
}

void clinear::calc_ref_point() {
	vector vx=p2->data-p1->data;
	double lx=vx.len;
	vector vy=vector(-vx.y,vx.x);
	
	if (lx == 0) 
		pp->data=(p1->data+p2->data)/2;
	else
		pp->data=(p1->data+p2->data)/2+vy*offset/lx;
}

void clinear::refchangenotify(cmeasurement ^csrc) {
	offset=-getdist(p1->data,p2->data,pp->data);
}


void clinear::draw(Graphics ^g) {
	Pen ^pen;
	SolidBrush ^bgb;

	if (drawrefpoints)	{
		// Draw points
		p1->draw(g);
		p2->draw(g);
	}

	vector A=c->t->toscreen(p1->data);
	vector B=c->t->toscreen(p2->data);
	vector vx=B-A;
	vector vy=vector(-vx.y,vx.x);
	vector vr=c->c->getreal(p1->data)-c->c->getreal(p2->data);
	double lr=vr.len;
	double lx=vx.len;

	calc_ref_point();

	if (selected) pp->draw(g);

	if (selected)
		pen=gcnew Pen( Color::FromArgb(255-linecol.R,255-linecol.G,255-linecol.B),2.0f );
	else
		pen=gcnew Pen( linecol,1.0f );

//	if (fillbackground) {
//		bgb=gcnew SolidBrush(FillBGCol);
//	} else {
//		bgb=nullptr;
//	}

	vector voffset;
	if (lx != 0) {
		vector vr=p2->data-p1->data;
		vector vw=vector(-vr.y,vr.x);
		voffset=c->t->toscreen(p1->data + vw/vr.len*offset)-c->t->toscreen(p1->data);
	} else
		voffset=vector(0,0);
	double step10x=find10step(lx/lr,c->t->screen.len/2);
	double tickxlen=5/lx;
	double t1,t2;

	drawline(g,A+voffset,B+voffset,pen);

	if (findsepoints(g,A+voffset,B+voffset,&t1,&t2)) {
		double dt1,dt2;dt1=t1;dt2=t2;
		if (dt1 < 0) dt1=0;
		if (dt2 > 1) dt2=1;
		double sp=floor(t1*lr/step10x);
		double ep=ceil(t2*lr/step10x);
		vector v=A+voffset+vx*(dt1+dt2)/2;
		vector rv=c->c->getreal(p1->data)-c->c->getreal(p2->data);
		if (offset > 0) {
			drawline(g,A-vy*2*tickxlen,A+voffset+vy*2*tickxlen,pen);
			drawline(g,B-vy*2*tickxlen,B+voffset+vy*2*tickxlen,pen);
		} else {
			drawline(g,A+voffset-vy*2*tickxlen,A+vy*2*tickxlen,pen);
			drawline(g,B+voffset-vy*2*tickxlen,B+vy*2*tickxlen,pen);
		}
			
		if (showticks) {
			for (int i=sp;i<=ep;i++) {
				double tickpos=(double)i*step10x/lr;
				if ((tickpos >= dt1) && (tickpos <= dt2))
					drawline(g,A+voffset+vx*tickpos-vy*tickxlen,A+voffset+vx*tickpos+vy*tickxlen,pen);
				for (int j=1;j<10;j++) {
					double tickpos=(double)(i+0.1*j)*step10x/lr;
					double ticklen=0.3;
					if (j==5) ticklen=0.6;
					if ((tickpos >= dt1) && (tickpos <= dt2))
						drawline(g,A+voffset+vx*tickpos-vy*tickxlen*ticklen,A+voffset+vx*tickpos+vy*tickxlen*ticklen,pen);
				}
			}
		}

		int val=0;
		switch(va) {
		case VAlign::Top: val=1;break;
		case VAlign::Center: val=0;break;
		case VAlign::Bottom: val=-1;break;
		}
		if (name->Length > 0)
			drawtxt(g,v,vx,0,val,name + " = " + c->f->format(rv.len));
		else
			drawtxt(g,v,vx,0,val,c->f->format(rv.len));
	}
}

List<cmeasurement^> ^clinear::touching(vector datai) {
	List<cmeasurement^> ^l=gcnew List<cmeasurement^>();
	List<cmeasurement^> ^lin;
	if (lin=p1->touching(datai)) { l->AddRange(lin); };
	if (lin=p2->touching(datai)) { l->AddRange(lin); };
	calc_ref_point();
	if (lin=pp->touching(datai)) { l->AddRange(lin); };
	if (l->Count > 0) return l;


	if (l->Count < 1) {
		// Check for Line Hit
		vector screen=c->t->toscreen(datai);
		vector A=c->t->toscreen(p1->data);
		vector B=c->t->toscreen(p2->data);
		vector vx=B-A;
		vector vy=vector(-vx.y,vx.x);
		double lx=vx.len;
		vector voffset;
		if (lx != 0) {
			vector vr=p2->data-p1->data;
			vector vw=vector(-vr.y,vr.x);
			voffset=c->t->toscreen(p1->data + vw/vr.len*offset)-c->t->toscreen(p1->data);
		} else
			voffset=vector(0,0);
		double ldx=fabs(getinlinedist(A+voffset,B+voffset,screen));
		if (ldx < 3) {
			l->Add(this);
			return l;
		}
		if (offset > 0) {
			double tickxlen=0;
			ldx=fabs(getinlinedist(A-vy*2*tickxlen,A+voffset+vy*2*tickxlen,screen));
			if (ldx < 3) { l->Add(this); return l; }
			ldx=fabs(getinlinedist(B-vy*2*tickxlen,B+voffset+vy*2*tickxlen,screen));
			if (ldx < 3) { l->Add(this); return l; }
		} else {
			double tickxlen=0;
			ldx=fabs(getinlinedist(A+voffset-vy*2*tickxlen,A+vy*2*tickxlen,screen));
			if (ldx < 3) { l->Add(this); return l; }
			ldx=fabs(getinlinedist(B+voffset-vy*2*tickxlen,B+vy*2*tickxlen,screen));
			if (ldx < 3) { l->Add(this); return l; }
		}		

		return nullptr;
	}
	return l;
}

List<cmeasurement^> ^clinear::isinside(vector data1, vector data2) {
	List<cmeasurement^> ^l=gcnew List<cmeasurement^>();
	List<cmeasurement^> ^lin;
	// if (drawrefpoints) {
		if (lin=p1->isinside(data1,data2)) { l->AddRange(lin); };
		if (lin=p2->isinside(data1,data2)) { l->AddRange(lin); };
	// }
	if (l->Count >= 2) {
		delete l;
		l=gcnew List<cmeasurement^>();
		l->Add(this);
		return l;
	}
	calc_ref_point();
	if (lin=pp->isinside(data1,data2)) { l->AddRange(lin); };
	if (l->Count < 1) 
		return nullptr;
	return l;
}

void clinear::moveto(vector data_from, vector data_to) {
	if (selected) {
		p1->moveto(data_from,data_to);
		p2->moveto(data_from,data_to);
	} else {
		if (p1->selected) p1->moveto(data_from,data_to);
		if (p2->selected) p2->moveto(data_from,data_to);
	}
}

void clinear::storestate() {
	p1->storestate();
	p2->storestate();
	pp->storestate();
}

void clinear::restorestate() {
	p1->restorestate();
	p2->restorestate();
	pp->restorestate();
}

void clinear::select() {
	selected=1;
	p1->unselect();
	p2->unselect();
	pp->unselect();
}

void clinear::unselect() {
	selected=0;
	p1->unselect();
	p2->unselect();
	pp->unselect();
}

//******************************************************* C C O L O R B A R *********************************************************

void ccolorbar::ccolorbar_default() {
	p1=nullptr;
	p2=nullptr;
	o1=0;v1=0;
	o2=1;v2=1;
	cbmode = ColorBarMode::Standard;
	prf=nullptr;
	filtsize=8;
	cp1=nullptr;
	cp2=nullptr;
	show=1;
	readonly=1;
}

ccolorbar::ccolorbar(vector data1, vector data2, csys ^c):cmeasurement(gcnew String("ColorBar"),c) {
	ccolorbar_default();
	p1=gcnew cpoint(data1, c);
	p2=gcnew cpoint(data2, c);
	p1->dmode=PointDisplayMode::PointOnly;p1->pts=PointStyles::Plus;
	p2->dmode=PointDisplayMode::PointOnly;p2->pts=PointStyles::Plus;
	cp1=gcnew crefpoint(data1,c);
	cp2=gcnew crefpoint(data2,c);
	cp1->setcallback(this);
	cp2->setcallback(this);
	calcprofile();
}

ccolorbar::ccolorbar(csys ^c, ia_xml_read ^x):cmeasurement(gcnew String("ColorBar"),c) {
	int foundp1,foundp2,rv;
	foundp1=foundp2=0;

	ccolorbar_default();

	while (1) {
		switch (x->NodeType) {
			case XmlNodeType::Element:
				if (rv = read_from_xml(x)) {
					if (rv & 0x02) foundp1=1;
					if (rv & 0x04) foundp2=1;
				} else {
					// Unknown element --> Skip over
					if (x->IsEmptyElement)	x->Read(); else x->Skip();
				}
				break;
			case XmlNodeType::EndElement:
				if (!foundp1 || !foundp2) 
					throw gcnew System::Exception("Bad XML Formatted File <COLORBAR> tag");
				x->Read();
				cp1=gcnew crefpoint(p1->data+o1*(p2->data-p1->data),c);
				cp2=gcnew crefpoint(p1->data+o2*(p2->data-p1->data),c);
				cp1->setcallback(this);
				cp2->setcallback(this);
				calcprofile();
				return;
			default:
				throw gcnew System::Exception("Bad XML Formatted File");
		}
	}
}
	
void ccolorbar::write_to_xml(ia_xml_write ^x) {
	cmeasurement::write_to_xml(x);
	x->WriteStartElement("P1");
		p1->write_to_xml(x);
	x->WriteEndElement();x->WriteWhitespace("\n");
	x->WriteStartElement("P2");
		p2->write_to_xml(x);
	x->WriteEndElement();x->WriteWhitespace("\n");
	x->write_value("O1",o1);
	x->write_value("O2",o2);
	x->write_value("V1",v1);
	x->write_value("V2",v2);
	x->write_value("SHOW",show);
	x->write_value("CBMODE",(int)cbmode);
}

int ccolorbar::read_from_xml(ia_xml_read ^x) {
	if (cmeasurement::read_from_xml(x)) return 1;

	if (String::Compare(x->Name,"P1")==0) {
		x->Read();
		p1=gcnew cpoint(c,x);
		return 0x02;
	} else if (String::Compare(x->Name,"P2")==0) {
		x->Read();
		p2=gcnew cpoint(c,x);
		return 0x04;
	} else if (String::Compare(x->Name,"O1")==0) {
		o1=x->ReadElementContentAsDouble();
	} else if (String::Compare(x->Name,"O2")==0) {
		o2=x->ReadElementContentAsDouble();
	} else if (String::Compare(x->Name,"V1")==0) {
		v1=x->ReadElementContentAsDouble();
	} else if (String::Compare(x->Name,"V2")==0) {
		v2=x->ReadElementContentAsDouble();
	} else if (String::Compare(x->Name,"SHOW")==0) {
		v2=x->ReadElementContentAsInt();
	} else if (String::Compare(x->Name,"CBMODE")==0) {
		cbmode=(ColorBarMode)x->ReadElementContentAsInt();
	} else return 0;
	return 1;
}
	
void ccolorbar::update_from_template(cmeasurement ^templ) {
	if (templ == nullptr) return;
	cmeasurement::update_from_template(templ);

	cbmode=((ccolorbar^)templ)->cbmode;
}

String ^ccolorbar::getdesc(void) {
	vector l=c->c->getreal(p1->data)-c->c->getreal(p2->data);
	if (name->Length > 0) 
		return gcnew String(name + " = " + c->f->format(l.len));
	else
		return gcnew String("L = "+c->f->format(l.len));
}

void ccolorbar::filterprofile(array<double> ^e) {
	array<double> ^f;
	f=makefilter(ProfileFilter::LowPass,filtsize);
	if (f == nullptr) return;
	filter(e,f,1);
	delete f;
}

void ccolorbar::calcprofile() {
	if (c->img == nullptr) return;
	if (prf == nullptr) {
		prf = gcnew profile(c->img,c->image_from_data(p1->data),c->image_from_data(p2->data));
		return;
	} 
	prf->shift(c->image_from_data(p1->data),c->image_from_data(p2->data));
}

String ^ccolorbar::exportCSV(void) {
	vector A,B;
	A=c->c->getreal(p1->data);
	B=c->c->getreal(p2->data);
	vector l=B-A;
	return String::Format("Colorbar\t{0}\t{1}\t{2}\t{3}\t{4}\t{5}\n",name,A.x,A.y,B.x,B.y,l.len);
}

void ccolorbar::refchangenotify(cmeasurement ^csrc) {
	array<double> ^e;
	double min,max;
	vector vx=p2->data-p1->data;
	
	double lx=vx.len;
	// Must be cp1 or cp2...
	if ((crefpoint ^)csrc == cp1) {
		double o=getxdist(p1->data,p2->data,cp1->data)/lx;
		if (o < 0.0) o=0.0;
		if (o > 1.0) o=1.0;
		o1=o;
	}
	if ((crefpoint ^)csrc == cp2) {
		double o=getxdist(p1->data,p2->data,cp2->data)/lx;
		if (o < 0.0) o=0.0;
		if (o > 1.0) o=1.0;
		o2=o;
	}
}

void ccolorbar::draw(Graphics ^g) {
	Pen ^pen;
	SolidBrush ^bgb;

	if (!show) return;

	// Draw points
	p1->draw(g);
	p2->draw(g);

	calcprofile();

	vector A=c->t->toscreen(p1->data);
	vector B=c->t->toscreen(p2->data);
	vector vx=B-A;
	vector vy=vector(-vx.y,vx.x);
	//vector vr=c->c->getreal(p1->data)-c->c->getreal(p2->data);
	//double lr=vr.len;
	//double lx=vx.len;
	//double ofs=offset/vx.len;

	if (vy.len == 0) return;

	if (selected)
		pen=gcnew Pen( Color::FromArgb(255-linecol.R,255-linecol.G,255-linecol.B),2.0f );
	else
		pen=gcnew Pen( linecol,1.0f );

	vector voffset;
	voffset=vy*(3.0/vy.len);
	//if (lx != 0) {
	//	//vector vr=p2->data-p1->data;
	//	vector vw=vector(-vr.y,vr.x);
	//	voffset=c->t->toscreen(p1->data + vw/vr.len*offset)-c->t->toscreen(p1->data);
	//} else
	//	voffset=vector(0,0);

	drawline(g,A-voffset,B-voffset,pen);
	drawline(g,A-voffset,A+voffset,pen);
	drawline(g,A+voffset,B+voffset,pen);
	drawline(g,B-voffset,B+voffset,pen);

	vector C;
	C=A+o1*(B-A);drawline(g,C-2*voffset,C+2*voffset,pen);
	drawtxt(g,C+2*voffset,vy,-1,0,c->f->cformat(v1));
	C=A+o2*(B-A);drawline(g,C-2*voffset,C+2*voffset,pen);
	drawtxt(g,C+2*voffset,vy,-1,0,c->f->cformat(v2));
}

List<cmeasurement^> ^ccolorbar::touching(vector datai) {
	List<cmeasurement^> ^l=gcnew List<cmeasurement^>();
	List<cmeasurement^> ^lin;

	if (!show) return nullptr;

	// Check for internal points first
	cp1->data=p1->data+o1*(p2->data-p1->data);
	cp2->data=p1->data+o2*(p2->data-p1->data);
	if (lin=cp1->touching(datai)) { l->AddRange(lin); return l; }
	if (lin=cp2->touching(datai)) { l->AddRange(lin); return l; }
	

	// Now go for the endpoints
	if (lin=p1->touching(datai)) { l->AddRange(lin); };
	if (lin=p2->touching(datai)) { l->AddRange(lin); };
	if (l->Count > 0) return l;

/*	calc_ref_point();
	if (lin=pp->touching(datai)) { l->AddRange(lin); };
	if (l->Count > 0) return l;

	if (prcur != ProfileCursors::None) {
		if (lin=cp1->touching(datai)) { l->AddRange(lin); return l; };
		if (lin=cp2->touching(datai)) { l->AddRange(lin); return l; };
	} */
	if (l->Count > 0) return l;

	if (l->Count < 1) {
		// Check for Line Hit
		vector screen=c->t->toscreen(datai);
		vector A=c->t->toscreen(p1->data);
		vector B=c->t->toscreen(p2->data);
		double ldx=fabs(getinlinedist(A,B,screen));
		if (ldx < 3) {
			l->Add(this);
			return l;
		}
		return nullptr;
	}
	return l;
}

List<cmeasurement^> ^ccolorbar::isinside(vector data1, vector data2) {
	List<cmeasurement^> ^l=gcnew List<cmeasurement^>();
	List<cmeasurement^> ^lin;

	if (!show) return nullptr;

	if (lin=p1->isinside(data1,data2)) { l->AddRange(lin); };
	if (lin=p2->isinside(data1,data2)) { l->AddRange(lin); };
	if (l->Count >= 2) {
		delete l;
		l=gcnew List<cmeasurement^>();
		l->Add(this);
		return l;
	}

	cp1->data=p1->data+o1*(p2->data-p1->data);
	cp2->data=p1->data+o2*(p2->data-p1->data);
	if (lin=cp1->isinside(data1,data2)) { l->AddRange(lin); }
	if (lin=cp2->isinside(data1,data2)) { l->AddRange(lin); }

	if (l->Count < 1) 
		return nullptr;
	return l;
}

void ccolorbar::moveto(vector data_from, vector data_to) {
	if (selected) {
		p1->moveto(data_from,data_to);
		p2->moveto(data_from,data_to);
	} else {
		if (p1->selected) p1->moveto(data_from,data_to);
		if (p2->selected) p2->moveto(data_from,data_to);
	}
}

void ccolorbar::storestate() {
	p1->storestate();
	p2->storestate();
	cp1->storestate();
	cp2->storestate();
}

void ccolorbar::restorestate() {
	p1->restorestate();
	p2->restorestate();
	cp1->restorestate();
	cp2->restorestate();
}

void ccolorbar::select() {
	selected=1;
	p1->unselect();
	p2->unselect();
	cp1->unselect();
	cp2->unselect();
}

void ccolorbar::unselect() {
	selected=0;
	p1->unselect();
	p2->unselect();
	cp1->unselect();
	cp2->unselect();
}


//******************************************************* C P R O F I L E ***********************************************************

void cprofile::cprofile_default() {
	p1=nullptr;
	p2=nullptr;
	pp=nullptr;
	cp1=nullptr;
	cp2=nullptr;
	prf=nullptr;
	offset=1;
	va=VAlign::Bottom;
	showprofile=1;
	showlength=1;
	showrgb=0;
	fillprofile=0;
	scaler=scaleg=scaleb=1;
	prcolor=Color::FromArgb(255,128,0);
	prfillcol=Color::FromArgb(128,64,0);
	pralpha=0.5;
	prsrc=ProfileSource::WeightedRGBSum;
	prflt=ProfileFilter::Off;
	prpost=ProfilePost::Normalize;
	filtsize=8;
	c1x=c1y=c2y=0.0;
	c2x=1.0;
}

cprofile::cprofile(vector data1, vector data2, csys ^c):cmeasurement(gcnew String("Profile"),c) {
	cprofile_default();
	p1=gcnew cpoint(data1, c);
	p2=gcnew cpoint(data2, c);
	p1->dmode=PointDisplayMode::PointOnly;p1->pts=PointStyles::Dot;
	p2->dmode=PointDisplayMode::PointOnly;p2->pts=PointStyles::Dot;

	offset=fabs(len(data2-data1))*0.25;
	pp=gcnew crefpoint((data1+data2)/2,c);
	pp->setcallback(this);
	cp1=gcnew crefpoint(data1,c);
	cp1->setcallback(this);
	cp2=gcnew crefpoint(data2,c);
	cp2->setcallback(this);
	cp1->pts=PointStyles::Plus;
	cp2->pts=PointStyles::Plus;
	calc_ref_point();
	calcprofile();
}

cprofile::cprofile(csys ^c, ia_xml_read ^x):cmeasurement(gcnew String("Profile"),c) {
	int foundp1,foundp2,rv;
	foundp1=foundp2=0;

	cprofile_default();

	while (1) {
		switch (x->NodeType) {
			case XmlNodeType::Element:
				if (rv = read_from_xml(x)) {
					if (rv & 0x02) foundp1=1;
					if (rv & 0x04) foundp2=1;
				} else {
					// Unknown element --> Skip over
					if (x->IsEmptyElement)	x->Read(); else x->Skip();
				}
				break;
			case XmlNodeType::EndElement:
				if (!foundp1 || !foundp2) 
					throw gcnew System::Exception("Bad XML Formatted File <PROFILE> tag");
				x->Read();
				pp=gcnew crefpoint((p1->data+p2->data)/2,c);
				pp->setcallback(this);
				cp1=gcnew crefpoint(p1->data,c);
				cp1->setcallback(this);
				cp2=gcnew crefpoint(p2->data,c);
				cp2->setcallback(this);
				cp1->pts=PointStyles::Plus;
				cp2->pts=PointStyles::Plus;
				calc_ref_point();
				calcprofile();
				return;
			default:
				throw gcnew System::Exception("Bad XML Formatted File");
		}
	}
}
	
void cprofile::write_to_xml(ia_xml_write ^x) {
	cmeasurement::write_to_xml(x);
	x->WriteStartElement("P1");
		p1->write_to_xml(x);
	x->WriteEndElement();x->WriteWhitespace("\n");
	x->WriteStartElement("P2");
		p2->write_to_xml(x);
	x->WriteEndElement();x->WriteWhitespace("\n");
	x->write_value("OFFSET",offset);
	x->write_value("SHOWPROFILE",showprofile);
	x->write_value("SHOWLENGTH",showlength);
	x->write_value("SHOWRGB",showrgb);
	x->write_value("VALIGN",(int)va);
	x->write_value("SCALER",scaler);
	x->write_value("SCALEG",scaleg);
	x->write_value("SCALEB",scaleb);
	x->write_value("FILLPROFILE",fillprofile);
	x->write_value("PRCOLOR",prcolor);
	x->write_value("PRFILLCOLOR",prfillcol);
	x->write_value("PRALPHA",pralpha);
	x->write_value("PRSRC",(int)prsrc);
	x->write_value("PRFLT",(int)prflt);
	x->write_value("PRPOST",(int)prpost);
	x->write_value("FILTSIZE",(int)filtsize);
	x->write_value("CURSORS",(int)prcur);
	x->write_value("C1X",c1x);
	x->write_value("C1Y",c1y);
	x->write_value("C2X",c2x);
	x->write_value("C2Y",c2y);

	//calc_ref_point();
}

int cprofile::read_from_xml(ia_xml_read ^x) {
	if (cmeasurement::read_from_xml(x)) return 1;

	if (String::Compare(x->Name,"P1")==0) {
		x->Read();
		p1=gcnew cpoint(c,x);
		return 0x02;
	} else if (String::Compare(x->Name,"P2")==0) {
		x->Read();
		p2=gcnew cpoint(c,x);
		return 0x04;
	} else if (String::Compare(x->Name,"OFFSET")==0) {
		offset=x->ReadElementContentAsDouble();
	} else if (String::Compare(x->Name,"SHOWPROFILE")==0) {
		showprofile=x->ReadElementContentAsInt();
	} else if (String::Compare(x->Name,"SHOWLENGTH")==0) {
		showlength=x->ReadElementContentAsInt();
	} else if (String::Compare(x->Name,"SHOWRGB")==0) {
		showrgb=x->ReadElementContentAsInt();
	} else if (String::Compare(x->Name,"VALIGN")==0) {
		va=(image_add::VAlign)x->ReadElementContentAsInt();
	} else if (String::Compare(x->Name,"SCALER")==0) {
		scaler=x->ReadElementContentAsDouble();
	} else if (String::Compare(x->Name,"SCALEG")==0) {
		scaleg=x->ReadElementContentAsDouble();
	} else if (String::Compare(x->Name,"SCALEB")==0) {
		scaleb=x->ReadElementContentAsDouble();
	} else if (String::Compare(x->Name,"FILLPROFILE")==0) {
		fillprofile=x->ReadElementContentAsInt();
	} else if (String::Compare(x->Name,"PRCOLOR")==0) {
		x->Read();prcolor=x->xml_read_color();
	} else if (String::Compare(x->Name,"PRFILLCOLOR")==0) {
		x->Read();prfillcol=x->xml_read_color();
	} else if (String::Compare(x->Name,"PRALPHA")==0) {
		pralpha=x->ReadElementContentAsDouble();
	} else if (String::Compare(x->Name,"PRSRC")==0) {
		prsrc=(image_add::ProfileSource)x->ReadElementContentAsInt();
	} else if (String::Compare(x->Name,"PRFLT")==0) {
		prflt=(image_add::ProfileFilter)x->ReadElementContentAsInt();
	} else if (String::Compare(x->Name,"PRPOST")==0) {
		prpost=(image_add::ProfilePost)x->ReadElementContentAsInt();
	} else if (String::Compare(x->Name,"FILTSIZE")==0) {
		filtsize=x->ReadElementContentAsDouble();
	} else if (String::Compare(x->Name,"CURSORS")==0) {
		prcur=(image_add::ProfileCursors)x->ReadElementContentAsInt();
	} else if (String::Compare(x->Name,"C1X")==0) {
		c1x=x->ReadElementContentAsDouble();
	} else if (String::Compare(x->Name,"C1Y")==0) {
		c1y=x->ReadElementContentAsDouble();
	} else if (String::Compare(x->Name,"C2X")==0) {
		c2x=x->ReadElementContentAsDouble();
	} else if (String::Compare(x->Name,"C2Y")==0) {
		c2y=x->ReadElementContentAsDouble();
	} else return 0;
	return 1;
}
	
void cprofile::update_from_template(cmeasurement ^templ) {
	if (templ == nullptr) return;
	cmeasurement::update_from_template(templ);

	showprofile=((cprofile^)templ)->showprofile;
	showlength=((cprofile^)templ)->showlength;
	showrgb=((cprofile^)templ)->showrgb;
	va=((cprofile^)templ)->va;
	scaler=((cprofile^)templ)->scaler;
	scaleg=((cprofile^)templ)->scaleg;
	scaleb=((cprofile^)templ)->scaleb;	
	fillprofile=((cprofile^)templ)->fillprofile;
	prcolor=((cprofile^)templ)->prcolor;	
	prfillcol=((cprofile^)templ)->prfillcol;	
	pralpha=((cprofile^)templ)->pralpha;	
	prsrc=((cprofile^)templ)->prsrc;
	prflt=((cprofile^)templ)->prflt;
	prpost=((cprofile^)templ)->prpost;
	filtsize=((cprofile^)templ)->filtsize;
	prcur=((cprofile^)templ)->prcur;
	c1x=((cprofile^)templ)->c1x;
	c1y=((cprofile^)templ)->c1y;
	c2x=((cprofile^)templ)->c2x;
	c2y=((cprofile^)templ)->c2y;
}

String ^cprofile::getdesc(void) {
	vector l=c->c->getreal(p1->data)-c->c->getreal(p2->data);
	if (name->Length > 0) 
		return gcnew String(name + " = " + c->f->format(l.len));
	else
		return gcnew String("L = "+c->f->format(l.len));
}

array<double> ^cprofile::getprofile() {
	array<double> ^e;
	double offs;

	if ((prf == nullptr) || (prf->pts <= 0)) return nullptr;

	e=gcnew array<double>(prf->pts);
	switch (prsrc) {
	case ProfileSource::WeightedRGBSum:
		for (int i = 0; i < prf->pts; i++) {
			e[i] = (double)prf->R[i] * scaler + prf->R[i] * scaleg + prf->B[i] * scaleb;
		}
		break;
	case ProfileSource::Red:
		for (int i = 0; i < prf->pts; i++) {
			e[i] = (double)prf->R[i];
		}
		break;
	case ProfileSource::Green:
		for (int i = 0; i < prf->pts; i++) {
			e[i] = (double)prf->G[i];
		}
		break;
	case ProfileSource::Blue:
		for (int i = 0; i < prf->pts; i++) {
			e[i] = (double)prf->B[i];
		}
		break;
	case ProfileSource::Cyan:
		for (int i = 0; i < prf->pts; i++) {
			e[i] = (double) 1.0 - prf->R[i];
		}
		break;
	case ProfileSource::Magenta:
		for (int i = 0; i < prf->pts; i++) {
			e[i] = (double) 1.0 - prf->G[i];
		}
		break;
	case ProfileSource::Yellow:
		for (int i = 0; i < prf->pts; i++) {
			e[i] = (double) 1.0 - prf->B[i];
		}
		break;
	case ProfileSource::Hue:
		for (int i = 0; i < prf->pts; i++) {
			double H, S, L;
			getHSLfromRGB(prf->R[i], prf->G[i], prf->B[i], &H, &S, &L);
			e[i] = (double)H;
		}
		// unfold
		offs = 0;
		for (int i = 1; i < prf->pts; i++) {
			double d1 = fabs(e[i] - (e[i - 1] - offs));
			double d2 = fabs(e[i] + 1.0 - (e[i - 1] - offs));
			double d3 = fabs(e[i] - 1.0 - (e[i - 1] - offs));
			if ((d2 < d1) && (d2 < d3)) {
				offs += 1.0;
			}
			else if ((d3 < d1) && (d3 < d2)) {
				offs -= 1.0;
			}
			e[i] += offs;
		}
		break;
	case ProfileSource::Saturation:
		for (int i = 0; i < prf->pts; i++) {
			double H, S, L;
			getHSLfromRGB(prf->R[i], prf->G[i], prf->B[i], &H, &S, &L);
			e[i] = (double)S;
		}
		break;
	case ProfileSource::Lightness:
		for (int i = 0; i < prf->pts; i++) {
			double H, S, L;
			getHSLfromRGB(prf->R[i], prf->G[i], prf->B[i], &H, &S, &L);
			e[i] = (double)L;
		}
		break;
	case ProfileSource::ColorVal:
		for (int i = 0; i < prf->pts; i++) {
			int valvalid = 1;
			double colval = 0;
			if (c->cbar == nullptr) valvalid = 0;
			if (valvalid && (c->cbar->prf == nullptr)) valvalid = 0;
			if (valvalid) {
				double colrel = c->cbar->prf->findcolor(prf->R[i], prf->G[i], prf->B[i]);
				if (colrel >= 0) {
					colval = c->cbar->v1 + (colrel - c->cbar->o1) / (c->cbar->o2 - c->cbar->o1)*(c->cbar->v2 - c->cbar->v1);
				}
				else valvalid = 0;
			}
			if (valvalid)
				e[i] = colval;
			else
				e[i] = 0;
		}
		break;
	}
	return e;
}

void cprofile::filterprofile(array<double> ^e) {
	double laste,s;
	array<double> ^f;

	switch (prflt) {
	case ProfileFilter::Off:
		return;
	case ProfileFilter::Derivative:
		laste=e[0];
		e[0]=e[1]-e[0];
		for (int i=1;i<e->Length;i++) {
			double tempe=e[i];
			e[i]=e[i]-laste;
			laste=tempe;
		}
		return;
	case ProfileFilter::HighPass:
		f=makefilter(prflt,filtsize);
		if (f == nullptr) return;
		filter(e,f,0);
		delete f;
		return;
	case ProfileFilter::LowPass:
		f=makefilter(prflt,filtsize);
		if (f == nullptr) return;
		filter(e,f,1);
		delete f;
		return;
	case ProfileFilter::Median:
		filter_median(e,ceil(filtsize));
		return;
	case ProfileFilter::Integrated:
		s=0;
		for (int i=0;i<e->Length;i++) {
			s+=e[i];
			e[i]=s-e[i];
		}
		return;
	}
}

void cprofile::postprofile(array<double> ^e) {
	double min,max;
	int minf,maxf;
	if (e == nullptr) return;
	switch (prpost) {
	case ProfilePost::Linear:
		return;
	case ProfilePost::Log:
		// find min
		minf=0;
		min=-1;
		for (int i=0;i<e->Length;i++) 
			if (e[i] != 0) {
				if (minf == 0) min=fabs(e[i]);
				if (fabs(e[i])<min) min=fabs(e[i]);
			}
		for (int i=0;i<e->Length;i++) {
			if (e[i]==0) 
				e[i]=log10(fabs(min));
			else {
				if (e[i] < 0) 
					e[i]=log10(-e[i]);
				else
					e[i]=log10(e[i]);
			}
		}
		return;
	case ProfilePost::Normalize:
		min=e[0];
		max=e[0];
		for (int i=1;i<e->Length;i++) {
			if (e[i] < min) min=e[i];
			if (e[i] > max) max=e[i];
		}
		if (max > min)
			for (int i=0;i<e->Length;i++)
				e[i]=(e[i]-min)/(max-min);
		return;
	}
}

String ^cprofile::exportCSV(void) {
	vector A,B;
	vector v;
	A=c->real_from_data(p1->data);
	B=c->real_from_data(p2->data);
	v=B-A;
	String ^s=gcnew String("");
	s+=String::Format("Profile\t{0}\n",name);
	if (showrgb) {
		s+=String::Format("\t\tL\tX\tY\tR\tG\tB\n");
		if ((prf != nullptr) && (prf->pts > 0)) {
			for (int i=0;i<prf->pts;i++) {
				vector q=A+prf->X[i]*v;
				s+=String::Format("\t\t{0}\t{1}\t{2}\t",len(q-A),q.x,q.y);
				s+=String::Format("{0}\t{1}\t{2}\n",prf->R[i],prf->G[i],prf->B[i]);
			}
		}	
	} else { 
		s+=String::Format("\t\tL\tX\tY\tC\n");
		array<double>^e=getprofile();
		if (e != nullptr) {
			filterprofile(e);
			for (int i=0;i<prf->pts;i++) {
				vector q=A+prf->X[i]*v;
				s+=String::Format("\t\t{0}\t{1}\t{2}\t",len(q-A),q.x,q.y);
				s+=String::Format("{0}\n",e[i]);
			}
			delete e;
		}
	}
	return s;
}

double getdata(array<double> ^X, array<double> ^Y, double ux) {
	if ((X == nullptr) || (Y == nullptr)) return 0.0;
	if (ux < X[0]) return 0.0;
	if (ux > X[X->Length-1]) return 0.0;
	int ir=1;
	while ((ir < X->Length-1) && (ux > X[ir])) ir++;
	return Y[ir-1]+(Y[ir]-Y[ir-1])*(ux-X[ir-1])/(X[ir]-X[ir-1]);
}

int findcross(array<double> ^X, array<double> ^Y, double *ux, double *uy,int dir) {
	if ((X == nullptr) || (Y == nullptr)) return 0;
	if (dir > 0) {
		// search left to right
		if (Y[0] > *uy) {
			int ir = 1;
			while ((ir < Y->Length) && (Y[ir] > *uy)) ir++;
			if (ir >= Y->Length) return 0;
			*ux=X[ir-1]+(*uy-Y[ir-1])/(Y[ir]-Y[ir-1])*(X[ir]-X[ir-1]);
			return 1;
		} else if (Y[0] < *uy) {
			int ir = 1;
			while ((ir < Y->Length) && (Y[ir] < *uy)) ir++;
			if (ir >= Y->Length) return 0;
			*ux=X[ir-1]+(*uy-Y[ir-1])/(Y[ir]-Y[ir-1])*(X[ir]-X[ir-1]);
			return 1;
		} else {
			*ux=X[0];
			return 1;
		}
	} else { 
		// search right to left
		if (Y[Y->Length-1] > *uy) {
			int ir = Y->Length-2;
			while ((ir >= 0) && (Y[ir] > *uy)) ir--;
			if (ir < 0) return 0;
			*ux=X[ir]+(*uy-Y[ir])/(Y[ir+1]-Y[ir])*(X[ir+1]-X[ir]);
			return 1;
		} else if (Y[Y->Length-1] < *uy) {
			int ir = Y->Length-2;
			while ((ir >= 0) && (Y[ir] < *uy)) ir--;
			if (ir < 0) return 0;
			*ux=X[ir]+(*uy-Y[ir])/(Y[ir+1]-Y[ir])*(X[ir+1]-X[ir]);
			return 1;
		} else {
			*ux=X[0];
			return 1;
		}
	}
	return 0;
}

int findminmax(array<double> ^X, array<double> ^Y, double *ux, double *uy, int maxmin, int dir) {
	if ((X == nullptr) || (Y == nullptr)) return 0;
	if (dir > 0) {
		// left to right
		int ir=1;
		if (maxmin > 0) {
			// Max search
			while ((ir < Y->Length-1) && ((Y[ir] < Y[ir+1]) || (Y[ir] < Y[ir-1]))) ir++;
		} else {
			// Min search
			while ((ir < Y->Length-1) && ((Y[ir] > Y[ir+1]) || (Y[ir] > Y[ir-1]))) ir++;
		}
		if (ir >= Y->Length-1) return 0;
		*ux=X[ir];
		*uy=Y[ir];
		return 1;
	} else {
		// right to left
		int ir=Y->Length-2;
		if (maxmin > 0) {
			// Max search
			while ((ir > 0) && ((Y[ir] < Y[ir+1]) || (Y[ir] < Y[ir-1]))) ir--;
		} else {
			// Min search
			while ((ir > 0) && ((Y[ir] > Y[ir+1]) || (Y[ir] > Y[ir-1]))) ir--;
		}
		if (ir <= 0) return 0;
		*ux=X[ir];
		*uy=Y[ir];
		return 1;
	}
	return 0;
}

void cprofile::calc_ref_point() {
	array<double> ^e;
	vector vx=p2->data-p1->data;
	double lx=vx.len;
	vector vy=vector(-vx.y,vx.x);
	double lr=len(c->real_from_data(p1->data)-c->real_from_data(p2->data));
	double min,max;
	double c1xr,c2xr,c1yr,c2yr;
	double p1x,p1y,p2x,p2y;
	double yscale=offset/lx;

	if (lx == 0) 
		pp->data=(p1->data+p2->data)/2;
	else
		pp->data=(p1->data+p2->data)/2+vy*offset/lx;

	c1xr=c1x;
	c1yr=c1y;
	c2xr=c2x;
	c2yr=c2y;

	switch (prcur) {
	case ProfileCursors::None:
		cp1->data=p1->data;
		cp2->data=p2->data;
		//c1x=0.0;c1y=0.0;
		//c2x=lx;c2y=0.0;
		break;
	case ProfileCursors::ManualX:
		if (c1xr < 0) c1xr=0;
		if (c1xr > lx) c1xr=lx;
		if (c2xr < 0) c2xr=0;
		if (c2xr > lx) c2xr=lx;
		e=getprofile();
		filterprofile(e);
		postprofile(e);
		c1yr=getdata(prf->X,e,c1xr/lx);
		c2yr=getdata(prf->X,e,c2xr/lx);
		delete e;
		cp1->data=p1->data+vx*c1xr/lx+vy*c1yr*yscale;
		cp2->data=p1->data+vx*c2xr/lx+vy*c2yr*yscale;
		break;
	case ProfileCursors::ManualY:
		e=getprofile();
		filterprofile(e);
		postprofile(e);
		p1x=c1xr/lx;p1y=c1yr/lx;p2x=c2xr/lx;p2y=c2yr/lx;
		if (!findcross(prf->X,e,&p1x,&p1y,1)) {
			p1x=0;p1y=0;
		}
		if (!findcross(prf->X,e,&p2x,&p2y,-1)) {
			p2x=1;p2y=0;
		}
		c1xr=p1x*lx;c1yr=p1y;c2xr=p2x*lx;c2yr=p2y;
		delete e;
		cp1->data=p1->data+vx*c1xr/lx+vy*c1yr*yscale;
		cp2->data=p1->data+vx*c2xr/lx+vy*c2yr*yscale;
		break;
	case ProfileCursors::ManualYCoupled:
		e=getprofile();
		filterprofile(e);
		postprofile(e);
		c2yr=c1yr;
		p1x=c1xr;p1y=c1yr;p2x=c2xr;p2y=c2yr;
		if (!findcross(prf->X,e,&p1x,&p1y,1)) {
			p1x=0;p1y=0;
		}
		if (!findcross(prf->X,e,&p2x,&p2y,-1)) {
			p2x=lx;p2y=0;
		}
		c1xr=p1x;c1yr=p1y;c2xr=p2x;c2yr=p2y;
		delete e;
		cp1->data=p1->data+vx*c1xr+vy*c1yr*yscale;
		cp2->data=p1->data+vx*c2xr+vy*c2yr*yscale;
		break;
	case ProfileCursors::Threshold:
		e=getprofile();
		filterprofile(e);
		postprofile(e);
		min=e[0];max=e[0];
		for (int i=0;i<e->Length;i++) { if (e[i] < min) min=e[i]; if (e[i] > max) max=e[i]; }
		if (c1yr < 0) c1yr=0;
		if (c1yr > 1) c1yr=1.0;
		c2yr=c1yr;
		c1yr=min+c1yr*(max-min);
		p1x=c1xr;p2x=c2xr;
		if (!findcross(prf->X,e,&p1x,&c1yr,1)) {
			p1x=0;c1yr=0;
		} else c1yr=(c1yr-min)/(max-min);
		c2yr=min+c2yr*(max-min);
		if (!findcross(prf->X,e,&p2x,&c2yr,-1)) {
			p2x=lx;c2yr=0;
		} else c2yr=(c2yr-min)/(max-min);
		c1xr=p1x;c2xr=p2x;
		delete e;
		cp1->data=p1->data+vx*c1xr+vy*c1yr*yscale;
		cp2->data=p1->data+vx*c2xr+vy*c2yr*yscale;
		break;
	case ProfileCursors::LocalMax:
		e=getprofile();
		filterprofile(e);
		postprofile(e);
		if (!findminmax(prf->X,e,&p1x,&p1y,1,1)) {
			p1x=0;p1y=0;
		}
		if (!findminmax(prf->X,e,&p2x,&p2y,1,-1)) {
			p2x=lx;p2y=0;
		}
		c1xr=p1x;c1yr=p1y;c2xr=p2x;c2yr=p2y;
		delete e;
		cp1->data=p1->data+vx*c1xr+vy*c1yr*yscale;
		cp2->data=p1->data+vx*c2xr+vy*c2yr*yscale;
		break;
	case ProfileCursors::LocalMin:
		e=getprofile();
		filterprofile(e);
		postprofile(e);
		if (!findminmax(prf->X,e,&p1x,&p1y,-1,1)) {
			p1x=0;p1y=0;
		}
		if (!findminmax(prf->X,e,&p2x,&p2y,-1,-1)) {
			p2x=lx;p2y=0;
		}
		c1xr=p1x;c1yr=p1y;c2xr=p2x;c2yr=p2y;
		delete e;
		cp1->data=p1->data+vx*c1xr+vy*c1yr*yscale;
		cp2->data=p1->data+vx*c2xr+vy*c2yr*yscale;
		break;
	}
}

void cprofile::calcprofile() {
	if (c->img == nullptr) return;
	if (prf == nullptr) {
		prf = gcnew profile(c->img,c->image_from_data(p1->data),c->image_from_data(p2->data));
		return;
	} 
	prf->shift(c->image_from_data(p1->data),c->image_from_data(p2->data));
}

void cprofile::refchangenotify(cmeasurement ^csrc) {
	array<double> ^e;
	double min,max;
	vector vx=p2->data-p1->data;
	double yscale;
	if ((crefpoint ^)csrc == pp) {
		offset=-getdist(p1->data,p2->data,pp->data);
		return;
	}
	yscale=offset/vx.len;
	
	double lx=vx.len;
	// Must be cp1 or cp2...
	switch (prcur) {
	case ProfileCursors::None:
		return;
	case ProfileCursors::ManualX:
		if ((crefpoint ^)csrc == cp1) 
			c1x=getxdist(p1->data,p2->data,cp1->data);
		if ((crefpoint ^)csrc == cp2) 
			c2x=getxdist(p1->data,p2->data,cp2->data);
		return;
	case ProfileCursors::ManualY:
		if ((crefpoint ^)csrc == cp1) 
			c1y=-getdist(p1->data,p2->data,cp1->data)/yscale;
		if ((crefpoint ^)csrc == cp2) 
			c2y=-getdist(p1->data,p2->data,cp2->data)/yscale;
		return;
	case ProfileCursors::ManualYCoupled:
		if ((crefpoint ^)csrc == cp1) {
			c1y=-getdist(p1->data,p2->data,cp1->data)/yscale;
			c2y=c1y;
		}
		if ((crefpoint ^)csrc == cp2) {
			c2y=-getdist(p1->data,p2->data,cp2->data)/yscale;
			c1y=c2y;
		}
		return;
	case ProfileCursors::Threshold: 
		e=getprofile();
		filterprofile(e);
		postprofile(e);
		min=e[0];max=e[0];
		for (int i=0;i<e->Length;i++) { if (e[i] < min) min=e[i]; if (e[i] > max) max=e[i]; }
		if ((crefpoint ^)csrc == cp1) {
			c1y=(-getdist(p1->data,p2->data,cp1->data)/yscale-min)/(max-min);
			c2y=c1y;
		}
		if ((crefpoint ^)csrc == cp2) {
			c2y=(-getdist(p1->data,p2->data,cp2->data)/yscale-min)/(max-min);
			c1y=c2y;
		}
		delete e;
		return;
	case ProfileCursors::LocalMax: 
	case ProfileCursors::LocalMin: 
		return;
	}
}


void cprofile::draw(Graphics ^g) {
	Pen ^pen;
	SolidBrush ^bgb;

	// Draw points
	p1->draw(g);
	p2->draw(g);

	calcprofile();

	vector A=c->t->toscreen(p1->data);
	vector B=c->t->toscreen(p2->data);
	vector vx=B-A;
	vector vy=vector(-vx.y,vx.x);
	vector vr=c->c->getreal(p1->data)-c->c->getreal(p2->data);
	double lr=vr.len;
	double lx=vx.len;
	double ofs=offset/vx.len;
	
	calc_ref_point();

	if (selected) { 
		pp->draw(g);
	}
	if (prcur != ProfileCursors::None) {
		cp1->draw(g);
		cp2->draw(g);
	}

	if (selected)
		pen=gcnew Pen( Color::FromArgb(255-linecol.R,255-linecol.G,255-linecol.B),2.0f );
	else
		pen=gcnew Pen( linecol,1.0f );

	vector voffset;
	if (lx != 0) {
		vector vr=p2->data-p1->data;
		vector vw=vector(-vr.y,vr.x);
		voffset=c->t->toscreen(p1->data + vw/vr.len*offset)-c->t->toscreen(p1->data);
	} else
		voffset=vector(0,0);


	if (showprofile && (prf != nullptr) && (prf->pts >= 2)) {
		array<PointF> ^p=gcnew array<PointF>(prf->pts+2);
		array<double> ^d=gcnew array<double>(prf->pts);
		double max,min;
		double sf;
		vector vy=voffset;
		vector v;
		if (showrgb) {
			// Color
			array<double> ^in;
			for (int k=0;k<3;k++) {
				Color frcol,flcol;
				switch (k) {
				case 0: 
					frcol=Color::FromArgb(255,0,0);
					flcol=Color::FromArgb((int)floor(pralpha*100+0.5),255,0,0);
					in=prf->R;
					sf = scaler;
					break;
				case 1: 
					frcol=Color::FromArgb(0,255,0);
					flcol=Color::FromArgb((int)floor(pralpha*100+0.5),0,255,0);
					in=prf->G;
					sf = scaleg;
					break;
				case 2:
					frcol=Color::FromArgb(0,0,255);
					flcol=Color::FromArgb((int)floor(pralpha*100+0.5),0,0,255);
					in=prf->B;
					sf = scaleb;
					break;
				}
				if (sf > 0) {
					min=max=d[0]=in[0]*sf;
					for (int i=1;i<prf->pts;i++) {
						d[i]=in[i]*sf;
						if (d[i] < min) min=d[i];
						if (d[i] > max) max=d[i];
					}
					if (max == min) { double m=max; min = m-1; max=m+1; }
					//if (normalize) {
						for (int i=0;i<prf->pts;i++) {
							v=A+vx*prf->X[i]+vy*(d[i]-min)/(max-min);
							p[i].X=v.x;
							p[i].Y=v.y;			
						}
					//} else {
					//	for (int i=0;i<prf->pts;i++) {
					//		v=A+vx*prf->X[i]+vy*d[i];
					//		p[i].X=v.x;
					//		p[i].Y=v.y;			
					//	}
					//}
					v=A+vx*prf->X[prf->pts-1];
					p[prf->pts].X=v.x;
					p[prf->pts].Y=v.y;
					v=A+vx*prf->X[0];
					p[prf->pts+1].X=v.x;
					p[prf->pts+1].Y=v.y;
					if (fillprofile && pralpha > 0) {
						Brush ^fb=gcnew SolidBrush(flcol);
						g->FillPolygon(fb,p);
						delete fb;
					}
					Pen ^dp=gcnew Pen(frcol,1.0f);
					g->DrawPolygon(dp,p);
					delete dp;
				}
			}
		} else {
			// Grayscale processing
			array<double> ^e=getprofile();
			filterprofile(e);
			postprofile(e);
			for (int i=0;i<prf->pts;i++) {
				v=A+vx*prf->X[i]+vy*e[i];
				p[i].X=v.x;
				p[i].Y=v.y;			
			} 
			delete e;
			v=A+vx*prf->X[prf->pts-1];
			p[prf->pts].X=v.x;
			p[prf->pts].Y=v.y;
			v=A+vx*prf->X[0];
			p[prf->pts+1].X=v.x;
			p[prf->pts+1].Y=v.y;

			if (fillprofile && pralpha > 0) {
				Brush ^fb=gcnew SolidBrush(Color::FromArgb((int)floor(pralpha*100+0.5),
					prfillcol.R,prfillcol.G,prfillcol.B));
				g->FillPolygon(fb,p);
				delete fb;
			}
			Pen ^dp=gcnew Pen(prcolor,1.0f);
			g->DrawPolygon(dp,p);
			delete dp;
		}
		delete d;
		delete p;
	}

	drawline(g,A,B,pen);

	if (showlength) {
		int val=0;
		switch(va) {
		case VAlign::Top: val=1;break;
		case VAlign::Center: val=0;break;
		case VAlign::Bottom: val=-1;break;
		}
		if (name->Length > 0)
			drawtxt(g,(A+B)/2,vx,0,val,name + " = " + c->f->format(lr));
		else
			drawtxt(g,(A+B)/2,vx,0,val,c->f->format(lr));
	}
}

List<cmeasurement^> ^cprofile::touching(vector datai) {
	List<cmeasurement^> ^l=gcnew List<cmeasurement^>();
	List<cmeasurement^> ^lin;
	if (lin=p1->touching(datai)) { l->AddRange(lin); };
	if (lin=p2->touching(datai)) { l->AddRange(lin); };
	if (l->Count > 0) return l;

	calc_ref_point();
	if (lin=pp->touching(datai)) { l->AddRange(lin); };
	if (l->Count > 0) return l;

	if (prcur != ProfileCursors::None) {
		if (lin=cp1->touching(datai)) { l->AddRange(lin); return l; };
		if (lin=cp2->touching(datai)) { l->AddRange(lin); return l; };
	}
	if (l->Count > 0) return l;

	if (l->Count < 1) {
		// Check for Line Hit
		vector screen=c->t->toscreen(datai);
		vector A=c->t->toscreen(p1->data);
		vector B=c->t->toscreen(p2->data);
		double ldx=fabs(getinlinedist(A,B,screen));
		if (ldx < 3) {
			l->Add(this);
			return l;
		}
		return nullptr;
	}
	return l;
}

List<cmeasurement^> ^cprofile::isinside(vector data1, vector data2) {
	List<cmeasurement^> ^l=gcnew List<cmeasurement^>();
	List<cmeasurement^> ^lin;
	// if (drawrefpoints) {
		if (lin=p1->isinside(data1,data2)) { l->AddRange(lin); };
		if (lin=p2->isinside(data1,data2)) { l->AddRange(lin); };
	// }
	if (l->Count >= 2) {
		delete l;
		l=gcnew List<cmeasurement^>();
		l->Add(this);
		return l;
	}
	calc_ref_point();
	if (lin=pp->isinside(data1,data2)) { l->AddRange(lin); };
	if (l->Count < 1) 
		return nullptr;
	return l;
}

void cprofile::moveto(vector data_from, vector data_to) {
	if (selected) {
		p1->moveto(data_from,data_to);
		p2->moveto(data_from,data_to);
	} else {
		if (p1->selected) p1->moveto(data_from,data_to);
		if (p2->selected) p2->moveto(data_from,data_to);
	}
}

void cprofile::storestate() {
	p1->storestate();
	p2->storestate();
	pp->storestate();
	cp1->storestate();
	cp2->storestate();
}

void cprofile::restorestate() {
	p1->restorestate();
	p2->restorestate();
	pp->restorestate();
	cp1->restorestate();
	cp2->restorestate();
}

void cprofile::select() {
	selected=1;
	p1->unselect();
	p2->unselect();
	pp->unselect();
	cp1->unselect();
	cp2->unselect();
}

void cprofile::unselect() {
	selected=0;
	p1->unselect();
	p2->unselect();
	pp->unselect();
	cp1->unselect();
	cp2->unselect();
}

//******************************************************* C C I R C L E 3 P ***********************************************************

vector getintersect(vector A,vector V, vector B,vector W, int *valid) {
	matrix m=matrix(V.x,-W.x,V.y,-W.y);
	if (m.det() == 0) {
		*valid=0;
		return vector(0,0);
	}
	*valid=1;
	vector r=m.inv()*(B-A);
	return A+V*r.x;
}

void ccircle3p::setcenter() {
	vector A=c->c->getreal(p1->data);
	vector B=c->c->getreal(p2->data);
	vector C=c->c->getreal(p3->data);
	vector E=(A+B)*0.5;
	vector F=(B+C)*0.5;
	vector AB=B-A;
	vector BC=C-B;
	if ((AB.len <= 0) || (BC.len <= 0)) {
		// Bad Point
		rad=-1;
		center->real=(A+B+C)/3;
		center->data=c->c->getdata(center->real);
		return;
	}
	vector P=vector(-AB.y,AB.x);
	vector Q=vector(-BC.y,BC.x);
	if ((P^Q)==0) {
		// Bad Point
		rad=-1;
		center->real=(A+B+C)/3;
		center->data=c->c->getdata(center->real);
		return;
	}
	int valid;
	vector ctr=getintersect(E,P,F,Q,&valid);
	if (!valid) {
		// Bad Point
		rad=-1;
		center->real=(A+B+C)/3;
		center->data=c->c->getdata(center->real);
		return;
	}
	center->real=ctr;
	center->data=c->c->getdata(center->real);
	ctr=p1->real-ctr;
	rad=ctr.len;
}

void ccircle3p::ccircle3p_default() {
	p1=nullptr;
	p2=nullptr;
	p3=nullptr;
	center=nullptr;
	pp=nullptr;
	radphi=M_PI/2;
	drawrefpoints=1;
	showradline=1;
	showdiameter = 0;
	showfullcircle = 0;

//	LineCol=Color::FromArgb(255,255,0);
//	fnt=gcnew System::Drawing::Font(FontFamily::GenericSansSerif,8.0F,FontStyle::Regular);

}

ccircle3p::ccircle3p(vector data1, vector data2, vector data3, csys ^cs):cmeasurement("circle3p",cs) {
	ccircle3p_default();
	p1=gcnew cpoint(data1, cs);
	p2=gcnew cpoint(data2, cs);
	p3=gcnew cpoint(data3, cs);
	p1->dmode=PointDisplayMode::PointOnly;p1->pts=PointStyles::Dot;
	p2->dmode=PointDisplayMode::PointOnly;p2->pts=PointStyles::Dot;
	p3->dmode=PointDisplayMode::PointOnly;p3->pts=PointStyles::Dot;
	center=gcnew cpoint(vector(0,0),cs);
	center->dmode=PointDisplayMode::PointOnly;center->pts=PointStyles::Plus;
	setcenter();
	pp=gcnew crefpoint(data1,cs);
	pp->setcallback(this);


}

ccircle3p::ccircle3p(vector data1, vector data2, csys ^cs):cmeasurement("circle3p",cs) {
	ccircle3p_default();
	p1=gcnew cpoint(data1, c);
	p2=gcnew cpoint((data2+data1)/2, c);
	p3=gcnew cpoint(data2, c);
	p1->dmode=PointDisplayMode::PointOnly;p1->pts=PointStyles::Dot;
	p2->dmode=PointDisplayMode::PointOnly;p2->pts=PointStyles::Dot;
	p3->dmode=PointDisplayMode::PointOnly;p3->pts=PointStyles::Dot;
	center=gcnew cpoint(vector(0,0),c);
	center->dmode=PointDisplayMode::PointOnly;center->pts=PointStyles::Plus;
	setcenter();
	pp=gcnew crefpoint(data1,c);
	pp->setcallback(this);
}

ccircle3p::ccircle3p(csys ^c, ia_xml_read ^x):cmeasurement("circle3p",c) {
	int foundp1,foundp2,foundp3,foundcenter,rv;
	foundp1=foundp2=foundp3=foundcenter=0;

	ccircle3p_default();

	while (1) {
		switch (x->NodeType) {
			case XmlNodeType::Element:
				if (rv=read_from_xml(x)) {
					if (rv & 0x02) foundp1=1;
					if (rv & 0x04) foundp2=1;
					if (rv & 0x08) foundp3=1;
					if (rv & 0x10) foundcenter=1;
				} else {
					// Unknown element --> Skip over
					if (x->IsEmptyElement)	x->Read(); else x->Skip();
				}
				break;
			case XmlNodeType::EndElement:
				if (!foundp1 || !foundp2 || !foundp3) 
					throw gcnew System::Exception("Bad XML Formatted File <CIRCLE3P> tag");
				x->Read();
				pp=gcnew crefpoint((p1->data+p3->data)/2,c);
				pp->setcallback(this);
				if (!foundcenter) center=gcnew cpoint(vector(0,0),c);
				setcenter();
				return;
			default:
				throw gcnew System::Exception("Bad XML Formatted File");
		}
	}
}

void ccircle3p::update_from_template(cmeasurement ^templ) {
	if (templ == nullptr) return;
	cmeasurement::update_from_template(templ);
//	LineCol=templ->LineCol;
	drawrefpoints=((ccircle3p^)templ)->drawrefpoints;
	showradline=((ccircle3p^)templ)->showradline;
	showdiameter= ((ccircle3p^)templ)->showdiameter;
	showfullcircle = ((ccircle3p^)templ)->showfullcircle;
//	fnt=gcnew Font(templ->fnt,templ->fnt->Style);
}

String ^ccircle3p::getdesc(void) {
	if (rad > 0) {
		if (name->Length > 0) 
			return gcnew String(name + " = " + c->f->format(rad));
		else
			return gcnew String("R = "+c->f->format(rad));
	} else {
		if (name->Length > 0) 
			return gcnew String(name + " = undefined");
		else
			return gcnew String("R = undefined");
	}
}

String ^ccircle3p::exportCSV(void) {
	vector A,B,C;
	A=c->c->getreal(p1->data);
	B=c->c->getreal(p2->data);
	C=c->c->getreal(p3->data);
	if (rad > 0) {
		return String::Format("Circle3P\t{0}\t{1}\t{2}\t{3}\n",name,center->real.x,center->real.y,rad) +
			String::Format("\t\t{0}\t{1}\n\t\t{2}\t{3}\n\t\t{4}\t{5}\n",A.x,A.y,B.x,B.y,C.x,C.y);
	} else {
		return String::Format("Circle3P\t{0}\tNAN\tNAN\tNAN\n",name) +
			String::Format("\t\t{0}\t{1}\n\t\t{2}\t{3}\n\t\t{4}\t{5}\n",A.x,A.y,B.x,B.y,C.x,C.y);
	}
}

void ccircle3p::calc_ref_point() {
	if (rad <= 0) {
		pp->data=p1->data;
		// pp->real=c->c->getreal(pp->data);
	} else {
		vector rv=center->real+vector(cos(radphi),sin(radphi))*rad;
		pp->data=c->c->getdata(rv);
	}
}


void ccircle3p::draw(Graphics ^g){
	Pen ^pen;

	p1->real=c->real_from_data(p1->data);
	p2->real=c->real_from_data(p2->data);
	p3->real=c->real_from_data(p3->data);

	if (drawrefpoints)	{
		// Draw points
		p1->draw(g);
		p2->draw(g);
		p3->draw(g);
	}

	setcenter();

	if (rad >=0) 
		center->draw(g);

	vector cs=c->t->toscreen(c->c->getdata(center->real));
	int onscreen=0;
	if ((cs.x >= 0) && (cs.x < c->t->screen.x) && (cs.y >= 0) && (cs.y < c->t->screen.y))
		onscreen=1;

	calc_ref_point();
	if (selected) pp->draw(g);

	if (selected)
		pen=gcnew Pen( Color::FromArgb(255-linecol.R,255-linecol.G,255-linecol.B),2.0f );
	else
		pen=gcnew Pen( linecol,1.0f );


	if (rad >= 0) {
		double ph1=atan2(p1->real.y-center->real.y,p1->real.x-center->real.x);
		double ph2=atan2(p2->real.y-center->real.y,p2->real.x-center->real.x);
		double ph3=atan2(p3->real.y-center->real.y,p3->real.x-center->real.x);
		double ph4=radphi;

		double dphi1=minangulardiff(ph2,ph1);
		double dphi2=minangulardiff(ph3,ph2);
		// double dphi3=minangulardiff(ph3,ph1);

		if (showfullcircle) {
			for (int i = 0; i < 72; i++) {
				double phi1, phi2;
				phi1 = (double)i / 72 * 2*M_PI;
				phi2 = (double)(i + 1) / 72 * 2 * M_PI;
				vector v1 = c->t->toscreen(c->c->getdata(center->real + vector(cos(phi1), sin(phi1))*rad));
				vector v2 = c->t->toscreen(c->c->getdata(center->real + vector(cos(phi2), sin(phi2))*rad));
				drawline(g, v1, v2, pen);
			}
		}
		else {
			for (int i = 0; i < 36; i++) {
				double phi1, phi2;
				phi1 = (double)i / 36 * dphi1 + ph1;
				phi2 = (double)(i + 1) / 36 * dphi1 + ph1;
				vector v1 = c->t->toscreen(c->c->getdata(center->real + vector(cos(phi1), sin(phi1))*rad));
				vector v2 = c->t->toscreen(c->c->getdata(center->real + vector(cos(phi2), sin(phi2))*rad));
				drawline(g, v1, v2, pen);
				phi1 = (double)i / 36 * dphi2 + ph2;
				phi2 = (double)(i + 1) / 36 * dphi2 + ph2;
				v1 = c->t->toscreen(c->c->getdata(center->real + vector(cos(phi1), sin(phi1))*rad));
				v2 = c->t->toscreen(c->c->getdata(center->real + vector(cos(phi2), sin(phi2))*rad));
				drawline(g, v1, v2, pen);
			}
		}

		//SolidBrush ^fb=gcnew SolidBrush(fntcol);

		vector r=vector(cos(radphi),sin(radphi))*rad;
		vector v=center->real+r;
		vector sl=c->t->toscreen(c->c->getdata(v))-c->t->toscreen(c->c->getdata(center->real));
		vector bx=c->t->toscreen(c->c->getdata(v));
		vector vx=sl/sl.len;
		vector vy=vector(-vx.y,vx.x);
		drawline(g,bx,bx+vx*20,pen);
		drawline(g,bx,bx+vx*5+vy*5,pen);
		drawline(g,bx,bx+vx*5-vy*5,pen);
		if (showdiameter) {
			if (name->Length > 0)
				drawtxt(g, bx + vx * 20, vx, -1, 0, name + " = " + c->f->format(2*rad));
			else
				drawtxt(g, bx + vx * 20, vx, -1, 0, "D = " + c->f->format(2*rad));
		}
		else {
			if (name->Length > 0)
				drawtxt(g, bx + vx * 20, vx, -1, 0, name + " = " + c->f->format(rad));
			else
				drawtxt(g, bx + vx * 20, vx, -1, 0, "R = " + c->f->format(rad));
		}

		if (showradline) {
			drawtickline(g, center->real, v, pen, 1, nullptr);
		}
		//delete fb;
	} else {
		vector v1=c->t->toscreen(p1->data);
		vector v2=c->t->toscreen(p2->data);
		vector v3=c->t->toscreen(p3->data);
		drawline(g,v1,v2,pen);
		drawline(g,v2,v3,pen);
	}
}

List<cmeasurement^>^ ccircle3p::touching(vector datai) {
	List<cmeasurement^> ^l=gcnew List<cmeasurement^>();
	List<cmeasurement^> ^lin;
	if (drawrefpoints) {
		if (lin=p1->touching(datai)) { l->AddRange(lin); };
		if (lin=p2->touching(datai)) { l->AddRange(lin); };
		if (lin=p3->touching(datai)) { l->AddRange(lin); };
	}
	setcenter();
	calc_ref_point();
	if (lin=pp->touching(datai)) { l->AddRange(lin); };
	if ((l->Count < 1) && (rad > 0)) {
		// Check for Center Hit
		if (lin=center->touching(datai)) { l->AddRange(lin); };
		if (l->Count > 0) return l;
		vector v=c->c->getreal(datai+vector(3,0))-center->real;
		double r1=v.len;
		v=c->c->getreal(datai+vector(0,3))-center->real;
		double r2=v.len;
		v=c->c->getreal(datai+vector(-3,0))-center->real;
		double r3=v.len;
		v=c->c->getreal(datai+vector(0,-3))-center->real;
		double r4=v.len;
		double rmin=r1;
		double rmax=r1;
		if (r2<rmin) rmin=r2;if (r3<rmin) rmin=r3;if (r4<rmin) rmin=r4;
		if (r2>rmax) rmax=r2;if (r3>rmax) rmax=r3;if (r4>rmax) rmax=r4;
		if ((rmax >= rad) && (rmin <= rad)) {
			l->Add(this);
			return l;
		}
	}
	if (l->Count < 1)
		return nullptr; else return l;
}

List<cmeasurement^>^ ccircle3p::isinside(vector data1, vector data2){
	List<cmeasurement^> ^l=gcnew List<cmeasurement^>();
	List<cmeasurement^> ^lin;
	if (drawrefpoints) {
		if (lin=p1->isinside(data1,data2)) { l->AddRange(lin); };
		if (lin=p2->isinside(data1,data2)) { l->AddRange(lin); };
		if (lin=p3->isinside(data1,data2)) { l->AddRange(lin); };
	}
	setcenter();
	calc_ref_point();
	if (lin=pp->isinside(data1,data2)) { l->AddRange(lin); };
	if (l->Count < 1) 
		return nullptr;
	return l;
}

void ccircle3p::moveto(vector data_from, vector data_to){
	if (selected) {
		p1->moveto(data_from,data_to);
		p2->moveto(data_from,data_to);
		p3->moveto(data_from,data_to);
		setcenter();
		calc_ref_point();
	} else {
		if (p1->selected) p1->moveto(data_from,data_to);
		if (p2->selected) p2->moveto(data_from,data_to);
		if (p3->selected) p3->moveto(data_from,data_to);
	}
}

void ccircle3p::storestate(){
	p1->storestate();
	p2->storestate();
	p3->storestate();
	pp->storestate();
}

void ccircle3p::restorestate(){
	p1->restorestate();
	p2->restorestate();
	p3->restorestate();
	pp->restorestate();
}

void ccircle3p::refchangenotify(cmeasurement ^csrc){
	vector v=c->c->getreal(pp->data)-center->real;
	radphi = atan2(v.y,v.x);
}

void ccircle3p::select(void){
	selected=1;
	p1->unselect();
	p2->unselect();
	p3->unselect();
	pp->unselect();
	center->unselect();
}

void ccircle3p::unselect(void){
	selected=0;
	p1->unselect();
	p2->unselect();
	p3->unselect();
	pp->unselect();
	center->unselect();
}

void ccircle3p::write_to_xml(ia_xml_write ^x) {
	cmeasurement::write_to_xml(x);
	
	x->WriteStartElement("P1");
		p1->write_to_xml(x);
	x->WriteEndElement();x->WriteWhitespace("\n");
	x->WriteStartElement("P2");
		p2->write_to_xml(x);
	x->WriteEndElement();x->WriteWhitespace("\n");
	x->WriteStartElement("P3");
		p3->write_to_xml(x);
	x->WriteEndElement();x->WriteWhitespace("\n");
	x->WriteStartElement("CENTER");
		center->write_to_xml(x);
	x->WriteEndElement();x->WriteWhitespace("\n");
	
	x->write_value("PHI",radphi);
	x->write_value("DRAWREFPOINTS",drawrefpoints);
	x->write_value("SHOWRADLINE", showradline);
	x->write_value("SHOWDIAMETER", showdiameter);
	x->write_value("SHOWFULLCIRCLE", showfullcircle);

}

int ccircle3p::read_from_xml(ia_xml_read ^x) {
	if (cmeasurement::read_from_xml(x)) return 1;

	if (String::Compare(x->Name,"P1")==0) {
		x->Read();
		p1=gcnew cpoint(c,x);
		return 0x02;
	} else if (String::Compare(x->Name,"P2")==0) {
		x->Read();
		p2=gcnew cpoint(c,x);
		return 0x04;
	} else if (String::Compare(x->Name,"P3")==0) {
		x->Read();
		p3=gcnew cpoint(c,x);
		return 0x08;
	} else if (String::Compare(x->Name,"CENTER")==0) {
		x->Read();
		center=gcnew cpoint(c,x);
		return 0x10;
	} else if (String::Compare(x->Name,"PHI")==0) {
		radphi=x->ReadElementContentAsDouble();
	} else if (String::Compare(x->Name,"DRAWREFPOINTS")==0) {
		drawrefpoints=x->ReadElementContentAsInt();
	} else if (String::Compare(x->Name, "SHOWRADLINE") == 0) {
		showradline = x->ReadElementContentAsInt();
	} else if (String::Compare(x->Name, "SHOWDIAMETER") == 0) {
		showdiameter = x->ReadElementContentAsInt();
	} else if (String::Compare(x->Name, "SHOWFULLCIRCLE") == 0) {
		showfullcircle = x->ReadElementContentAsInt();
	} else return 0;
	return 1;
}


//******************************************************* C A N G L E ***********************************************************

void cangle::cangle_default() {
	p1=nullptr;
	p2=nullptr;
	p3=nullptr;
	pp=nullptr;
	displayrad=0;
	drawrefpoints=1;
	drawarrow=1;
	noneg=0;
	phi=0;
}

cangle::cangle(vector data1, vector data2, vector data3, csys ^cs):cmeasurement("angle",cs) {
	cangle_default();
	p1=gcnew cpoint(data1, cs);
	p2=gcnew cpoint(data2, cs);
	p3=gcnew cpoint(data3, cs);
	p1->dmode=PointDisplayMode::PointOnly;p1->pts=PointStyles::Plus;
	p2->dmode=PointDisplayMode::PointOnly;p2->pts=PointStyles::Dot;
	p3->dmode=PointDisplayMode::PointOnly;p3->pts=PointStyles::Dot;
	pp=gcnew crefpoint(data1,cs);
	pp->setcallback(this);
	phi=angulardiff(c->real_from_data(p3->data)-c->real_from_data(p1->data),
		c->real_from_data(p2->data)-c->real_from_data(p1->data));
	if (noneg && phi < 0) phi+=2*M_PI;
	displayrad=(len(p2->data-p1->data)+
		len(p3->data-p1->data))/2;
	calc_ref_point();
}

cangle::cangle(csys ^c, ia_xml_read ^x):cmeasurement("angle",c) {
	int foundp1,foundp2,foundp3,foundr,rv;
	foundp1=foundp2=foundp3=foundr=0;

	cangle_default();

	while (1) {
		switch (x->NodeType) {
			case XmlNodeType::Element:
				if (rv=read_from_xml(x)) {
					if (rv & 0x02) foundp1=1;
					if (rv & 0x04) foundp2=1;
					if (rv & 0x08) foundp3=1;
					if (rv & 0x10) foundr=1;
				} else {
					// Unknown element --> Skip over
					if (x->IsEmptyElement)	x->Read(); else x->Skip();
				}
				break;
			case XmlNodeType::EndElement:
				if (!foundp1 || !foundp2 || !foundp3) 
					throw gcnew System::Exception("Bad XML Formatted File <CIRCLE3P> tag");
				x->Read();
				phi=angulardiff(c->real_from_data(p3->data)-c->real_from_data(p1->data),
					c->real_from_data(p2->data)-c->real_from_data(p1->data));
				if (noneg && phi < 0) phi+=2*M_PI;
				pp=gcnew crefpoint((p1->data+p3->data)/2,c);
				pp->setcallback(this);
				if (!foundr) {
					displayrad=(len(p2->data-p1->data)+
						len(p3->data-p1->data))/2;
				}
				calc_ref_point();
				return;
			default:
				throw gcnew System::Exception("Bad XML Formatted File");
		}
	}
}

void cangle::update_from_template(cmeasurement ^templ) {
	if (templ == nullptr) return;
	cmeasurement::update_from_template(templ);
	drawrefpoints=((cangle^)templ)->drawrefpoints;
	drawarrow=((cangle^)templ)->drawarrow;
	noneg=((cangle^)templ)->noneg;
}

String ^cangle::getdesc(void) {
	if (phi != 0) {
		if (name->Length > 0) 
			return gcnew String(name + " = " + c->f->format_angle(phi));
		else
			return gcnew String("PHI = "+ c->f->format_angle(phi));
	} else {
		if (name->Length > 0) 
			return gcnew String(name + " = undefined");
		else
			return gcnew String("PHI = undefined");
	}
}

String ^cangle::exportCSV(void) {
	vector A,B,C;
	A=c->c->getreal(p1->data);
	B=c->c->getreal(p2->data);
	C=c->c->getreal(p3->data);
	return String::Format("Angle\t{0}\t{1}\t{2}\t{3}\n",name,A.x,A.y,phi) +
		String::Format("\t\t{0}\t{1}\n\t\t{2}\t{3}\n",B.x,B.y,C.x,C.y);
}

void cangle::calc_ref_point() {
	if (displayrad > 0) {
		p1->real=c->real_from_data(p1->data);
		p2->real=c->real_from_data(p2->data);
		p3->real=c->real_from_data(p3->data);
		double rrad=len(c->real_from_data(p1->data+norm(p2->data-p1->data)*displayrad)-p1->real);
		double phi2=atan2(p2->real.y-p1->real.y,p2->real.x-p1->real.x);
		vector v=p1->real+vector(cos(phi2+phi/2),sin(phi2+phi/2))*rrad;
		pp->data=c->data_from_real(v);
	} else {
		pp->data=(p1->data+p2->data+p3->data)/3;
	}
}

void cangle::draw(Graphics ^g){
	Pen ^pen;
	double phi2,phi3;

	p1->real=c->real_from_data(p1->data);
	p2->real=c->real_from_data(p2->data);
	p3->real=c->real_from_data(p3->data);

	if (drawrefpoints)	{
		// Draw points
		p1->draw(g);
		p2->draw(g);
		p3->draw(g);
	}

	phi2=atan2(p2->real.y-p1->real.y,p2->real.x-p1->real.x);
	phi = angulardiff(p3->real-p1->real,p2->real-p1->real);
	if (noneg && phi < 0) phi+=2*M_PI;
	phi3=phi2+phi;

	calc_ref_point();
	if (selected) pp->draw(g);

	if (selected)
		pen=gcnew Pen( Color::FromArgb(255-linecol.R,255-linecol.G,255-linecol.B),2.0f );
	else
		pen=gcnew Pen( linecol,1.0f );
	
	double rrad=len(c->real_from_data(p1->data+norm(p2->data-p1->data)*displayrad)-p1->real);

	for (int i=0;i<36;i++) {
		double ph1,ph2;
		ph1=(double)i/36*phi+phi2;
		ph2=(double)(i+1)/36*phi+phi2;
		vector v1=c->t->toscreen(c->c->getdata(p1->real+vector(cos(ph1),sin(ph1))*rrad));
		vector v2=c->t->toscreen(c->c->getdata(p1->real+vector(cos(ph2),sin(ph2))*rrad));
		drawline(g,v1,v2,pen);
	}

	if (drawarrow) {
		vector vx=c->t->toscreen(c->c->getdata(p1->real+vector(cos(phi3),sin(phi3))*rrad));
		vector vr=norm(vx-c->t->toscreen(p1->data));
		vector vt=norm(vector(-vr.y,vr.x));
		if (phi < 0) vt*=-1;
		drawline(g,vx,vx+vr*5+vt*5,pen);
		drawline(g,vx,vx-vr*5+vt*5,pen);
		drawline(g,vx-vr*5,vx+vr*5,pen);

		vx=c->t->toscreen(c->c->getdata(p1->real+vector(cos(phi2),sin(phi2))*rrad));
		vr=norm(vx-c->t->toscreen(p1->data));
		vt=norm(vector(-vr.y,vr.x));
		if (phi > 0) vt*=-1;
		drawline(g,vx,vx+vr*5+vt*5,pen);
		drawline(g,vx,vx-vr*5+vt*5,pen);
		drawline(g,vx-vr*5,vx+vr*5,pen);
	}

	if (len(p2->real-p1->real) < rrad) 
		drawline(g,c->t->toscreen(p1->data),
			c->t->toscreen(c->c->getdata(p1->real+norm(p2->real-p1->real)*rrad)),pen);
	else
		drawline(g,c->t->toscreen(p1->data),c->t->toscreen(p2->data),pen);

	if (len(p3->real-p1->real) < rrad) 
		drawline(g,c->t->toscreen(p1->data),
			c->t->toscreen(c->c->getdata(p1->real+norm(p3->real-p1->real)*rrad)),pen);
	else
		drawline(g,c->t->toscreen(p1->data),c->t->toscreen(p3->data),pen);

	vector vc=p1->real+vector(cos(phi2+phi/2),sin(phi2+phi/2))*rrad;
	vector vcc=p1->real+vector(cos(phi2+phi/2),sin(phi2+phi/2))*(rrad+1);
	vector vr=c->t->toscreen(c->c->getdata(vc));
	vector vx=c->t->toscreen(c->c->getdata(vcc))-vr;

	if (name->Length > 0)
		drawtxt(g,vr,vx,-1,0,name + " = " + c->f->format_angle(fabs(phi)));
	else
		drawtxt(g,vr,vx,-1,0,c->f->format_angle(fabs(phi)));
}

List<cmeasurement^>^ cangle::touching(vector datai) {
	List<cmeasurement^> ^l=gcnew List<cmeasurement^>();
	List<cmeasurement^> ^lin;

	p1->real=c->real_from_data(p1->data);
	p2->real=c->real_from_data(p2->data);
	p3->real=c->real_from_data(p3->data);

	if (drawrefpoints) {
		if (lin=p1->touching(datai)) { l->AddRange(lin); };
		if (lin=p2->touching(datai)) { l->AddRange(lin); };
		if (lin=p3->touching(datai)) { l->AddRange(lin); };
	}
	calc_ref_point();
	if (lin=pp->touching(datai)) { l->AddRange(lin); };

	if ((l->Count < 1) && (displayrad > 0)) {
		// Check for Line Hit
		vector screen=c->t->toscreen(datai);
		vector real=c->c->getreal(datai);
		vector A=c->t->toscreen(p1->data);
		vector B;
		double ldx;
		double rrad=len(c->real_from_data(p1->data+norm(p2->data-p1->data)*displayrad)-p1->real);

		if (len(p2->real-p1->real) < rrad)
			B=c->t->toscreen(c->c->getdata(p1->real+norm(p2->real-p1->real)*rrad));
		else
			B=c->t->toscreen(p2->data);
		ldx=fabs(getinlinedist(A,B,screen));
		if (ldx < 3) { l->Add(this);return l; };

		if (len(p3->real-p1->real) < rrad)		
			B=c->t->toscreen(c->c->getdata(p1->real+norm(p3->real-p1->real)*rrad));
		else
			B=c->t->toscreen(p3->data);
		ldx=fabs(getinlinedist(A,B,screen));
		if (ldx < 3) { l->Add(this);return l; };

		double r1=len(c->real_from_screen(screen+vector(3,0))-p1->real);
		double r2=len(c->real_from_screen(screen+vector(0,3))-p1->real);
		double r3=len(c->real_from_screen(screen+vector(-3,0))-p1->real);
		double r4=len(c->real_from_screen(screen+vector(0,-3))-p1->real);
		double rmin=r1;
		double rmax=r1;
		if (r2<rmin) rmin=r2;if (r3<rmin) rmin=r3;if (r4<rmin) rmin=r4;
		if (r2>rmax) rmax=r2;if (r3>rmax) rmax=r3;if (r4>rmax) rmax=r4;
		if ((rmax >= rrad) && (rmin <= rrad)) {
			double a=angulardiff(real-p1->real,p2->real-p1->real);
			if (noneg && (a < 0)) a+=2*M_PI;
			if ((phi > 0) && (a >= 0) && (a <= phi)) {
				l->Add(this);
				return l;
			} 
			if ((phi < 0) && (a <= 0) && (a >= phi)) {
				l->Add(this);
				return l;
			} 
		}
	}
	if (l->Count < 1)
		return nullptr; else return l;
}

List<cmeasurement^>^ cangle::isinside(vector data1, vector data2){
	List<cmeasurement^> ^l=gcnew List<cmeasurement^>();
	List<cmeasurement^> ^lin;

//	p1->real=c->real_from_data(p1->data);
//	p2->real=c->real_from_data(p2->data);
//	p3->real=c->real_from_data(p3->data);


	if (drawrefpoints) {
		if (lin=p1->isinside(data1,data2)) { l->AddRange(lin); };
		if (lin=p2->isinside(data1,data2)) { l->AddRange(lin); };
		if (lin=p3->isinside(data1,data2)) { l->AddRange(lin); };
	}
	calc_ref_point();
	if (lin=pp->isinside(data1,data2)) { l->AddRange(lin); };
	if (l->Count < 1) 
		return nullptr;
	return l;
}

void cangle::moveto(vector data_from, vector data_to){
	if (selected) {
		p1->moveto(data_from,data_to);
		p2->moveto(data_from,data_to);
		p3->moveto(data_from,data_to);
		calc_ref_point();
	} else {
		if (p1->selected) p1->moveto(data_from,data_to);
		if (p2->selected) p2->moveto(data_from,data_to);
		if (p3->selected) p3->moveto(data_from,data_to);
	}
}

void cangle::storestate() {
	p1->storestate();
	p2->storestate();
	p3->storestate();
	pp->storestate();
	displayrad_store=displayrad;
}

void cangle::restorestate() {
	p1->restorestate();
	p2->restorestate();
	p3->restorestate();
	pp->restorestate();
	displayrad=displayrad_store;
}

void cangle::refchangenotify(cmeasurement ^csrc){
	//vector v=c->c->getreal(pp->data)-p1->real;
	displayrad = len(pp->data-p1->data); // v.len;
}

void cangle::select(void){
	selected=1;
	p1->unselect();
	p2->unselect();
	p3->unselect();
	pp->unselect();
}

void cangle::unselect(void){
	selected=0;
	p1->unselect();
	p2->unselect();
	p3->unselect();
	pp->unselect();
}

void cangle::write_to_xml(ia_xml_write ^x) {
	cmeasurement::write_to_xml(x);
	
	x->WriteStartElement("P1");
		p1->write_to_xml(x);
	x->WriteEndElement();x->WriteWhitespace("\n");
	x->WriteStartElement("P2");
		p2->write_to_xml(x);
	x->WriteEndElement();x->WriteWhitespace("\n");
	x->WriteStartElement("P3");
		p3->write_to_xml(x);
	x->WriteEndElement();x->WriteWhitespace("\n");
	
	x->write_value("PHI",phi);
	x->write_value("DISPLAYRAD",displayrad);
	x->write_value("DRAWREFPOINTS",drawrefpoints);
	x->write_value("DRAWARROW",drawarrow);
	x->write_value("NONEG",noneg);

}

int cangle::read_from_xml(ia_xml_read ^x) {
	if (cmeasurement::read_from_xml(x)) return 1;

	if (String::Compare(x->Name,"P1")==0) {
		x->Read();
		p1=gcnew cpoint(c,x);
		return 0x02;
	} else if (String::Compare(x->Name,"P2")==0) {
		x->Read();
		p2=gcnew cpoint(c,x);
		return 0x04;
	} else if (String::Compare(x->Name,"P3")==0) {
		x->Read();
		p3=gcnew cpoint(c,x);
		return 0x08;
	} else if (String::Compare(x->Name,"DISPLAYRAD")==0) {
		displayrad=x->ReadElementContentAsDouble();
		return 0x10;
	} else if (String::Compare(x->Name,"PHI")==0) {
		phi=x->ReadElementContentAsDouble();
	} else if (String::Compare(x->Name,"DRAWREFPOINTS")==0) {
		drawrefpoints=x->ReadElementContentAsInt();
	} else if (String::Compare(x->Name,"DRAWARROW")==0) {
		drawarrow=x->ReadElementContentAsInt();
	} else if (String::Compare(x->Name,"NONEG")==0) {
		noneg=x->ReadElementContentAsInt();
	} else return 0;
	return 1;
}


//******************************************************* C A R E A ***********************************************************

void carea::carea_default() {
	plist=nullptr;
	area=0;
	length=0;
	perimeter=0;

	fillcol=Color::FromArgb(0,127,64);
	fillalp=0.5;

	drawrefpoints=1;
	closed=1;
	fill=1;
	showlength=0;
	showarea=1;
	
}

carea::carea(List<vector^> ^pl, csys ^cs):cmeasurement("area",cs) {
	carea_default();
	plist=gcnew List<cpoint^>();
	for (int i=0;i<pl->Count;i++) {
		plist->Add(gcnew cpoint(*pl[i],cs));
		plist[i]->dmode=PointDisplayMode::PointOnly;
		plist[i]->pts=PointStyles::Dot;
	}
	calc_area();
}

carea::carea(csys ^c, ia_xml_read ^x):cmeasurement("area",c) {
	int foundpts,rv;
	foundpts=0;

	carea_default();
	plist=gcnew List<cpoint^>();

	while (1) {
		switch (x->NodeType) {
			case XmlNodeType::Element:
				if (rv=read_from_xml(x)) {
					if (rv & 0x02) foundpts++;
				} else {
					// Unknown element --> Skip over
					if (x->IsEmptyElement)	x->Read(); else x->Skip();
				}
				break;
			case XmlNodeType::EndElement:
				if ((foundpts != 0) && (foundpts < 3)) 
					throw gcnew System::Exception("Bad XML Formatted File <AREA> tag");
				x->Read();
				calc_area();
				return;
			default:
				throw gcnew System::Exception("Bad XML Formatted File <AREA> tag");
		}
	}
}

void carea::update_from_template(cmeasurement ^templ) {
	if (templ == nullptr) return;
	cmeasurement::update_from_template(templ);

	fillcol=((carea^)templ)->fillcol;
	fillalp=((carea^)templ)->fillalp;
	drawrefpoints=((carea^)templ)->drawrefpoints;
	closed=((carea^)templ)->closed;
	fill=((carea^)templ)->fill;
	showlength=((carea^)templ)->showlength;
	showarea=((carea^)templ)->showarea;
}

String ^carea::getdesc(void) {
	if (name->Length > 0) 
		return gcnew String(name + " = " + c->f->format_area(area));
	else
		return gcnew String("A = "+ c->f->format_area(area));
}

String ^carea::exportCSV(void) {
	if ((plist == nullptr) || (plist->Count < 3)) return nullptr;
	if (closed) {// Area
		String ^s=gcnew String("");
		s+=String::Format("Area\t{0}\t{1}\t{2}\n",name,area,perimeter);
		s+=String::Format("\tCOG\t{0}\t{1}\n",cog.x,cog.y);
		for (int i=0;i<plist->Count;i++) {
			vector pn=c->real_from_data(plist[i]->data);
			s+=String::Format("\t\t{0}\t{1}\n",pn.x,pn.y);
		}
		return s;
	} else { // Path
		String ^s=gcnew String("");
		s+=String::Format("Path\t{0}\t{1}\n",name,length);
		for (int i=0;i<plist->Count;i++) {
			vector pn=c->real_from_data(plist[i]->data);
			s+=String::Format("\t\t{0}\t{1}\n",pn.x,pn.y);
		}
		return s;
	}
}

void carea::calc_area() {
	double cogx,cogy;
	area=0;
	cogx=cogy=0;
	length=0;
	perimeter=0;
	for (int i=0;i<plist->Count;i++) {
		vector pn=c->real_from_data(plist[i]->data);
		vector pm;
		if (i+1 < plist->Count) 
			pm=c->real_from_data(plist[i+1]->data);
		else 
			pm=c->real_from_data(plist[0]->data);
		perimeter+=len(pm-pn);
		if (i+1 < plist->Count)
			length+=len(pm-pn);
		area+=pn^pm;
		cogx+=(pn.x+pm.x)*(pn.x*pm.y-pm.x*pn.y);
		cogy+=(pn.y+pm.y)*(pn.x*pm.y-pm.x*pn.y);
	}
	area=area/2;
	cog=vector(cogx/6/area,cogy/6/area);
}

void carea::draw(Graphics ^g){
	Pen ^pen;

	for (int i=0;i<plist->Count;i++) 
		plist[i]->real=c->real_from_data(plist[i]->data);


	if (drawrefpoints)	{
		// Draw points
		for (int i=0;i<plist->Count;i++) 
			plist[i]->draw(g);
	}

	calc_area();

	if (selected)
		pen=gcnew Pen( Color::FromArgb(255-linecol.R,255-linecol.G,255-linecol.B),2.0f );
	else
		pen=gcnew Pen( linecol,1.0f );

	array<PointF> ^gpl = gcnew array<PointF>(plist->Count);

	for (int i=0;i<plist->Count;i++) {
		vector v1=c->screen_from_data(plist[i]->data);
		gpl[i].X=v1.x;
		gpl[i].Y=v1.y;
	}
	if (fill && fillalp > 0) {
		Brush ^b=gcnew SolidBrush(Color::FromArgb((int)floor(fillalp*100+0.5),fillcol.R,fillcol.G,fillcol.B));
		g->FillPolygon(b,gpl);
		delete b;
	}
	for (int i=0;i<plist->Count-1;i++) {
		drawline(g,vector(gpl[i].X,gpl[i].Y),
			vector(gpl[i+1].X,gpl[i+1].Y),pen);
	} 
	if (closed) {
		drawline(g,vector(gpl[plist->Count-1].X,gpl[plist->Count-1].Y),
			vector(gpl[0].X,gpl[0].Y),pen);
	}

	if (showarea) {
		vector vr=c->screen_from_real(cog);
		vector vx=vector(1,0);
		if (name->Length > 0)
			drawtxt(g,vr,vx,0,0,name + " = " + c->f->format_area(fabs(area)));
		else
			drawtxt(g,vr,vx,0,0,c->f->format_area(fabs(area)));
	}

	if (showlength) {
		double sv=(closed)?perimeter/2:length/2;
		int sgm=0;
		int sp=0;
		while (sv > len(plist[(sp+1)%plist->Count]->real-plist[sp]->real)) {
			sv-=len(plist[(sp+1)%plist->Count]->real-plist[sp]->real);
			sp=(sp+1)%plist->Count;
		}
		vector v1=vector(gpl[sp].X,gpl[sp].Y);
		vector v2=vector(gpl[(sp+1)%plist->Count].X,gpl[(sp+1)%plist->Count].Y);
		vector vx=norm(v2-v1);
		if (name->Length > 0)
			drawtxt(g,(v1+v2)/2,vx,0,-1,name + " = " + c->f->format(2*sv));
		else
			drawtxt(g,(v1+v2)/2,vx,0,-1,c->f->format(2*sv));
	}
	delete gpl;
}

List<cmeasurement^>^ carea::touching(vector datai) {
	List<cmeasurement^> ^l=gcnew List<cmeasurement^>();
	List<cmeasurement^> ^lin;
	if (drawrefpoints) {
		for (int i=0;i<plist->Count;i++) {
			if (lin=plist[i]->touching(datai)) { l->AddRange(lin); };
		}
	}

	if (l->Count < 1) {
		// Check for Line Hit
		vector screen=c->t->toscreen(datai);
		vector real=c->c->getreal(datai);
		for (int i=0;i<plist->Count-1;i++) {
			vector A=c->screen_from_data(plist[i]->data);
			vector B=c->screen_from_data(plist[i+1]->data);
			double ldx=fabs(getinlinedist(A,B,screen));
			if (ldx < 3) { l->Add(this);return l; };
		}
		if (closed) {
			vector A=c->screen_from_data(plist[plist->Count-1]->data);
			vector B=c->screen_from_data(plist[0]->data);
			double ldx=fabs(getinlinedist(A,B,screen));
			if (ldx < 3) { l->Add(this);return l; };
		}
	}
	if (l->Count < 1)
		return nullptr; else return l;
}

List<cmeasurement^>^ carea::isinside(vector data1, vector data2){
	List<cmeasurement^> ^l=gcnew List<cmeasurement^>();
	List<cmeasurement^> ^lin;
	if (drawrefpoints) {
		for (int i=0;i<plist->Count;i++) {
			if (lin=plist[i]->isinside(data1,data2)) { l->AddRange(lin); };
		}
	}
	if (l->Count < 1) 
		return nullptr;
	return l;
}

void carea::moveto(vector data_from, vector data_to){
	if (selected) {
		for (int i=0;i<plist->Count;i++)
			plist[i]->moveto(data_from,data_to);
		calc_area();
	} else {
		for (int i=0;i<plist->Count;i++)
			if (plist[i]->selected) plist[i]->moveto(data_from,data_to);
		calc_area();
	}
}

void carea::storestate() {
	for (int i=0;i<plist->Count;i++) 
		plist[i]->storestate();
}

void carea::restorestate() {
	for (int i=0;i<plist->Count;i++) 
		plist[i]->restorestate();
}

void carea::refchangenotify(cmeasurement ^csrc){
//	vector v=c->c->getreal(pp->data)-p1->real;
//	displayrad = v.len;
}

void carea::select(void){
	selected=1;
	for (int i=0;i<plist->Count;i++) 
		plist[i]->unselect();
}

void carea::unselect(void){
	selected=0;
	for (int i=0;i<plist->Count;i++) 
		plist[i]->unselect();
}

void carea::write_to_xml(ia_xml_write ^x) {
	cmeasurement::write_to_xml(x);
	
	for (int i=0;i<plist->Count;i++) {
		x->WriteStartElement("PNT");
			plist[i]->write_to_xml(x);
		x->WriteEndElement();x->WriteWhitespace("\n");
	}
	
	x->write_value("AREA",area);
	x->write_value("LENGTH",length);
	x->write_value("PERIMETER",perimeter);

	x->write_value("FILLCOL",fillcol);	
	x->write_value("FILLALP",fillalp);	
	
	x->write_value("DRAWREFPOINTS",drawrefpoints);	
	x->write_value("CLOSED",closed);
	x->write_value("FILL",fill);
	x->write_value("SHOWLENGTH",showlength);
	x->write_value("SHOWAREA",showarea);

}

int carea::read_from_xml(ia_xml_read ^x) {
	if (cmeasurement::read_from_xml(x)) return 1;

	if (String::Compare(x->Name,"PNT")==0) {
		x->Read();
		cpoint ^p=gcnew cpoint(c,x);
		plist->Add(p);
		return 0x02;
	} else if (String::Compare(x->Name,"AREA")==0) {
		area=x->ReadElementContentAsDouble();
	} else if (String::Compare(x->Name,"LENGTH")==0) {
		length=x->ReadElementContentAsDouble();
	} else if (String::Compare(x->Name,"PERIMETER")==0) {
		perimeter=x->ReadElementContentAsDouble();
	} else if (String::Compare(x->Name,"FILLCOL")==0) {
		x->Read();fillcol=x->xml_read_color();
	} else if (String::Compare(x->Name,"FILLALP")==0) {
		fillalp=x->ReadElementContentAsDouble();
	} else if (String::Compare(x->Name,"DRAWREFPOINTS")==0) {
		drawrefpoints=x->ReadElementContentAsInt();
	} else if (String::Compare(x->Name,"CLOSED")==0) {
		closed=x->ReadElementContentAsInt();
	} else if (String::Compare(x->Name,"FILL")==0) {
		fill=x->ReadElementContentAsInt();
	} else if (String::Compare(x->Name,"SHOWLENGTH")==0) {
		showlength=x->ReadElementContentAsInt();
	} else if (String::Compare(x->Name,"SHOWAREA")==0) {
		showarea=x->ReadElementContentAsInt();
	} else return 0;
	return 1;
}



//******************************************************* C O O R D I N A T E ***********************************************************

void coordinate::coordinate_default() {
	p1=nullptr;
	p2=nullptr;
	p3=nullptr;
	//	LineCol=Color::FromArgb(192,192,192);
	bkg=Color::FromArgb(255,255,255);
//	fnt=gcnew System::Drawing::Font(FontFamily::GenericSansSerif,8.0F,FontStyle::Regular);
	yscale=1;
	tiltangle=0;
	//twopoint=tp;
	//force90=0;
	drawcoord=1;
	drawmesh=0;
	drawrefpoints=1;
	xcross=0.0;
	ycross=0.0;
	xcrossauto=0;
	ycrossauto=0;
//	meshx=1;
//	meshy=1;
}

coordinate::coordinate(csys ^c, int tp):cmeasurement("CoordinateSystem",c) {
	coordinate_default();
	c->c=this;
	readonly=1;
	p1=gcnew cpoint(vector(0,0), vector(0,0), c, this);
	p2=gcnew cpoint(vector(1,0), vector(1,0), c, this);
	p3=gcnew cpoint(vector(0,1), vector(0,1), c, this);
	if (tp) 
		cmode=CoordMode::TwoPoint;
	else
		cmode=CoordMode::ThreePoint;
	calctrf(); // Sets C and M
}

coordinate::coordinate(csys ^c, ia_xml_read ^x):cmeasurement("CoordinateSystem",c) {
	int foundp1,foundp2,foundp3,foundmode,rv;
	foundp1=foundp2=foundp3=foundmode=0;

	coordinate_default();

	c->c=this;

	while (1) {
		switch (x->NodeType) {
			case XmlNodeType::Element:
				if (rv = read_from_xml(x)) {
					if (rv & 0x02) foundp1=1;
					if (rv & 0x04) foundp2=1;
					if (rv & 0x08) foundp3=1;
					if (rv & 0x10) foundmode=1;
				} else {
					// Unknown element --> Skip over
					if (x->IsEmptyElement)	x->Read(); else x->Skip();
				}
				break;
			case XmlNodeType::EndElement:
				if (!foundp1 || !foundp2 || !foundp3 || !foundmode) 
					throw gcnew System::Exception("Bad XML Formatted File <COORDINATE> tag");
				x->Read();
				calctrf(); // Sets C and M
				return;
			default:
				throw gcnew System::Exception("Bad XML Formatted File");
		}
	}
}

void coordinate::write_to_xml(ia_xml_write ^x) {
	cmeasurement::write_to_xml(x);
	x->WriteStartElement("P1");
		p1->write_to_xml(x);
	x->WriteEndElement();x->WriteWhitespace("\n");
	x->WriteStartElement("P2");
		p2->write_to_xml(x);
	x->WriteEndElement();x->WriteWhitespace("\n");
	x->WriteStartElement("P3");
		p3->write_to_xml(x);
	x->WriteEndElement();x->WriteWhitespace("\n");
	x->write_value("BKGCOLOR",bkg);
	x->write_value("YSCALE",yscale);
	x->write_value("TILTANGLE",tiltangle);
	x->write_value("MODE",(int) cmode);
//	x->write_value("TWOPOINT",twopoint);
//	x->write_value("FORCE90",force90);
	x->write_value("DRAWCOORD",drawcoord);
	x->write_value("DRAWMESH",drawmesh);
	x->write_value("DRAWREFPOINTS",drawrefpoints);
	x->write_value("XCROSS",xcross);
	x->write_value("YCROSS",ycross);
	x->write_value("XCROSSAUTO",xcrossauto);
	x->write_value("YCROSSAUTO",ycrossauto);
	x->write_value("UNIT",c->f->unit);
	x->write_value("FORMAT",c->f->form);
	x->write_value("CUNIT",c->f->cunit);
	x->write_value("CFORMAT",c->f->cform);
	// fnt=gcnew System::Drawing::Font(FontFamily::GenericSansSerif,8.0F,FontStyle::Regular);
}

int coordinate::read_from_xml(ia_xml_read ^x) {
	if (cmeasurement::read_from_xml(x)) return 1;

	if (String::Compare(x->Name,"P1")==0) {
		x->Read();
		p1=gcnew cpoint(c,x);
		p1->isref=1;p1->refsys=this;
		return 0x02;
	} else if (String::Compare(x->Name,"P2")==0) {
		x->Read();
		p2=gcnew cpoint(c,x);
		p2->isref=1;p2->refsys=this;
		return 0x04;
	} else if (String::Compare(x->Name,"P3")==0) {
		x->Read();
		p3=gcnew cpoint(c,x);
		p3->isref=1;p3->refsys=this;
		return 0x08;
	} else if (String::Compare(x->Name,"BKGCOLOR")==0) {
		x->Read();bkg=x->xml_read_color();
	} else if (String::Compare(x->Name,"YSCALE")==0) {
		yscale=x->ReadElementContentAsDouble();
	} else if (String::Compare(x->Name,"TILTANGLE")==0) {
		tiltangle=x->ReadElementContentAsDouble();
	} else if (String::Compare(x->Name,"MODE")==0) {
		cmode=(CoordMode) x->ReadElementContentAsInt();
		return 0x10;
	} else if (String::Compare(x->Name,"DRAWCOORD")==0) {
		drawcoord=x->ReadElementContentAsInt();
	} else if (String::Compare(x->Name,"DRAWMESH")==0) {
		drawmesh=x->ReadElementContentAsInt();
	} else if (String::Compare(x->Name,"DRAWREFPOINTS")==0) {
		drawrefpoints=x->ReadElementContentAsInt();
	} else if (String::Compare(x->Name,"XCROSS")==0) {
		xcross=x->ReadElementContentAsDouble();
	} else if (String::Compare(x->Name,"YCROSS")==0) {
		ycross=x->ReadElementContentAsDouble();
	} else if (String::Compare(x->Name,"XCROSSAUTO")==0) {
		xcrossauto=x->ReadElementContentAsInt();
	} else if (String::Compare(x->Name,"YCROSSAUTO")==0) {
		ycrossauto=x->ReadElementContentAsInt();
	} else if (String::Compare(x->Name,"UNIT")==0) {
		if (x->IsEmptyElement) { c->f->unit="";x->Read(); }
		else c->f->unit=x->ReadElementContentAsString();
	} else if (String::Compare(x->Name,"FORMAT")==0) {
		c->f->form=x->ReadElementContentAsString();
	} else if (String::Compare(x->Name,"CUNIT")==0) {
		if (x->IsEmptyElement) { c->f->unit="";x->Read(); }
		else c->f->cunit=x->ReadElementContentAsString();
	} else if (String::Compare(x->Name,"CFORMAT")==0) {
		c->f->cform=x->ReadElementContentAsString();
	} else return 0;
	return 1;
}

// iteration function
vectorN itF(vectorN N,vector a, vector A, vector b, vector B, vector c, vector C) {
	vectorN v;
	v.len=6;
	v[0]=N[0]*A.x-N[4]*N[1]*A.y+N[2]-a.x;
	v[1]=N[1]*A.x+N[4]*N[0]*A.y+N[3]-a.y;
	v[2]=N[0]*B.x-N[4]*N[1]*B.y+N[2]-b.x;
	v[3]=N[1]*B.x+N[4]*N[0]*B.y+N[3]-b.y;
	v[4]=N[0]*N[5]-N[4]*N[1]*C.y+N[2]-c.x;
	v[5]=N[1]*N[5]+N[4]*N[0]*C.y+N[3]-c.y;
	return v;
}

// Frechet Derivative
matrixN itFs(vectorN N,vector a, vector A, vector b, vector B, vector c, vector C) {
	matrixN m;m.len=6;

	m[0,5]=0;
	m[1,5]=0;
	m[2,5]=0;
	m[3,5]=0;
	m[4,5]=N[0];
	m[5,5]=N[1];

	m[0,4]=-N[1]*A.y;
	m[1,4]=N[0]*A.y;
	m[2,4]=-N[1]*B.y;
	m[3,4]=N[0]*B.y;
	m[4,4]=-N[1]*C.y;
	m[5,4]=N[0]*C.y;

	m[0,3]=0;
	m[1,3]=1;
	m[2,3]=0;
	m[3,3]=1;
	m[4,3]=0;
	m[5,3]=1;

	m[0,2]=1;
	m[1,2]=0;
	m[2,2]=1;
	m[3,2]=0;
	m[4,2]=1;
	m[5,2]=0;

	m[0,1]=-N[4]*A.y;
	m[1,1]=A.x;
	m[2,1]=-N[4]*B.y;
	m[3,1]=B.x;
	m[4,1]=-N[4]*C.y;
	m[5,1]=N[5];

	m[0,0]=A.x;
	m[1,0]=N[4]*A.y;
	m[2,0]=B.x;
	m[3,0]=N[4]*B.y;
	m[4,0]=N[5];
	m[5,0]=N[4]*C.y;

	return m;
}

/*
// Frechet Derivative
matrixN itFs(vectorN N, vector a, vector A, vector b, vector B, vector c, vector C) {
	matrixN m;
	vectorN dV;

	m.len=6;
	dV.len=6;

	for (int i=0;i<6;i++) {
		dV*=0; 
		dV[i]=1e-9;
		vectorN v=(itF(N+dV,a,A,b,B,c,C)-itF(N,a,A,b,B,c,C))/1e-9;
		for (int j=0;j<6;j++) m[j,i]=v[j];
	}
	return m;
}
*/
int coordinate::istwopoint(void) {
	if ((cmode == CoordMode::TwoPoint) || (cmode == CoordMode::TwoPointYScale) ||
		(cmode == CoordMode::TwoPointSimTilt))
		return 1;
	return 0;
}

void coordinate::calctrf(void) {

	if (istwopoint()) {
		// Two Point Cal
		matrixN Q; Q.len=4;
		Q[0,0]=p1->real.x;	Q[0,1]=p1->real.y;Q[0,2]=1;		Q[0,3]=0;
		Q[1,0]=-p1->real.y;	Q[1,1]=p1->real.x;	Q[1,2]=0;		Q[1,3]=1;
		Q[2,0]=p2->real.x;	Q[2,1]=p2->real.y;Q[2,2]=1;		Q[2,3]=0;
		Q[3,0]=-p2->real.y;	Q[3,1]=p2->real.x;	Q[3,2]=0;		Q[3,3]=1;

		/*Q[0,0]=p1->real.x;	Q[0,1]=-p1->real.y;Q[0,2]=1;		Q[0,3]=0;
		Q[1,0]=p1->real.y;	Q[1,1]=p1->real.x;	Q[1,2]=0;		Q[1,3]=1;
		Q[2,0]=p2->real.x;	Q[2,1]=-p2->real.y;Q[2,2]=1;		Q[2,3]=0;
		Q[3,0]=p2->real.y;	Q[3,1]=p2->real.x;	Q[3,2]=0;		Q[3,3]=1;*/
		vectorN a;a.len=4;
		a[0]=p1->data.x;
		a[1]=p1->data.y;
		a[2]=p2->data.x;
		a[3]=p2->data.y;

		matrixN QI=Q.inv();
		vectorN b=QI * a;
		M=matrix(b[0],-b[1],b[1],b[0]);
		C=vector(b[2],b[3]);

		switch (cmode) {
		case CoordMode::TwoPointYScale: 
			M.yx*=yscale;
			M.yy*=yscale;
			break;
		case CoordMode::TwoPointSimTilt:
			double sc=1/sqrt(1+tan(tiltangle*M_PI/180.0)*tan(tiltangle*M_PI/180.0));
			M.yx*=sc;
			M.yy*=sc;
			break;
		}

/*		if (force90) {
			vector R3=getreal(p3->data);
			double yscale=R3.y/p3->real.y;
			M.yx*=yscale;
			M.yy*=yscale;
		}*/

	} else {
		// Three point
//		if (force90) {
		if (cmode == CoordMode::ThreePointForce90) {
			// This is a nonlinear system --> iterative solution seemed to be the simplest until some mathematician solves the nonlinear system of 6 equations
			// First: Define a starting point so that the iteration is at least fast...
			matrixN Q; Q.len=4;
			Q[0,0]=p1->real.x;	Q[0,1]=p1->real.y;Q[0,2]=1;		Q[0,3]=0;
			Q[1,0]=-p1->real.y;	Q[1,1]=p1->real.x;	Q[1,2]=0;		Q[1,3]=1;
			Q[2,0]=p2->real.x;	Q[2,1]=p2->real.y;Q[2,2]=1;		Q[2,3]=0;
			Q[3,0]=-p2->real.y;	Q[3,1]=p2->real.x;	Q[3,2]=0;		Q[3,3]=1;

			vectorN a;a.len=4;
			a[0]=p1->data.x;
			a[1]=p1->data.y;
			a[2]=p2->data.x;
			a[3]=p2->data.y;

			matrixN QI=Q.inv();
			vectorN b=QI * a;

			// Starting point defined...
			vectorN X;X.len=6;
			X[0]=b[0];
			X[1]=b[1];
			X[2]=b[2];
			X[3]=b[3];
			X[4]=1;
			X[5]=0;
			// Go iterate 10 times - which seemed enough for now.
			for (int i=0;i<10;i++) {
				matrixN M=itFs(X,p1->data,p1->real,p2->data,p2->real,p3->data,p3->real);
				X-=M.inv()*itF(X,p1->data,p1->real,p2->data,p2->real,p3->data,p3->real);
			}
			M=matrix(X[0],-X[4]*X[1],X[1],X[4]*X[0]);
			C=vector(X[2],X[3]);
			p3->real.x=X[5];
		} else {
			// Classic Three Point
			matrixN Q; Q.len=6;
			Q[0,0]=p1->real.x;	Q[0,1]=p1->real.y;	Q[0,2]=0;		Q[0,3]=0;		Q[0,4]=1;	Q[0,5]=0;
			Q[1,0]=0;		Q[1,1]=0;		Q[1,2]=p1->real.x;	Q[1,3]=p1->real.y;	Q[1,4]=0;	Q[1,5]=1;
			Q[2,0]=p2->real.x;	Q[2,1]=p2->real.y;	Q[2,2]=0;		Q[2,3]=0;		Q[2,4]=1;	Q[2,5]=0;
			Q[3,0]=0;		Q[3,1]=0;		Q[3,2]=p2->real.x;	Q[3,3]=p2->real.y;	Q[3,4]=0;	Q[3,5]=1;
			Q[4,0]=p3->real.x;	Q[4,1]=p3->real.y;	Q[4,2]=0;		Q[4,3]=0;		Q[4,4]=1;	Q[4,5]=0;
			Q[5,0]=0;		Q[5,1]=0;		Q[5,2]=p3->real.x;	Q[5,3]=p3->real.y;	Q[5,4]=0;	Q[5,5]=1;
			vectorN a;a.len=6;
			a[0]=p1->data.x;
			a[1]=p1->data.y;
			a[2]=p2->data.x;
			a[3]=p2->data.y;
			a[4]=p3->data.x;
			a[5]=p3->data.y;

			matrixN QI=Q.inv();
			vectorN b=QI * a;
			M=matrix(b[0],b[1],b[2],b[3]);
			C=vector(b[4],b[5]);
		}
	}
}

/*
void coordinate::twopointmode(int m) {
	if (m) {
		if (twopoint) return; // nothing to do
		// simple: Just switch two-point mode on
		twopoint=1;
		calctrf();
	} else {
		if (!twopoint) return; // nothing to do
		// switch from two to three points --> calculate a third point that fits
		vector newd=p2->data-p1->data;
		vector vy=vector(newd.y,-newd.x);
		p3->data=p1->data+vy;
		p3->real=getreal(p3->data);
		twopoint=0;
		calctrf();
	}
}

void coordinate::force90mode(int m) {
	force90=m;
	if (twopoint) return;
	calctrf();
}
*/

void coordinate::changemode(CoordMode newmode) {
	if (newmode == cmode) return;
	switch (newmode) {
	case CoordMode::TwoPoint: 
	case CoordMode::TwoPointYScale: 
	case CoordMode::TwoPointSimTilt: 
		cmode=newmode;
		break;
	case CoordMode::ThreePoint: 
	case CoordMode::ThreePointForce90: 
		if ((cmode == CoordMode::TwoPoint) || (cmode == CoordMode::TwoPointYScale) ||
			(cmode == CoordMode::TwoPointSimTilt)) {
			vector newd=p2->data-p1->data;
			vector vy=vector(newd.y,-newd.x);
			p3->data=p1->data+vy;
			p3->real=getreal(p3->data);
		}
		cmode=newmode;
		break;
	}
	calctrf();
}

vector coordinate::getreal(vector data) {
	vector real=M.inv()*(data-C);
	if (istwopoint()) real.y*=-1;
	//if (twopoint) real.y*=-1;
	return real;
}

vector coordinate::getdata(vector real) {
	if (istwopoint()) real.y*=-1;
//	if (twopoint) real.y*=-1;
	vector data=M*real+C;
	return data;
}


String ^coordinate::getdesc(void) {
	if (name->Length > 0) 
		return gcnew String("CS(" + name + ")");
	else
		return gcnew String("Coordinate System");
}
	

void coordinate::draw(Graphics ^g) {
	Pen ^pen;
	Pen ^spen;

	if (selected)
		pen=gcnew Pen( Color::FromArgb(255-linecol.R,255-linecol.G,255-linecol.B),2.0f );
	else
		pen=gcnew Pen( linecol,1.0f );
	spen=gcnew Pen( linecol,1.0f );
	spen->DashStyle = System::Drawing::Drawing2D::DashStyle::DashDotDot;



	if (drawcoord) {
		SolidBrush ^fb=gcnew SolidBrush(fntcol);
		vector A,B,D,vx,vy;
		double lx,ly,step10x,step10y,cx,cy;

		if (drawrefpoints)	{
			// Draw points
			p1->draw(g);
			p2->draw(g);
			//if (twopoint == 0)
			if (!istwopoint()) 
				p3->draw(g);
		}



		if (xcrossauto && ycrossauto) {
			A=c->t->toscreen(getdata(vector(0,0)));
			B=c->t->toscreen(getdata(vector(0+1.0,0)));
			D=(istwopoint())?(c->t->toscreen(getdata(vector(0,0+1.0)))):(c->t->toscreen(getdata(vector(0,0+1.0))));
			vx=B-A;vy=D-A;
			lx=vx.len;ly=vy.len;
			step10x=find10step(lx,c->t->screen.len/2);
			step10y=find10step(ly,c->t->screen.len/2);
			vector centerpoint=getreal(c->t->todata(c->t->screen/2));
			cx=floor(centerpoint.x/step10x+0.5)*step10x;
			cy=floor(centerpoint.y/step10y+0.5)*step10y;
			A=c->t->toscreen(getdata(vector(cx,cy)));
			B=c->t->toscreen(getdata(vector(cx+1.0,cy)));
			D=(istwopoint())?(c->t->toscreen(getdata(vector(cx,cy+1.0)))):(c->t->toscreen(getdata(vector(cx,cy+1.0))));
		} else if (xcrossauto && !ycrossauto) {
			A=c->t->toscreen(getdata(vector(0,0)));
			B=c->t->toscreen(getdata(vector(0+1.0,0)));
			D=(istwopoint())?(c->t->toscreen(getdata(vector(0,0+1.0)))):(c->t->toscreen(getdata(vector(0,0+1.0))));
			vx=B-A;vy=D-A;
			lx=vx.len;ly=vy.len;
			step10x=find10step(lx,c->t->screen.len/2);
			step10y=find10step(ly,c->t->screen.len/2);
			vector centerpoint=getreal(c->t->todata(c->t->screen/2));
			cx=floor(centerpoint.x/step10x+0.5)*step10x;
			//cy=floor(centerpoint.y/step10y+0.5)*step10y;
			cy=ycross;
			A=c->t->toscreen(getdata(vector(cx,cy)));
			B=c->t->toscreen(getdata(vector(cx+1.0,cy)));
			D=(istwopoint())?(c->t->toscreen(getdata(vector(cx,cy+1.0)))):(c->t->toscreen(getdata(vector(cx,cy+1.0))));
		} else if (!xcrossauto && ycrossauto) {
			A=c->t->toscreen(getdata(vector(0,0)));
			B=c->t->toscreen(getdata(vector(0+1.0,0)));
			D=(istwopoint())?(c->t->toscreen(getdata(vector(0,0+1.0)))):(c->t->toscreen(getdata(vector(0,0+1.0))));
			vx=B-A;vy=D-A;
			lx=vx.len;ly=vy.len;
			step10x=find10step(lx,c->t->screen.len/2);
			step10y=find10step(ly,c->t->screen.len/2);
			vector centerpoint=getreal(c->t->todata(c->t->screen/2));
			cx=floor(centerpoint.x/step10x+0.5)*step10x;
			cy=floor(centerpoint.y/step10y+0.5)*step10y;
			//vector CP=getreal(vector(xcross,cy));
			cx=xcross;
			//cy=CP.y;
			A=c->t->toscreen(getdata(vector(cx,cy)));
			B=c->t->toscreen(getdata(vector(cx+1.0,cy)));
			D=(istwopoint())?(c->t->toscreen(getdata(vector(cx,cy+1.0)))):(c->t->toscreen(getdata(vector(cx,cy+1.0))));
		} else {
			A=c->t->toscreen(getdata(vector(xcross,ycross)));
			B=c->t->toscreen(getdata(vector(xcross+1.0,ycross)));
			D=(istwopoint())?(c->t->toscreen(getdata(vector(xcross,ycross+1.0)))):(c->t->toscreen(getdata(vector(xcross,ycross+1.0))));
			vx=B-A;
			vy=D-A;
			lx=vx.len;
			ly=vy.len;
			step10x=find10step(lx,c->t->screen.len/2);
			step10y=find10step(ly,c->t->screen.len/2);
			cx=0;cy=0;
		}
		double tickxlen=5/ly;
		double tickylen=5/lx;
		double t1,t2;
		drawinfline(g,A,B,pen);
		drawinfline(g,A,D,pen);

		if (findsepoints(g,A,B,&t1,&t2)) {
			double sp=floor(t1/step10x);
			double ep=ceil(t2/step10x);
			for (int i=sp;i<=ep;i++) {
				if (i != 0) {
					double tickpos=(double)i*step10x;
					if (drawmesh) {
						drawinfline(g,A+vx*tickpos-vy,A+vx*tickpos+vy,pen);
//						drawtxt(g,fnt,fb,A+vx*tickpos-vy*tickxlen,vy,1,1,c->f->format(tickpos+cx));
						drawtxt(g,A+vx*tickpos-vy*tickxlen,vy,1,1,c->f->format(tickpos+cx));
					} else {
						drawline(g,A+vx*tickpos-vy*tickxlen,A+vx*tickpos+vy*tickxlen,pen);
//						drawtxt(g,fnt,fb,A+vx*tickpos-vy*tickxlen,vy,1,0,c->f->format(tickpos+cx));
						drawtxt(g,A+vx*tickpos-vy*tickxlen,vy,1,0,c->f->format(tickpos+cx));
					}
				}
				for (int j=1;j<10;j++) {
					double tickpos=(double)(i+0.1*j)*step10x;
					double ticklen=0.3;
					if (j==5) ticklen=0.6;
					if (drawmesh) {
						drawinfline(g,A+vx*tickpos-vy,A+vx*tickpos+vy,spen);
					} else {
						drawline(g,A+vx*tickpos-vy*tickxlen*ticklen,A+vx*tickpos+vy*tickxlen*ticklen,pen);
					}
				}
			}
		}
		if (findsepoints(g,A,D,&t1,&t2)) {
			double sp=floor(t1/step10x);
			double ep=ceil(t2/step10x);
			for (int i=sp;i<=ep;i++) {
				if (i != 0) {
					double tickpos=(double)i*step10y;
					if (drawmesh) {
						drawinfline(g,A+vy*tickpos-vx,A+vy*tickpos+vx,pen);
//						drawtxt(g,fnt,fb,A+vy*tickpos-vx*tickxlen,vx,1,1,c->f->format(tickpos+cy));
						drawtxt(g,A+vy*tickpos-vx*tickxlen,vx,1,1,c->f->format(tickpos+cy));
					} else {
						drawline(g,A+vy*tickpos-vx*tickylen,A+vy*tickpos+vx*tickylen,pen);
//						drawtxt(g,fnt,fb,A+vy*tickpos-vx*tickxlen,vx,1,0,c->f->format(tickpos+cy));
						drawtxt(g,A+vy*tickpos-vx*tickxlen,vx,1,0,c->f->format(tickpos+cy));
					}
				}
				for (int j=1;j<10;j++) {
					double tickpos=(double)(i+0.1*j)*step10y;
					double ticklen=0.3;
					if (j==5) ticklen=0.6;
					if (drawmesh) {
						drawinfline(g,A+vy*tickpos-vx,A+vy*tickpos+vx,spen);
					} else {
						drawline(g,A+vy*tickpos-vx*tickylen*ticklen,A+vy*tickpos+vx*tickylen*ticklen,pen);
					}
				}
			}
		}
		delete fb;
	}
	delete pen;
	delete spen;

}

List<cmeasurement^> ^coordinate::touching(vector datai) {
	List<cmeasurement^> ^l=gcnew List<cmeasurement^>();
	List<cmeasurement^> ^lin;

	if (!drawcoord) return nullptr;

	if (drawrefpoints) {
		if (lin=p1->touching(datai)) { l->AddRange(lin); };
		if (lin=p2->touching(datai)) { l->AddRange(lin); };
		//if (!twopoint)
		if (!istwopoint()) 
			if (lin=p3->touching(datai)) { l->AddRange(lin); };
	}
	if (l->Count < 1) {
		// Check for Line Hit
		vector A,B,D,vx,vy;
		double lx,ly,step10x,step10y,cx,cy;
		if (xcrossauto && ycrossauto) {
			A=c->t->toscreen(getdata(vector(0,0)));
			B=c->t->toscreen(getdata(vector(0+1.0,0)));
			D=(istwopoint())?(c->t->toscreen(getdata(vector(0,0+1.0)))):(c->t->toscreen(getdata(vector(0,0+1.0))));
			vx=B-A;vy=D-A;
			lx=vx.len;ly=vy.len;
			step10x=find10step(lx,c->t->screen.len/2);
			step10y=find10step(ly,c->t->screen.len/2);
			vector centerpoint=getreal(c->t->todata(c->t->screen/2));
			cx=floor(centerpoint.x/step10x+0.5)*step10x;
			cy=floor(centerpoint.y/step10y+0.5)*step10y;
			A=c->t->toscreen(getdata(vector(cx,cy)));
			B=c->t->toscreen(getdata(vector(cx+1.0,cy)));
			D=(istwopoint())?(c->t->toscreen(getdata(vector(cx,cy+1.0)))):(c->t->toscreen(getdata(vector(cx,cy+1.0))));
		} else if (xcrossauto && !ycrossauto) {
			A=c->t->toscreen(getdata(vector(0,0)));
			B=c->t->toscreen(getdata(vector(0+1.0,0)));
			D=(istwopoint())?(c->t->toscreen(getdata(vector(0,0+1.0)))):(c->t->toscreen(getdata(vector(0,0+1.0))));
			vx=B-A;vy=D-A;
			lx=vx.len;ly=vy.len;
			step10x=find10step(lx,c->t->screen.len/2);
			step10y=find10step(ly,c->t->screen.len/2);
			vector centerpoint=getreal(c->t->todata(c->t->screen/2));
			cx=floor(centerpoint.x/step10x+0.5)*step10x;
			//cy=floor(centerpoint.y/step10y+0.5)*step10y;
			cy=ycross;
			A=c->t->toscreen(getdata(vector(cx,cy)));
			B=c->t->toscreen(getdata(vector(cx+1.0,cy)));
			D=(istwopoint())?(c->t->toscreen(getdata(vector(cx,cy+1.0)))):(c->t->toscreen(getdata(vector(cx,cy+1.0))));
		} else if (!xcrossauto && ycrossauto) {
			A=c->t->toscreen(getdata(vector(0,0)));
			B=c->t->toscreen(getdata(vector(0+1.0,0)));
			D=(istwopoint())?(c->t->toscreen(getdata(vector(0,0+1.0)))):(c->t->toscreen(getdata(vector(0,0+1.0))));
			vx=B-A;vy=D-A;
			lx=vx.len;ly=vy.len;
			step10x=find10step(lx,c->t->screen.len/2);
			step10y=find10step(ly,c->t->screen.len/2);
			vector centerpoint=getreal(c->t->todata(c->t->screen/2));
			cx=floor(centerpoint.x/step10x+0.5)*step10x;
			cy=floor(centerpoint.y/step10y+0.5)*step10y;
			//vector CP=getreal(vector(xcross,cy));
			cx=xcross;
			//cy=CP.y;
			A=c->t->toscreen(getdata(vector(cx,cy)));
			B=c->t->toscreen(getdata(vector(cx+1.0,cy)));
			D=(istwopoint())?(c->t->toscreen(getdata(vector(cx,cy+1.0)))):(c->t->toscreen(getdata(vector(cx,cy+1.0))));
		} else {
			A=c->t->toscreen(getdata(vector(xcross,ycross)));
			B=c->t->toscreen(getdata(vector(xcross+1.0,ycross)));
			D=(istwopoint())?(c->t->toscreen(getdata(vector(xcross,ycross+1.0)))):(c->t->toscreen(getdata(vector(xcross,ycross+1.0))));
			vx=B-A;
			vy=D-A;
			lx=vx.len;
			ly=vy.len;
			step10x=find10step(lx,c->t->screen.len/2);
			step10y=find10step(ly,c->t->screen.len/2);
			cx=0;cy=0;
		}
		double ldx=fabs(getlinedist(A,B,c->t->toscreen(datai)));
		double ldy=fabs(getlinedist(A,D,c->t->toscreen(datai)));
		if ((ldx < 3) || (ldy < 3)) {
			l->Add(this);
			return l;
		}
		return nullptr;
	}
	return l;
}

List<cmeasurement^> ^coordinate::isinside(vector data1, vector data2) {
	List<cmeasurement^> ^l=gcnew List<cmeasurement^>();
	List<cmeasurement^> ^lin;

	if (!drawcoord) return nullptr;

	if (drawrefpoints) {
		if (lin=p1->isinside(data1,data2)) { l->AddRange(lin); };
		if (lin=p2->isinside(data1,data2)) { l->AddRange(lin); };
		// if (!twopoint)
		if (!istwopoint()) 
			if (lin=p3->isinside(data1,data2)) { l->AddRange(lin); }
	}
	if (l->Count < 1) 
		return nullptr;
	return l;
}

void coordinate::moveto(vector data_from, vector data_to) {
	if (selected) {
		p1->moveto(data_from,data_to);
		p2->moveto(data_from,data_to);
		p3->moveto(data_from,data_to);
	} else {
		if (p1->selected) p1->moveto(data_from,data_to);
		if (p2->selected) p2->moveto(data_from,data_to);
		if (p3->selected) p3->moveto(data_from,data_to);
	}
}

void coordinate::storestate() {
	p1->storestate();
	p2->storestate();
	p3->storestate();
}

void coordinate::restorestate() {
	p1->restorestate();
	p2->restorestate();
	p3->restorestate();
	calctrf();
}

void coordinate::select() {
	selected=1;
	p1->unselect();
	p2->unselect();
	p3->unselect();
}

void coordinate::unselect() {
	selected=0;
	p1->unselect();
	p2->unselect();
	p3->unselect();
}

void coordinate::setUnit(String ^s) {
	c->f->unit=gcnew String(s);
}

String^ coordinate::getUnit(void) {
	return c->f->unit;
}

void coordinate::setFormat(String ^s) {
	c->f->form=gcnew String(s);
}

String^ coordinate::getFormat(void) {
	return c->f->form;
}

void coordinate::setCUnit(String ^s) {
	c->f->cunit=gcnew String(s);
}

String^ coordinate::getCUnit(void) {
	return c->f->cunit;
}

void coordinate::setCFormat(String ^s) {
	c->f->cform=gcnew String(s);
}

String^ coordinate::getCFormat(void) {
	return c->f->cform;
}


