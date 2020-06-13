#include "StdAfx.h"
#include "ia_xml_io.h"
#include <math.h>
#include "vectormatrix.h"

using namespace image_add;

ia_xml_read::ia_xml_read(String ^fn):System::Xml::XmlTextReader(fn)
{
	WhitespaceHandling=System::Xml::WhitespaceHandling::None;
}

Font ^ia_xml_read::xml_read_font() {
	Font ^f;
	String ^fname;
	float size;
	int flags;
	int ffound,sfound,tfound;
	ffound=sfound=tfound=0;
	while (1) {
		switch (NodeType) {
			case XmlNodeType::Element:
				if (String::Compare(Name,"FontFamily")==0) {
					if (IsEmptyElement) { fname="";Read(); }
					else fname=ReadElementContentAsString();
					ffound=1;
				} else if (String::Compare(Name,"Size")==0) {
					size=(float)ReadElementContentAsDouble();
					sfound=1;
				} else if (String::Compare(Name,"Style")==0) {
					flags=ReadElementContentAsInt();
					tfound=1;
				} else {
					// Unknown element --> Skip over
					if (IsEmptyElement)	Read(); else Skip();
				}
				break;
			case XmlNodeType::EndElement:
				if (!ffound || !sfound || !tfound) 
					throw gcnew System::Exception("Bad XML Formatted File <FONT> tag");
				Read();
				FontStyle fs;
				fs=FontStyle::Regular;
				if (flags & 0x01) fs = fs | FontStyle::Bold;
				if (flags & 0x02) fs = fs | FontStyle::Italic;
				if (flags & 0x04) fs = fs | FontStyle::Strikeout;
				if (flags & 0x08) fs = fs | FontStyle::Underline;
				f=gcnew Font(fname,size,fs);
				return f;
			default:
				throw gcnew System::Exception("Bad XML Formatted File <FONT> tag");
		}
	}
}

vector ia_xml_read::xml_read_vector() {
	vector v;
	int xfound,yfound;
	xfound=yfound=0;
	while (1) {
		switch (NodeType) {
			case XmlNodeType::Element:
				if (String::Compare(Name,"X")==0) {
					v.x=ReadElementContentAsDouble();
					xfound=1;
				} else if (String::Compare(Name,"Y")==0) {
					v.y=ReadElementContentAsDouble();
					yfound=1;
				} else {
					// Unknown element --> Skip over
					if (IsEmptyElement)	Read(); else Skip();
				}
				break;
			case XmlNodeType::EndElement:
				if (!xfound || !yfound) 
					throw gcnew System::Exception("Bad XML Formatted File <VECTOR> tag");
				Read();
				return v;
			default:
				throw gcnew System::Exception("Bad XML Formatted File <VECTOR> tag");
		}
	}
}

Color ia_xml_read::xml_read_color() {
	int cr,cg,cb;
	int rfound,gfound,bfound;
	rfound=gfound=bfound=0;
	while (1) {
		switch (NodeType) {
			case XmlNodeType::Whitespace:
				Read();
				break;
			case XmlNodeType::Element:
				if (String::Compare(Name,"R")==0) {
					cr=ReadElementContentAsInt();
					rfound=1;
				} else if (String::Compare(Name,"G")==0) {
					cg=ReadElementContentAsInt();
					gfound=1;
				} else if (String::Compare(Name,"B")==0) {
					cb=ReadElementContentAsInt();
					bfound=1;
				} else {
					// Unknown element --> Skip over
					if (IsEmptyElement)	Read(); else Skip();
				}
				break;
			case XmlNodeType::EndElement:
				if (!rfound || !gfound || !bfound) 
					throw gcnew System::Exception("Bad XML Formatted File <COLOR> tag");
				Read();
				return Color::FromArgb(cr,cg,cb);
			default:
				throw gcnew System::Exception("Bad XML Formatted File <COLOR> tag");
		}
	}
}

void ia_xml_read::read_xml_dimension(int *w, int *h) {
	int foundw, foundh;
	foundw=foundh=0;
	while (1) {
		switch (NodeType) {
			case XmlNodeType::Element:
				if (String::Compare(Name,"W")==0) {
					*w=ReadElementContentAsInt();
					foundw=1;
				} else if (String::Compare(Name,"H")==0) {
					*h=ReadElementContentAsInt();
					foundh=1;
				} else {
					if (IsEmptyElement)	Read(); else Skip();
				}
				break;
			case XmlNodeType::EndElement:
				if (!foundw || !foundh) 
					throw gcnew System::Exception("Bad XML Formatted File <DIMENSION> Tag");
				Read();
				return;
			default: 
				throw gcnew System::Exception("Bad XML Formatted File <DIMENSION> Tag");
		}
	}
}




ia_xml_write::ia_xml_write(String ^fn):System::Xml::XmlTextWriter(fn,nullptr)
{
}

void ia_xml_write::write_value(String ^name,double d) {
	WriteStartElement(name);
		WriteValue(d);
	WriteEndElement();
}

void ia_xml_write::write_value(String ^name,int d) {
	WriteStartElement(name);
		WriteValue(d);
	WriteEndElement();
}

void ia_xml_write::write_value(String ^name,String ^v) {
	WriteStartElement(name);
		WriteValue(v);
	WriteEndElement();
}

void ia_xml_write::write_value(String ^name, vector v) {
	WriteStartElement(name);
		write_value("X",v.x);
		write_value("Y",v.y);
	WriteEndElement();
}

void ia_xml_write::write_dimension(String ^name, int w, int h) {
	WriteStartElement(name);
		write_value("W",w);
		write_value("H",h);
	WriteEndElement();
}

void ia_xml_write::write_value(String ^name,Color c) {
	WriteStartElement(name);
		write_value("R",c.R);
		write_value("G",c.G);
		write_value("B",c.B);
	WriteEndElement();
}

void ia_xml_write::write_value(String ^name,Font ^f) {
	int flags=0;
	WriteStartElement(name);
		write_value("FontFamily",f->FontFamily->GetName(0));
		write_value("Size",(double) f->SizeInPoints);
		flags+=(f->Bold)?0x01:0x00;
		flags+=(f->Italic)?0x02:0x00;
		flags+=(f->Strikeout)?0x04:0x00;
		flags+=(f->Underline)?0x08:0x00;
		write_value("Style",flags);
	WriteEndElement();
}

