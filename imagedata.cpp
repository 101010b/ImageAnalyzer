#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <math.h>
// #include "image_add_win.h"
#include "imagedata.h"
#include <jpeglib.h>
#include <zlib.h>
// typedef System::Byte Byte;
//#include <setjmp.h>
//#include <exception>

using namespace image_add;

// #pragma comment(lib,"zlibstat.lib")
// #pragma comment(lib,"jpeglib.lib")

imagedata::imagedata(void) {
	width=0;
	height=0;
	filename=nullptr;
	data=nullptr;
	bmap=nullptr;
}

imagedata::imagedata(int i) {
	width=0;
	height=0;
	filename=nullptr;
	data=nullptr;
	bmap=nullptr;

	if (!Clipboard::ContainsImage()) return;
	
	bmap=gcnew Bitmap(Clipboard::GetImage());
	filename="ClipBoard";

	width=bmap->Width;
	height=bmap->Height;
	data=gcnew array<System::Byte>(width*height*3);

	BitmapData^ rawdata=bmap->LockBits(Drawing::Rectangle(0,0,width,height),
		System::Drawing::Imaging::ImageLockMode::ReadWrite,
		System::Drawing::Imaging::PixelFormat::Format24bppRgb);
	int tempsw=rawdata->Stride;
	array<System::Byte>^ tempd=gcnew array<System::Byte>(tempsw*height);
	System::Runtime::InteropServices::Marshal::Copy(rawdata->Scan0, tempd,0, tempsw*height );
	for (int x=0;x<width;x++) {
		for (int y=0;y<height;y++) {
			data[y*3*width+x*3+0]=tempd[tempsw*y+x*3+2];
			data[y*3*width+x*3+1]=tempd[tempsw*y+x*3+1];
			data[y*3*width+x*3+2]=tempd[tempsw*y+x*3+0];
		}
	}
	bmap->UnlockBits( rawdata );
	delete tempd;

}

imagedata::imagedata(String ^fn) {
	width=0;
	height=0;
	filename=nullptr;
	data=nullptr;
	bmap=nullptr;

	try {
		int c=fn->Length-1;
		while ((c > 0) && (fn[c] != '.')) c--; c++;
		if ((String::Compare(fn,c,gcnew String("JPG"),0,3,true)==0) ||
			(String::Compare(fn,c,gcnew String("JPEG"),0,4,true)==0)) {
			// use JPG Library
			read_from_jpeg(fn);
		} else {
			// try windows functions
			read_using_windows(fn);
		}
		filename=gcnew String(fn);
	} 
	catch (System::Exception ^e) {
		delete data;
		delete bmap;
		delete filename;
		width=0;
		height=0;
	}
}

imagedata::~imagedata() {
	delete filename;
	delete data;
	delete bmap;
	width=0;
	height=0;
}

void jpg_compress_error(j_common_ptr cinfo) {
	throw gcnew System::Exception("Problem in JPEG compressor");
}

//extern "C" int jpeg_mem_size(j_compress_ptr cinfo);

array<System::Byte> ^imagedata::compressJPG() {
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;	
	int i;
	JSAMPROW buffer[1];
	unsigned char * linebuffer;
	unsigned char * membuf;
	unsigned long memsize;
	int jpegsize;
	array<System::Byte> ^a;

	memsize=0;
	membuf=NULL;

	jerr.error_exit=jpg_compress_error;
	cinfo.err=jpeg_std_error(&jerr);
	try {
		jpeg_create_compress(&cinfo);
		cinfo.image_width = width;
		cinfo.image_height = height;
		cinfo.input_components = 3;		/* # of color components per pixel */
		cinfo.in_color_space = JCS_RGB; /* colorspace of input image */
		jpeg_mem_dest(&cinfo,&membuf, &memsize);
		jpeg_set_defaults(&cinfo);
		jpeg_set_quality(&cinfo, 95 /* quality */ , 1 /* limit to baseline-JPEG values */);
		jpeg_start_compress(&cinfo, 1);

		linebuffer=(unsigned char *)malloc(width*3);
		while (cinfo.next_scanline < cinfo.image_height) {
			for (i=0;i<width;i++) {
				linebuffer[3*i+0]=data[cinfo.next_scanline*3*width+i*3+0];
				linebuffer[3*i+1]=data[cinfo.next_scanline*3*width+i*3+1];
				linebuffer[3*i+2]=data[cinfo.next_scanline*3*width+i*3+2];
			}
			buffer[0]=linebuffer;
			jpeg_write_scanlines(&cinfo, buffer, 1);
		}
		free(linebuffer);

		jpeg_finish_compress(&cinfo);
		//jpegsize = jpeg_mem_size(&cinfo);
		a=gcnew array<System::Byte>(memsize);
		for (i=0;i<memsize;i++) a[i]=membuf[i];
		jpeg_destroy_compress(&cinfo);
		return a;
	}
	catch (System::Exception ^e) {
		return nullptr;
	}
}


array<System::Byte> ^imagedata::compressZ() {
	array<System::Byte> ^compressed=gcnew array<System::Byte>(width*height*3+10000);
	array<System::Byte> ^final;
#define BS 4096
	unsigned char *compin, *compout;
	int written;
	int towrite;
	z_stream z;
	int compsize=0;

	z.zalloc = Z_NULL;
    z.zfree = Z_NULL;
    z.opaque = Z_NULL;

	if (deflateInit(&z,9) != Z_OK) return nullptr; 

	compin=(unsigned char*)malloc(BS);
	compout=(unsigned char*)malloc(BS);
	if (!compin || !compout) return nullptr;

	written=0;
	towrite=width*height*3;

	while (towrite) {
		int wrlen=0;
		int flush;
		while (towrite && wrlen < BS) {
			compin[wrlen]=data[written];
			wrlen++;written++;towrite--;
		}
		flush=(towrite==0)?Z_FINISH:Z_NO_FLUSH;
		z.avail_in=wrlen;
		z.next_in=compin;
		do {
			int have;
			z.avail_out=BS;
			z.next_out=compout;
			if (deflate(&z,flush) == Z_STREAM_ERROR) {
				free(compin);free(compout);
				return nullptr;
			}
			have=BS-z.avail_out;
			for (int i=0;i<have;i++) 
				compressed[compsize++]=compout[i];
		} while (z.avail_out == 0);
	} 
	deflateEnd(&z);
	free(compin);
	free(compout);

	final=gcnew array<System::Byte>(compsize);
	for (int i=0;i<compsize;i++) final[i]=compressed[i];
	delete compressed;

	return final;
}

void jpg_decompress_error(j_common_ptr cinfo) {
	throw gcnew System::Exception("Problem in JPEG decompressor");
}

int imagedata::decompressJPG(array<System::Byte> ^src) {
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;	
	int i;
	JSAMPARRAY buffer;
	unsigned char *a;
	int sl;
	int x,y;

	cinfo.err = jpeg_std_error(&jerr);
	jerr.error_exit = jpg_decompress_error;
	try {
		a=(unsigned char *) malloc(src->Length);
		if (!a) throw gcnew System::Exception("Out of Memory");
		for (i=0;i<src->Length;i++) a[i]=src[i];

		jpeg_create_decompress(&cinfo);
		jpeg_mem_src(&cinfo, a, src->Length);
	
		jpeg_read_header(&cinfo, TRUE);
		jpeg_start_decompress(&cinfo);

		if ((cinfo.output_width != width) || (cinfo.output_height != height)) 
			throw gcnew System::Exception("Image dimension Error");

		// data=gcnew array<System::Byte>(width*height*3);

		buffer = (*cinfo.mem->alloc_sarray)
			((j_common_ptr) &cinfo, JPOOL_IMAGE, 
			cinfo.output_width*cinfo.output_components, 1);

		sl=0;
		while (cinfo.output_scanline < cinfo.output_height) {
			jpeg_read_scanlines(&cinfo, buffer, 1);
			for (i=0;i<3*width;i++) data[width*3*sl+i]=buffer[0][i];
			//System::Runtime::InteropServices::Marshal::Copy(data[width*3*sl],0,
			//memcpy(&data[width*3*sl],buffer[0],3*width);
			sl++;
		}

		jpeg_finish_decompress(&cinfo);
		jpeg_destroy_decompress(&cinfo);
	}
	catch (System::Exception ^e) { 
		width=0;
		height=0;
		delete data;
		throw gcnew System::Exception("Decompression Error");
	}
	return 1;

	// Success
}

int imagedata::decompressZ(array<System::Byte> ^src) {

#define BS 4096
	unsigned char *compin, *compout;
	z_stream z;
	int toread=src->Length;
	int read=0;
	int written=0;
	int maxsize=width*height*3;
	int ret=0;

	z.zalloc=Z_NULL;
	z.zfree=Z_NULL;
	z.opaque=Z_NULL;
	z.avail_in=0;
	z.next_in=Z_NULL;
	
	if (inflateInit(&z) != Z_OK)
		return 0;

	compin=(unsigned char*)malloc(BS);
	compout=(unsigned char*)malloc(BS);

	do {
		int rs=0;
		if (toread > BS) {
			for (int i=0;i<BS;i++) {
				compin[i]=src[read];
				read++;toread--;
			} 
			rs=BS;
		} else {
			rs=0;
			while (toread) {
				compin[rs]=src[read];
				rs++;read++;toread--;
			}
		}
		z.avail_in=rs;
		if (z.avail_in == 0) break;
		z.next_in=compin;
		
		do {
			z.avail_out=BS;
			z.next_out=compout;
			ret=inflate(&z,Z_NO_FLUSH);
			if ((ret == Z_STREAM_ERROR) ||
				(ret == Z_NEED_DICT) || 
				(ret == Z_DATA_ERROR) ||
				(ret == Z_MEM_ERROR)) {
				free(compin);free(compout);
				return 0;
			}
			int have=BS-z.avail_out;
			for (int i=0;i<have;i++) {
				data[written]=compout[i];
				written++;
			}
		} while (z.avail_out == 0);
	} while (ret != Z_STREAM_END);
	return 1;
}

void imagedata::write_to_xml(XmlTextWriter ^x) {
	// array<unsigned char>^ buffer = gcnew array<unsigned char>(4096);
	int size=width*height*3;
	int towrite;
	int written=0;
	int block=0;
	array<System::Byte>^ compressed;
	compressed=compressJPG();
	towrite=compressed->Length;
	x->WriteStartElement("IMAGEW");x->WriteValue(width);x->WriteEndElement();
	x->WriteStartElement("IMAGEH");x->WriteValue(height);x->WriteEndElement();
	x->WriteStartElement("COMPRESSION");x->WriteValue("JPG");x->WriteEndElement();
	x->WriteStartElement("RAWSIZE");x->WriteValue(size);x->WriteEndElement();
	x->WriteStartElement("COMPRESSED");x->WriteValue(towrite);x->WriteEndElement();
	x->WriteWhitespace("\n");
	x->WriteStartElement("DATA");//x->WriteWhitespace("\n");
		while (towrite > 0) {
			if (towrite > 2*4096) {
				x->WriteStartElement("CHUNK");
					x->WriteBase64((array<unsigned char>^)compressed,written,4096);
				x->WriteEndElement();x->WriteWhitespace("\n");
				written+=4096;
				towrite-=4096;
			} else {
				if (towrite <= 4096) {
					x->WriteStartElement("CHUNK");
						x->WriteBase64((array<unsigned char>^)compressed,written,towrite);
					x->WriteEndElement();x->WriteWhitespace("\n");
					written+=towrite;
					towrite=0;
				} else {
					int wr=towrite/2;
					x->WriteStartElement("CHUNK");
						x->WriteBase64((array<unsigned char>^)compressed,written,wr);
					x->WriteEndElement();x->WriteWhitespace("\n");
					written+=wr;
					towrite-=wr;
				}
			}
		}
	x->WriteEndElement();x->WriteWhitespace("\n");
}

void imagedata::read_from_xml(XmlTextReader ^x) {
	int cs,rs;
	int wfound,hfound,csfound,dfound;
	wfound=hfound=csfound=dfound=0;
	String ^comp=gcnew String("Z");

	while (1) {
		switch (x->NodeType) {
			case XmlNodeType::Element:
				if (String::Compare(x->Name,"IMAGEW")==0) {
					width=x->ReadElementContentAsInt();
					wfound=1;
				} else if (String::Compare(x->Name,"IMAGEH")==0) {
					height=x->ReadElementContentAsInt();
					hfound=1;
				} else if (String::Compare(x->Name,"COMPRESSION")==0) {
					if (x->IsEmptyElement) { comp="";x->Read(); }
					else comp=x->ReadElementContentAsString();
				} else if (String::Compare(x->Name,"RAWSIZE")==0) {
					rs=x->ReadElementContentAsInt();
				} else if (String::Compare(x->Name,"COMPRESSED")==0) {
					cs=x->ReadElementContentAsInt();
					csfound=1;
				} else if (String::Compare(x->Name,"DATA")==0) {
					if (!wfound || !hfound || !csfound) 
						throw gcnew System::Exception("Bad XML Formatted File <DATA> tag");
					array<System::Byte> ^compressed=gcnew array<System::Byte>(cs);
					array<System::Byte> ^buffer=gcnew array<System::Byte>(4096);
					int toread=cs;
					int read=0;
					int finished=0;
					int bs=0;

					x->Read();

					while (!finished) {
						switch(x->NodeType) {
							case XmlNodeType::Element:
								if (String::Compare(x->Name,"CHUNK")==0) {
									x->Read();
									bs=x->ReadContentAsBase64(buffer,0,4096);
									for (int i=0;i<bs;i++) {
										if (toread <= 0) 
											throw gcnew System::Exception("Buffer Overflow in XML Reader");
										compressed[read]=buffer[i];
										read++;toread--;
									}
									switch (x->NodeType) {
									case XmlNodeType::Text:
										// some leftovers
										x->ReadContentAsString(); // this also reads the end element
										break;
									case XmlNodeType::EndElement:
										x->Read();
										break;
									}
									// x->Read();
								} else {
									// Unknown element --> Skip over
									if (x->IsEmptyElement)	x->Read(); else x->Skip();								}
								break;
							case XmlNodeType::EndElement:
								finished=1;
								x->Read();
								break;
							default:
								throw gcnew System::Exception("Bad XML Formatted File");
						}
					}
					if (toread > 0) 
						throw gcnew System::Exception("Bad XML Formatted File");

					data=gcnew array<System::Byte>(width*height*3);
					// decompress it
					if (comp == "Z") {
						if (!decompressZ(compressed)) 
							throw gcnew System::Exception("Bad XML Formatted File");
					} else if (comp == "JPG") {
						if (!decompressJPG(compressed)) 
							throw gcnew System::Exception("Bad XML Formatted File");
					}

					// Copy to Bitmap
					bmap=gcnew Bitmap(width,height);
					BitmapData^ rawdata=bmap->LockBits(Drawing::Rectangle(0,0,width,height),
						System::Drawing::Imaging::ImageLockMode::ReadWrite,
						System::Drawing::Imaging::PixelFormat::Format24bppRgb);
					int tempsw=rawdata->Stride;
					array<System::Byte>^ tempd=gcnew array<System::Byte>(tempsw*height);
					for (int x=0;x<width;x++) {
						for (int y=0;y<height;y++) {
							tempd[tempsw*y+x*3+2]=data[y*3*width+x*3+0];
							tempd[tempsw*y+x*3+1]=data[y*3*width+x*3+1];
							tempd[tempsw*y+x*3+0]=data[y*3*width+x*3+2];
						}
					}
					System::Runtime::InteropServices::Marshal::Copy(tempd,0,rawdata->Scan0, tempsw*height );
					bmap->UnlockBits( rawdata );
					delete tempd;

					dfound=1;
				} else {
					// Unknown element --> Skip over
					if (x->IsEmptyElement)	x->Read(); else x->Skip();
				}
				break;
			case XmlNodeType::EndElement:
				if (!dfound) 
					throw gcnew System::Exception("Bad XML Formatted File <IMAGE> tag");
				x->Read();
				return;
			default:
				throw gcnew System::Exception("Bad XML Formatted File");
		}
	}
}

//struct my_error_mgr {
//  struct jpeg_error_mgr pub;	/* "public" fields */
//  jmp_buf setjmp_buffer;	/* for return to caller */
//};

//typedef struct my_error_mgr * my_error_ptr;


METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
	throw gcnew System::Exception("Problem in JPEG reader");
//  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
//  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
 // (*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
//  longjmp(myerr->setjmp_buffer, 1);
}

void imagedata::read_from_jpeg(String ^fn) {
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;	
	int i;
	FILE *infile;
	JSAMPARRAY buffer;
	int sl;
	int x,y;
	char *fns;

	fns=(char *)malloc(fn->Length+1);
	for (i=0;i<fn->Length;i++) fns[i]=fn[i];
	fns[i]=0;

	infile = fopen(fns, "rb");
	free(fns);
	if (infile == NULL) {
		// File open failed
		throw gcnew System::Exception("File Read Error of File" + fn);
	}
	//free(fns);

	// JPEG CODE STARTS HERE
	cinfo.err = jpeg_std_error(&jerr);
	jerr.error_exit = my_error_exit;
	try {
		jpeg_create_decompress(&cinfo);
		jpeg_stdio_src(&cinfo, infile);
	
		jpeg_read_header(&cinfo, TRUE);
		jpeg_start_decompress(&cinfo);
	}

	catch (System::Exception ^e) {
		// Header read error
		fclose(infile);
		width=0;
		height=0;
		throw gcnew System::Exception("File Format Error in " + fn);
	}
	width=cinfo.output_width;
	height=cinfo.output_height;
	try { data=gcnew array<System::Byte>(width*height*3); } 
	catch (System::Exception ^e) { 
		jpeg_destroy_decompress(&cinfo);
		width=0;
		height=0;
		throw gcnew System::Exception("Out of memory reading File " + fn);
	}

	try {
		buffer = (*cinfo.mem->alloc_sarray)
			((j_common_ptr) &cinfo, JPOOL_IMAGE, 
			width*cinfo.output_components, 1);
		sl=0;
		while (cinfo.output_scanline < cinfo.output_height) {
			jpeg_read_scanlines(&cinfo, buffer, 1);
			for (i=0;i<3*width;i++) data[width*3*sl+i]=buffer[0][i];
			//System::Runtime::InteropServices::Marshal::Copy(data[width*3*sl],0,
			//memcpy(&data[width*3*sl],buffer[0],3*width);
			sl++;
		}

		jpeg_finish_decompress(&cinfo);
		jpeg_destroy_decompress(&cinfo);
	} 
	catch (System::Exception ^e) {
		// JPEG read failed
		fclose(infile);
		data=nullptr;
		bmap=nullptr;
		throw gcnew System::Exception("File Read Error in File " + fn);
	}
	fclose(infile);

	// Convert to bmap
	try { bmap=gcnew Bitmap(width,height); } 
	catch (Exception ^e) { 
		delete data;
		throw gcnew System::Exception("Out of memory reading File " + fn);
	}

	BitmapData^ rawdata=bmap->LockBits(Drawing::Rectangle(0,0,width,height),
		System::Drawing::Imaging::ImageLockMode::ReadWrite,
		System::Drawing::Imaging::PixelFormat::Format24bppRgb);
	int tempsw=rawdata->Stride;
	array<System::Byte>^ tempd=gcnew array<System::Byte>(tempsw*height);
	for (x=0;x<width;x++) {
		for (y=0;y<height;y++) {
			tempd[tempsw*y+x*3+2]=data[y*3*width+x*3+0];
			tempd[tempsw*y+x*3+1]=data[y*3*width+x*3+1];
			tempd[tempsw*y+x*3+0]=data[y*3*width+x*3+2];
		}
	}
	System::Runtime::InteropServices::Marshal::Copy(tempd,0,rawdata->Scan0, tempsw*height );
	bmap->UnlockBits( rawdata );
	delete tempd;
	// Success
}

void imagedata::read_using_windows(String ^fn) {
	int x,y;
	try {
		bmap=gcnew Bitmap(fn);
		width=bmap->Width;
		height=bmap->Height;
		data=gcnew array<System::Byte>(width*height*3);

		BitmapData^ rawdata=bmap->LockBits(Drawing::Rectangle(0,0,width,height),
			System::Drawing::Imaging::ImageLockMode::ReadWrite,
			System::Drawing::Imaging::PixelFormat::Format24bppRgb);
		int tempsw=rawdata->Stride;
		array<System::Byte>^ tempd=gcnew array<System::Byte>(tempsw*height);
		System::Runtime::InteropServices::Marshal::Copy( rawdata->Scan0, tempd, 0, tempsw*height );
		for (x=0;x<width;x++) {
			for (y=0;y<height;y++) {
				data[y*3*width+x*3+0]=tempd[tempsw*y+x*3+2];
				data[y*3*width+x*3+1]=tempd[tempsw*y+x*3+1];
				data[y*3*width+x*3+2]=tempd[tempsw*y+x*3+0];
			}
		}
		bmap->UnlockBits( rawdata );
		delete tempd;
	}
	catch (System::Exception ^e) {
		delete bmap;
		delete data;
		throw gcnew System::Exception("File Read Error in File " + fn);
	}
}

void imagedata::write_to_graphics(Graphics ^g, double minx, double miny, double maxx, double maxy, 
	int centercross, int expand) {
	if (expand) {
		g->DrawImage(bmap,RectangleF(minx,miny,maxx-minx,maxy-miny));
	} else {
		g->InterpolationMode = Drawing2D::InterpolationMode::NearestNeighbor;
		g->DrawImage(bmap,RectangleF(minx,miny,maxx-minx,maxy-miny));
	}
	if (centercross) {
		Pen ^ p = gcnew Pen(Color::Red,1.0f);
		g->DrawLine(p, Point(g->VisibleClipBounds.Width/2,0),Point(g->VisibleClipBounds.Width/2,g->VisibleClipBounds.Height-1));
		g->DrawLine(p, Point(0,g->VisibleClipBounds.Height/2),Point(g->VisibleClipBounds.Width-1,g->VisibleClipBounds.Height/2));
		delete p;
	}
}

void imagedata::write_zoom(Graphics ^g, double minx, double miny, double maxx, double maxy, 
	int centercross, int expand) {
      PointF ulCorner = PointF(0,0);
      PointF urCorner = PointF(g->VisibleClipBounds.Width-1,0);
      PointF llCorner = PointF(0,g->VisibleClipBounds.Height-1);
      array<PointF>^ destPara = {ulCorner,urCorner,llCorner};
	g->InterpolationMode = Drawing2D::InterpolationMode::NearestNeighbor;
	g->DrawImage(bmap,destPara,RectangleF(minx,miny,maxx-minx,maxy-miny),GraphicsUnit::Pixel);
	if (centercross) {
		Pen ^ p = gcnew Pen(Color::Red,1.0f);
		g->DrawLine(p, Point(g->VisibleClipBounds.Width/2,0),Point(g->VisibleClipBounds.Width/2,g->VisibleClipBounds.Height-1));
		g->DrawLine(p, Point(0,g->VisibleClipBounds.Height/2),Point(g->VisibleClipBounds.Width-1,g->VisibleClipBounds.Height/2));
		delete p;
	}
}


int imagedata::getcolor(int x, int y, int *r, int *g, int *b) {
	if ((x < 0) || (x >= width) || (y < 0) || (y >= height)) return 0;
	*r = data[y*3*width+x*3+0];
	*g = data[y*3*width+x*3+1];
	*b = data[y*3*width+x*3+2];
	return 1;
}

	
void imagedata::write_to_picmap(array<System::Byte> ^d, int sw, int BWidth, int BHeight,
			double minx, double miny, double maxx, double maxy, int centercross, int expand) {
/*	Bitmap ^bmap;
	int x,y,xr,yr;
	int xs,ys;
	//array<Byte>^ d;
	//int sw;

	if (width*height <= 0) return;

	//bmap = gcnew Bitmap(pbox->Size.Width,pbox->Size.Height);
	//pbox->Image = bmap;
	
	//BitmapData^ rawdata=bmap->LockBits(Drawing::Rectangle(0,0,pbox->Size.Width,pbox->Size.Height),
	//	System::Drawing::Imaging::ImageLockMode::ReadWrite,
	//	System::Drawing::Imaging::PixelFormat::Format24bppRgb);
	//sw=rawdata->Stride;
	//d=gcnew array<Byte>(sw*pbox->Size.Height);

	if (expand) {
		int minr=1;
		int maxr=0;
		int ming=1;
		int maxg=0;
		int minb=1;
		int maxb=0;
		for (x=floor(minx);x<=ceil(maxx);x++) {
			for (y=floor(miny);y<ceil(maxy);y++) {
				if ((x >=0) && (x < width) && (y >= 0) && (y < height)) {
					int r=data[y*width*3+x*3];
					int g=data[y*width*3+x*3+1];
					int b=data[y*width*3+x*3+2];
					if (minr > maxr) { minr=maxr=r; ming=maxg=g; minb=maxb=b; }
					if (r > maxr) maxr=r; if (r < minr) minr=r;
					if (g > maxg) maxg=g; if (g < ming) ming=g;
					if (b > maxb) maxb=b; if (b < minb) minb=b;
				}
			}
		}
		if (minr > maxr) { minr=ming=minb=0;maxr=maxg=maxb=255; }
		if (maxr==minr) maxr++;
		if (maxg==ming) maxg++;
		if (maxb==minb) maxb++;
		for (x=0;x<BWidth;x++) {
			xr=floor(minx+x*(maxx-minx)/BWidth+0.5);
			xs=x % 32;
			for (y=0;y<BHeight;y++) {
				yr=floor(miny+y*(maxy-miny)/BHeight+0.5);
				ys=y % 32;
				if (centercross && (x==BWidth/2 || y==BHeight/2)) {
					d[y*sw+3*x+2]=255;
					d[y*sw+3*x+1]=0;
					d[y*sw+3*x+0]=0;
				} else {
					if ((xr >= 0) && (xr < width) && (yr >= 0) && (yr < height)) {
						d[y*sw+3*x]=(data[yr*width*3+3*xr+2]-minb)*255/(maxb-minb);
						d[y*sw+3*x+1]=(data[yr*width*3+3*xr+1]-ming)*255/(maxg-ming);
						d[y*sw+3*x+2]=(data[yr*width*3+3*xr]-minr)*255/(maxr-minr);
					} else {
						//if ((xs == ys) || (31-xs==ys)) {
							d[y*sw+3*x]=128;
							d[y*sw+3*x+1]=128;
							d[y*sw+3*x+2]=128;
						//} else {
						//	d[y*sw+3*x]=255;
						//	d[y*sw+3*x+1]=255;
						//	d[y*sw+3*x+2]=255;
						//}
					}
				}
			}
		}
	} else {
		for (x=0;x<BWidth;x++) {
			xr=floor(minx+x*(maxx-minx)/BWidth+0.5);
			xs=x % 32;
			for (y=0;y<BHeight;y++) {
				yr=floor(miny+y*(maxy-miny)/BHeight+0.5);
				ys=y % 32;
				if ((xr >= 0) && (xr < width) && (yr >= 0) && (yr < height)) {
					d[y*sw+3*x]=data[yr*width*3+3*xr+2];
					d[y*sw+3*x+1]=data[yr*width*3+3*xr+1];
					d[y*sw+3*x+2]=data[yr*width*3+3*xr];
				} else {
					if ((xs == ys) || (31-xs==ys)) {
						d[y*sw+3*x]=128;
						d[y*sw+3*x+1]=128;
						d[y*sw+3*x+2]=128;
					} else {
						d[y*sw+3*x]=255;
						d[y*sw+3*x+1]=255;
						d[y*sw+3*x+2]=255;
					}
				}
			}
		}
	}
	//System::Runtime::InteropServices::Marshal::Copy( d, 0, rawdata->Scan0, sw*pbox->Size.Height );
	//bmap->UnlockBits( rawdata );
	//pbox->Refresh(); */
}

profile::profile(imagedata ^id, vector s, vector e) {
	img=id;
	start=s;
	stop=e;
	pts=0;
	X=R=G=B=nullptr;
	calcprofile();
}

profile::~profile() {
	if (X != nullptr) delete X;
	if (R != nullptr) delete R;
	if (G != nullptr) delete G;
	if (B != nullptr) delete B;
	X=R=G=B=nullptr;
	pts=0;
}

void profile::shift(vector news, vector newe) {
	if ((start == news) && (stop == newe)) return;
	start=news;
	stop=newe;
	calcprofile();
}

double dbl_min(double a, double b) { return (a < b)?a:b; }
double dbl_max(double a, double b) { return (a > b)?a:b; }

void intersect(double *ms, double *me, double m1, double m2) {
	if (m1 > m2) {
		double t=m1;
		m1=m2;
		m2=t;
	}
	if (*me < *ms) return; // already empty
	if (m2 < *ms) { // empty
		*me=0;
		*ms=1;
		return;
	}
	if (m1 > *me) { // empty
		*me=0;
		*ms=1;
		return;
	}
	// overlap
	*ms = dbl_max(*ms,m1);
	*me = dbl_min(*me,m2);
}

double gauss(double r) {
	return exp(-r*r);
}

void profile::calcprofile() {
	vector v;
	double l;
	double ms, me;
	
	if (X != nullptr) delete X;
	if (R != nullptr) delete R;
	if (G != nullptr) delete G;
	if (B != nullptr) delete B;
	X=R=G=B=nullptr;
	pts=0;

	
	v=stop-start;
	ms=0.0;me=1.0;
	if (v.x != 0) {
		double m1=-start.x/v.x;
		double m2=((double)img->width-1.0-start.x)/v.x;
		intersect(&ms,&me,m1,m2);
	}
	if (v.y != 0) {
		double m1=-start.y/v.y;
		double m2=((double)img->height-1.0-start.y)/v.y;
		intersect(&ms,&me,m1,m2);
	}
	if (ms >= me) {
		// empty
		return;
	}
	
	l=v.len;
	pts=floor(l/0.5);
	X=gcnew array<double>(pts);
	R=gcnew array<double>(pts);
	G=gcnew array<double>(pts);
	B=gcnew array<double>(pts);
	
	for (int i=0;i<pts;i++) {
		double q=ms+(double)i*(me-ms)/(pts-1);
		vector xc=start+q*v;
		double im, ir, ig, ib;
		im=ir=ig=ib=0.0;
		for (double mx=floor(xc.x+0.5)-3;mx<=floor(xc.x+0.5)+3;mx+=1.0) 
			for (double my=floor(xc.y+0.5)-3;my<=floor(xc.y+0.5)+3;my+=1.0) {
				int iir,iig,iib;
				if (img->getcolor((int) mx, (int) my, &iir, &iig, &iib)) {
					double flt=gauss(len(vector(mx,my)-xc)/1.0);
					im+=flt;
					ir+=flt*(double)iir/255.0;
					ig+=flt*(double)iig/255.0;
					ib+=flt*(double)iib/255.0;
				}					
			}
		if (im >= 0) {
			ir/=im;
			ig/=im;
			ib/=im;
		}
		X[i]=q;
		R[i]=ir;
		G[i]=ig;
		B[i]=ib;
	}
}

double profile::findcolor(double r, double g, double b) {
	double mindist;
	int i,minidx;
	if (X == nullptr) return -1;
	mindist=sqrt((R[0]-r)*(R[0]-r)+(G[0]-g)*(G[0]-g)+(B[0]-b)*(B[0]-b));
	minidx=0;
	for (i=1;i<pts;i++) {
		double dist=sqrt((R[i]-r)*(R[i]-r)+(G[i]-g)*(G[i]-g)+(B[i]-b)*(B[i]-b));
		if (dist < mindist) {
			mindist=dist;
			minidx=i;
		}
	}
	return X[minidx];
}

