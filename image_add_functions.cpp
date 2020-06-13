#include "stdafx.h"
#include "image_add_win.h"
#include "about_win.h"
#include "ia_xml_io.h"
#include <math.h>

using namespace image_add;

void image_add_win::clearmdata() {
	int i=0;
	while (i< mdata->Count) {
		if (mdata[i]->readonly==1) 
			i++;
		else {
			delete mdata[i];
			mdata->RemoveAt(i);
			//itemlist->
		}
	}
	selection = nullptr;
}

System::Void image_add_win::initialize_new_image() {
	double scale;

	this->Text = gcnew String(imd->filename);
	cs->frm->imgfilename=gcnew String(imd->filename);
	cs->frm->imgfileinfo=String::Format("W={0}, H={1}",imd->width,imd->height);
	cs->img=imd;

	// fit to image as a starting point
	//crx=cry=0;
	double ascreen=(double) pbox2->Size.Width/pbox2->Size.Height;
	double apic=(double) imd->width/imd->height;
	if (ascreen > apic) {
		// Screen is wider --> match height
		scale=(double)pbox2->Size.Height/imd->height;
	} else {
		scale=(double)pbox2->Size.Width/imd->width;
	}

//	coord=gcnew coordinate(vector(0,0),vector(0,0),
//		vector(imd->width/2,0),vector((double)imd->width/imd->height,0),
//		vector(0,-imd->height/2),vector(0,1));
	cs->c->p1->data=vector(0,0);
	cs->c->p1->real=vector(0,0);
	cs->c->p2->data=vector(imd->width/2,0);
	cs->c->p2->real=vector((double)imd->width/imd->height,0);
	cs->c->cmode=CoordMode::TwoPoint;
	cs->c->calctrf();
	//coord=gcnew coordinate(vector(0,0),vector(0,0),
	//	vector(imd->width/2,0),vector((double)imd->width/imd->height,0));

	cs->t->resize(vector(pbox2->Size.Width,pbox2->Size.Height));
	cs->t->rescale(scale);
	cs->t->recenter(vector(0,0));

	clearmdata();

	redraw();
}

System::Void image_add_win::open_image(String ^filename) {
	imagedata ^imdtemp;

	String ^fn, ^ext;
	fn=Path::GetFileName(filename);
	ext=Path::GetExtension(filename);
	if (String::Compare(ext,".IMA",StringComparison::OrdinalIgnoreCase)==0)
		read_xml_file(filename,0);
	else if (String::Compare(ext,".IMC",StringComparison::OrdinalIgnoreCase)==0)
			read_xml_file(filename,1);
	else {
		// Regular File
		imdtemp=gcnew imagedata(filename);
		if (!imdtemp->filename) {
			delete imdtemp;
			MessageBox::Show( "Failed to Read File " + filename );
			return;
		}
		delete imd;
		imd=imdtemp;
		cs->frm->filename="[undefined]";
		cs->frm->calname="[undefined]";
		cs->img=imd;
		initialize_new_image();
	}
}

void read_xml_elements(ia_xml_read ^x, csys ^tcs, List<cmeasurement^> ^md,int asdefault) {
	int foundcrd,foundfrm;
	foundcrd=foundfrm=0;
	while (1) {
		switch (x->NodeType) {
			case XmlNodeType::Element:
				if (String::Compare(x->Name,"Point")==0) {
					x->Read();md->Add(gcnew cpoint(tcs,x));
				} else if (String::Compare(x->Name,"Line")==0) {
					x->Read();md->Add(gcnew clinear(tcs,x));
				} else if (String::Compare(x->Name,"ColorBar")==0) {
					x->Read();md->Add(gcnew ccolorbar(tcs,x));
				} else if (String::Compare(x->Name,"Profile")==0) {
					x->Read();md->Add(gcnew cprofile(tcs,x));
				} else if (String::Compare(x->Name,"circle3p")==0) {
					x->Read();md->Add(gcnew ccircle3p(tcs,x));
				} else if (String::Compare(x->Name,"angle")==0) {
					x->Read();md->Add(gcnew cangle(tcs,x));
				} else if (String::Compare(x->Name,"area")==0) {
					x->Read();md->Add(gcnew carea(tcs,x));
				} else if (String::Compare(x->Name,"annotation")==0) {
					x->Read();md->Add(gcnew cannotation(tcs,x));
				} else if (String::Compare(x->Name,"CoordinateSystem")==0) {
					if (asdefault) 
						x->Skip();
					else {
						x->Read();md->Add(gcnew coordinate(tcs,x));
						foundcrd=1;
					}
				} else if (String::Compare(x->Name,"Frame")==0) {
					x->Read();md->Add(gcnew cframe(tcs,x));
					foundfrm=1;
				} else {
					// Unknown element --> Skip over
					if (x->IsEmptyElement)	x->Read(); else x->Skip();
				}
				break;
			case XmlNodeType::EndElement:
				if (!asdefault) {
					if (!foundcrd || !foundfrm) 
						throw gcnew System::Exception("Bad XML Formatted File <ELEMENTS> tag");
				}
				x->Read();
				return;
			default:
				throw gcnew System::Exception("Bad XML Formatted File <ELEMENTS> tag");
		}
	}
}

void image_add_win::read_xml_file(String ^filename, int asrefonly) {
	String ^imd_filename;
	int imd_width, imd_height;
	String ^el_name;
	String ^el_type;
	List<cmeasurement^> ^md = gcnew List<cmeasurement ^>(); 
	List<cmeasurement^> ^md2 = gcnew List<cmeasurement ^>(); 
	csys ^tcs = gcnew csys(gcnew formatter(), gcnew transformer(cs->t->screen));
	imagedata ^embedded_imd=nullptr;
	double scale;
	try {
		ia_xml_read ^x=gcnew ia_xml_read(filename);
		int endread;
		//x->WhitespaceHandling=WhitespaceHandling::None;
		int mode=0;
		// Step to Root Element 
		if (!x->ReadToFollowing("IMAGEANALYZER")) throw gcnew System::Exception("XML File is not one of ImageAnalyzer");
		x->Read();
		endread=0;
		while (!endread) {
			switch (x->NodeType) {
			case XmlNodeType::Element: 
				if (String::Compare(x->Name,"IMAGEINFO")==0) {
					int foundn,founddim;
					int embedded=0;
					int finished=0;
					foundn=founddim=0;
					x->Read();
					while (!finished) {
						switch (x->NodeType) {
							case XmlNodeType::Element:
								if (String::Compare(x->Name,"FILENAME")==0) {
									if (x->IsEmptyElement) 	imd_filename=""; else 
										imd_filename=x->ReadElementContentAsString();
									foundn=1;
								} else if (String::Compare(x->Name,"DIMENSION")==0) {
									x->Read(); x->read_xml_dimension(&imd_width,&imd_height); 
									founddim=1;
								} else if (String::Compare(x->Name,"EMBEDDED")==0) {
									embedded=x->ReadElementContentAsInt();
								} else if (String::Compare(x->Name,"IMAGE")==0) {
									if (asrefonly) {
										// Skip
										x->Skip();
									} else {
										// ReadImage
										x->Read();
										embedded_imd=gcnew imagedata();
										embedded_imd->filename="EMBEDDED";
										embedded_imd->read_from_xml(x);
										tcs->img=embedded_imd;
									}
								} else {
									// Unknown element --> Skip over
									if (x->IsEmptyElement)	x->Read(); else x->Skip();
								}
								break;
							case XmlNodeType::EndElement:
								if (!foundn || !founddim) 
									throw gcnew System::Exception("Bad XML Formatted File <IMAGEINFO> tag");
								x->Read();
								finished=1;
								break;
							default:
								throw gcnew System::Exception("Bad XML Formatted File");
						}
					}
				} else if (String::Compare(x->Name,"ELEMENTS")==0) {
					x->Read();read_xml_elements(x,tcs,md,0);
				} else if (String::Compare(x->Name,"DEFAULTS")==0) {
					x->Read();read_xml_elements(x,tcs,md2,1);
					if (!asrefonly) {
						if (templates!=nullptr) {
							while (templates->Count > 0) {
								delete templates[0];
								templates->RemoveAt(0);
							}
							delete templates;
							templates=nullptr;
						}
						templates=gcnew List<cmeasurement^>();
						templates->AddRange(md2);
					}
				} else {
					// Unknown Element --> Ignore
					if (!x->IsEmptyElement)	x->Skip();
				}
				break;
			case XmlNodeType::EndElement: 
				endread=1;
				break;
			default: throw gcnew System::Exception("Bad XML File");
			}
		}
		x->Close();
		delete x;
	} catch (Exception ^e) {
		MessageBox::Show(e->ToString());
		return;
	}
	// Still here? Read the image file
	
	if (asrefonly) {
		if ((imd == nullptr) || (imd->width*imd->height <= 0)) {
			MessageBox::Show( "Image must be loaded befor applying a calibration");
			return;
		}
		if ((imd->width != imd_width) || (imd->height != imd_height)){
			MessageBox::Show( "File " + filename + " specifies different image dimensions --> Calibration Cannot be applied");
			return;
		}
		tcs->frm->calname=filename;
	} else {
		// Try to read the file close to the xml file
		if (embedded_imd == nullptr) {
			imagedata ^imdtemp=gcnew imagedata(Path::GetDirectoryName(filename) + "\\" + Path::GetFileName(imd_filename));
			if (!imdtemp->filename) {
				delete imdtemp;
				// Now the full path
				imdtemp = gcnew imagedata(imd_filename);
				if (!imdtemp->filename) {
					delete imdtemp;
					// Try within the current working directory
					imdtemp = gcnew imagedata(Path::GetFileName(imd_filename));
					if (!imdtemp->filename) {
						// Fail finally
						delete imdtemp;
						MessageBox::Show( "Failed to Read File " + imd_filename );
						return;
					}
				}
			}
			if ((imdtemp->width != imd_width) || (imdtemp->height != imd_height)){
				delete imdtemp;
				MessageBox::Show( "File " + imd_filename + " hase wrong dimension");
				return;
			}
			if (imd != nullptr) delete imd;
			imd=imdtemp;
			tcs->img=imd;
		} else {
			// Was XML Embedded
			if (imd != nullptr) delete imd;
			imd=embedded_imd;
			tcs->img=imd;
		}
		tcs->frm->imgfilename=imd->filename;
		tcs->frm->imgfileinfo=String::Format("W={0}, H={1}",imd->width,imd->height);
	}

	clearmdata();
	mdata=md;
	cs=tcs;

	if (!asrefonly) {
		this->Text = gcnew String(imd->filename);
		double ascreen=(double) pbox2->Size.Width/pbox2->Size.Height;
		double apic=(double) imd->width/imd->height;
		if (ascreen > apic) {
			// Screen is wider --> match height
			scale=(double)pbox2->Size.Height/imd->height;
		} else {
			scale=(double)pbox2->Size.Width/imd->width;
		}
		//cs->c->p1->data=vector(0,0);
		//cs->c->p1->real=vector(0,0);
		//cs->c->p2->data=vector(imd->width/2,0);
		//cs->c->p2->real=vector((double)imd->width/imd->height,0);
		//cs->c->twopoint=1;
		cs->c->calctrf();
		cs->frm->filename=gcnew String(filename);
	} else {
		double ascreen=(double) pbox2->Size.Width/pbox2->Size.Height;
		double apic=(double) imd->width/imd->height;
		if (ascreen > apic) {
			// Screen is wider --> match height
			scale=(double)pbox2->Size.Height/imd->height;
		} else {
			scale=(double)pbox2->Size.Width/imd->width;
		}
		cs->frm->calname=gcnew String(filename);
	}
	cs->t->resize(vector(pbox2->Size.Width,pbox2->Size.Height));
	cs->t->rescale(scale);
	cs->t->recenter(vector(0,0));
	cs->img=imd;
	redraw();
}

System::Void image_add_win::menu_open_image_Click(System::Object^  sender, System::EventArgs^  e) {
	OpenFileDialog^ sfd = gcnew OpenFileDialog();
	//char fn[4096];
	//int i;
	double scale;

    sfd->Filter = "Image Files|*.jpg;*.bmp;*.png;*.tif;*.IMA|All Files|*.*";
    if( sfd->ShowDialog() != System::Windows::Forms::DialogResult::OK )
    {
        return;
    }

	open_image(sfd->FileName);
}

System::Void image_add_win::ib_open_Click(System::Object^  sender, System::EventArgs^  e) {
	menu_open_image_Click(sender, e);
}

System::Void image_add_win::menu_cal_load_Click(System::Object^  sender, System::EventArgs^  e) {
	OpenFileDialog^ sfd = gcnew OpenFileDialog();
	double scale;

    sfd->Filter = "Cal Files|*.IMC|All Files|*.*";
    if( sfd->ShowDialog() != System::Windows::Forms::DialogResult::OK )
    {
        return;
    }

	read_xml_file(sfd->FileName,1);
}

System::Void image_add_win::menu_cal_save_Click(System::Object^  sender, System::EventArgs^  e) {
	SaveFileDialog^ sfd = gcnew SaveFileDialog();
    sfd->Filter = "Cal Files|*.IMC|All Files|*.*";
    if( sfd->ShowDialog() != System::Windows::Forms::DialogResult::OK )
    {
        return;
    }
	write_xml_file(sfd->FileName,0);
}

System::Void image_add_win::ib_paste_Click(System::Object^  sender, System::EventArgs^  e) {
	imagedata ^imdtemp;
	double scale;
	imdtemp=gcnew imagedata(0);
	if (!imdtemp->filename) {
		delete imdtemp;
		MessageBox::Show( "No Suitable Data in the clip board");
		return;
	}
	delete imd;
	imd=imdtemp;

	initialize_new_image();
}

System::Void image_add_win::image_add_win_DragEnter(System::Object^  sender, System::Windows::Forms::DragEventArgs^  e) {
	if (e->Data->GetDataPresent(DataFormats::FileDrop)) {
		array<String^> ^f=(array<String^>^) e->Data->GetData(DataFormats::FileDrop, false);
		if (f->Length == 1)
			e->Effect = DragDropEffects::All;
		else
			e->Effect = DragDropEffects::None;
	} else
		e->Effect = DragDropEffects::None;
}

System::Void image_add_win::image_add_win_DragDrop(System::Object^  sender, System::Windows::Forms::DragEventArgs^  e) {
	array<String^> ^f=(array<String^>^) e->Data->GetData(DataFormats::FileDrop, false);
	if (f->Length == 1) {
		open_image(f[0]);
	}
}



//[System.Runtime.InteropServices.DllImport("gdi32")]
//public static extern int GetEnhMetaFileBits(int hemf, int cbBuffer, byte[] lpbBuffer); 
[System::Runtime::InteropServices::DllImport("user32.dll")]
extern bool OpenClipboard(int *hWndNewOwner);
[System::Runtime::InteropServices::DllImport("user32.dll")]
extern bool EmptyClipboard();
[System::Runtime::InteropServices::DllImport("user32.dll")]
extern int *SetClipboardData(unsigned int uFormat, int *hMem);
[System::Runtime::InteropServices::DllImport("user32.dll")]
extern bool CloseClipboard();
[System::Runtime::InteropServices::DllImport("gdi32.dll")]
extern int *CopyEnhMetaFile(int *hemfSrc, int *hNULL);
[System::Runtime::InteropServices::DllImport("gdi32.dll")]
extern bool DeleteEnhMetaFile(int *hemf);

System::Void image_add_win::ib_copyclip_Click(System::Object^  sender, System::EventArgs^  e) {
	// Copy Image to Clipboard
	Bitmap ^bm=gcnew Bitmap((int)cs->t->screen.x,(int)cs->t->screen.y);
	Graphics ^g=Graphics::FromImage(bm);
	g->SmoothingMode = System::Drawing::Drawing2D::SmoothingMode::AntiAlias;
	paint_to(g);
	delete g;
	Clipboard::SetDataObject(bm,true);
	// delete bm;
}

System::Void image_add_win::ib_copyclip_emf_Click(System::Object^  sender, System::EventArgs^  e) {
	// Copy Image to Clipboard as Enhanced MetaFile
	
	Graphics ^g=Graphics::FromHwndInternal(IntPtr::Zero);
	IntPtr hdc = g->GetHdc();
	//MemoryStream ^mfstream=gcnew MemoryStream();
	//Metafile ^mf=gcnew Metafile(mfstream,hdc,EmfType::EmfOnly);
	RectangleF rf = RectangleF(0,0,cs->t->screen.x-1,cs->t->screen.y-1);
	Metafile ^mf=gcnew Metafile(hdc,rf,MetafileFrameUnit::Pixel,EmfType::EmfOnly);
	Graphics ^g2=Graphics::FromImage(mf);
	paint_to(g2);
	delete g2;
	
	IntPtr hEMF=mf->GetHenhmetafile();
	if ( !hEMF.Equals(gcnew IntPtr(0)) ) {
		int *hEMF2 = CopyEnhMetaFile((int*)hEMF.ToPointer(),0);
		if( hEMF2 != 0 ) {
			IntPtr hwnd=this->Handle;
			if( OpenClipboard((int *) hwnd.ToPointer())) {
				if (EmptyClipboard()) {
					int *hRes = SetClipboardData( 14 /*CF_ENHMETAFILE*/, hEMF2 );
					// bResult = hRes.Equals( hEMF2 );
					CloseClipboard();
				}
			}
		}
	}
	g->ReleaseHdc(hdc);
	return;
}

System::Void image_add_win::ib_copyCSV_Click(System::Object^  sender, System::EventArgs^  e) {
	if (imd == nullptr) return;
	if (imd->filename == nullptr) return;
	if (cs == nullptr) return;
	String ^s=gcnew String("");
	for (int i=0;i<mdata->Count;i++) {
		if (mdata[i]->readonly) {
			String ^sn = mdata[i]->exportCSV();
			if (sn != nullptr)
				s+=sn;
		}
	}
	for (int i=0;i<mdata->Count;i++) {
		if (!mdata[i]->readonly) {
			String ^sn = mdata[i]->exportCSV();
			if (sn != nullptr)
				s+=sn;
		}
	}
	Clipboard::SetDataObject(s,true);
}

//static ImageCodecInfo^ GetEncoderInfo( ImageFormat^ format );

static ImageCodecInfo^ GetEncoderInfo( ImageFormat^ format )
{
   int j;
   array<ImageCodecInfo^>^encoders;
   encoders = ImageCodecInfo::GetImageEncoders();
   for ( j = 0; j < encoders->Length; ++j )
   {
      if ( encoders[ j ]->FormatID == format->Guid)
            return encoders[ j ];

   }
   return nullptr;
}

System::Void image_add_win::ib_export_Click(System::Object^  sender, System::EventArgs^  e) {
	SaveFileDialog^ sfd = gcnew SaveFileDialog();
    sfd->Filter = "Export Image File|*.jpg|All Files|*.*";
    if( sfd->ShowDialog() != System::Windows::Forms::DialogResult::OK )
        return;

	// Copy Image to Clipboard
	Bitmap ^bm=gcnew Bitmap((int)cs->t->screen.x,(int)cs->t->screen.y);
	Graphics ^g=Graphics::FromImage(bm);
	g->SmoothingMode = System::Drawing::Drawing2D::SmoothingMode::AntiAlias;
	paint_to(g);
	delete g;

	ImageCodecInfo^ jpgcodecinfo=GetEncoderInfo( ImageFormat::Jpeg );
	EncoderParameters^ encpara = gcnew EncoderParameters( 1 );
	encpara->Param[ 0 ] = gcnew EncoderParameter( Encoder::Quality,__int64(95) );
	 
	bm->Save(sfd->FileName, jpgcodecinfo, encpara);
	delete bm;
}

void image_add_win::write_xml_file(String ^fn,int embed_image) {
	ia_xml_write ^x;
	x=gcnew ia_xml_write(fn);
	x->WriteStartElement("IMAGEANALYZER");
		x->WriteStartElement("IMAGEINFO");
			x->write_value("FILENAME",imd->filename);
			x->write_value("EMBEDDED",embed_image);
			x->write_dimension("DIMENSION",imd->width,imd->height);
			if (embed_image) {
				x->WriteStartElement("IMAGE");
					imd->write_to_xml(x);
				x->WriteEndElement();x->WriteWhitespace("\n");
			}
		x->WriteEndElement();x->WriteWhitespace("\n");
		x->WriteStartElement("ELEMENTS");
			for (int i=0;i<mdata->Count;i++) {
				x->WriteStartElement(mdata[i]->type);
					mdata[i]->write_to_xml(x);
				x->WriteEndElement();x->WriteWhitespace("\n");
			}
		x->WriteEndElement();x->WriteWhitespace("\n");
		x->WriteStartElement("DEFAULTS");
			for (int i=0;i<templates->Count;i++) {
				x->WriteStartElement(templates[i]->type);
					templates[i]->write_to_xml(x);
				x->WriteEndElement();x->WriteWhitespace("\n");
			}
		x->WriteEndElement();x->WriteWhitespace("\n");
	x->WriteEndElement();x->WriteWhitespace("\n");
	x->Close();
	delete x;	
	cs->frm->filename=gcnew String(fn);
}

System::Void image_add_win::ib_save_Click(System::Object^  sender, System::EventArgs^  e) {
	SaveFileDialog^ sfd = gcnew SaveFileDialog();
    sfd->Filter = "IMA Files|*.IMA|All Files|*.*";
    if( sfd->ShowDialog() != System::Windows::Forms::DialogResult::OK )
    {
        return;
    }
	write_xml_file(sfd->FileName,1);

}

System::Void image_add_win::toolStripMenuItem2_Click(System::Object^  sender, System::EventArgs^  e) {
	ib_save_Click(sender, e);
}

System::Void image_add_win::menu_save_meta_only_Click(System::Object^  sender, System::EventArgs^  e) {
	SaveFileDialog^ sfd = gcnew SaveFileDialog();
    sfd->Filter = "IMA Files|*.IMA|All Files|*.*";
    if( sfd->ShowDialog() != System::Windows::Forms::DialogResult::OK )
    {
        return;
    }
	write_xml_file(sfd->FileName,0);
}


System::Void image_add_win::ib_zoomout_Click(System::Object^  sender, System::EventArgs^  e) {
	if ((imd == nullptr) || (imd->width*imd->height <= 0)) return;

	double ascreen=(double) pbox2->Size.Width/pbox2->Size.Height;
	double apic=(double) imd->width/imd->height;
	double scale;
	if (ascreen > apic) {
		// Screen is wider --> match height
		scale=(double)pbox2->Size.Height/imd->height;
	} else {
		scale=(double)pbox2->Size.Width/imd->width;
	}
	cs->t->resize(vector(pbox2->Size.Width,pbox2->Size.Height));
	cs->t->rescale(scale);
	cs->t->recenter(vector(0,0));

	repaint();
}

System::Void image_add_win::aboutToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e) {
	about_win ^w=gcnew about_win();
	w->VERSION->Text=gcnew String(IA_VERSION);
	w->ShowDialog();
	delete w;
}

vector image_add_win::getrealcoord(vector sc) {
	vector inp=cs->t->todata(sc);
	vector real=cs->c->getreal(inp);
	return real;
}

System::Void image_add_win::redraw(void) {
	// pbox2->Invalidate();
	repaint();
}

void image_add_win::paint_to(Graphics ^g) {
	double cx1,cx2,cy1,cy2;
	int i;

	if (cs == nullptr) return;

	//Graphics ^g=pbox->CreateGraphics();
	g->Clear(cs->c->bkg); // Color::FromArgb(255,255,255));

	if (!imd || (imd->width*imd->height <= 0)) {
		//delete g;
		return;
	}

	vector c1=cs->t->toscreen(vector(-imd->width/2,-imd->height/2));
	vector c2=cs->t->toscreen(vector(imd->width/2,imd->height/2));

	imd->write_to_graphics(g, c1.x, c1.y, c2.x, c2.y,0,0);

	for (i=0;i<mdata->Count;i++) {
		if (!mdata[i]->readonly)
			mdata[i]->draw(g);
	}
	for (i=0;i<mdata->Count;i++) {
		if (mdata[i]->readonly)
			mdata[i]->draw(g);
	}
}

System::Void image_add_win::repaint(void) {

	if (pbox2 == nullptr) return;
	if (cs == nullptr) return;

	if ((pbox2->Size.Width != cs->t->screen.x) || (pbox2->Size.Height != cs->t->screen.y)) {
		// Resize required
		cs->t->resize(vector(pbox2->Size.Width,pbox2->Size.Height));
		//delete ctxt->MaximumBuffer;
		ctxt->MaximumBuffer = System::Drawing::Size(pbox2->Size.Width+1, pbox2->Size.Height+1);
		delete pbox_grafx;
		pbox_grafx = ctxt->Allocate(pbox2->CreateGraphics(), Rectangle( 0, 0, pbox2->Size.Width, pbox2->Size.Height ));
		delete pbox_g;
		pbox_g=pbox_grafx->Graphics;
		delete pbox_real_g;
		pbox_real_g=pbox2->CreateGraphics();	
		pbox_g->SmoothingMode = System::Drawing::Drawing2D::SmoothingMode::AntiAlias;
	}

/*	BitmapData^ rawdata=pbox_bmap->LockBits(Drawing::Rectangle(0,0,pbox->Size.Width,pbox->Size.Height),
		System::Drawing::Imaging::ImageLockMode::ReadWrite,
		System::Drawing::Imaging::PixelFormat::Format24bppRgb);
	System::Runtime::InteropServices::Marshal::Copy( pbox_d, 0, rawdata->Scan0, pbox_sw*pbox->Size.Height );
	pbox_bmap->UnlockBits( rawdata );
	pbox->Refresh(); */

	paint_to(pbox_g);

	if (mousemode == 1) {
		Pen ^ pen=gcnew Pen( Color::FromArgb(80,255,80),1.0f );
		vector mp1=cs->t->toscreen(mousepos);
		vector mp2=cs->t->toscreen(mousedownpos);
		float minx,miny,w,h;
		if (mp1.x < mp2.x) minx=mp1.x; else minx=mp2.x;
		if (mp1.y < mp2.y) miny=mp1.y; else miny=mp2.y;
		w=fabs(mp1.x-mp2.x);
		h=fabs(mp1.y-mp2.y);
		pbox_g->DrawRectangle(pen,minx,miny,w,h);
		delete pen;
	}
	if (mousemode == 3) {
		Pen ^ pen=gcnew Pen( Color::FromArgb(80,255,80),1.0f );
		vector mp1=cs->t->toscreen(mousepos);
		vector mp2=cs->t->toscreen(mousedownpos);
		pbox_g->DrawLine(pen,(float)mp1.x,(float)mp1.y,(float)mp2.x,(float)mp2.y);
		delete pen;
	}
	if (mousemode == 4) {
		Pen ^ pen=gcnew Pen( Color::FromArgb(80,255,80),1.0f );
		vector mp=cs->t->toscreen(mousepos);
		pbox_g->DrawLine(pen,(float)mp.x,(float)0,(float)mp.x,(float)cs->t->screen.y);
		pbox_g->DrawLine(pen,(float)0,(float)mp.y,(float)cs->t->screen.x,(float)mp.y);
		delete pen;
	}
	if (mousemode == 5) {
		vector mp1,mp2;
		Pen ^ pen=gcnew Pen( Color::FromArgb(80,255,80),1.0f );
		if (plist->Count > 1) {
			for (int i=1;i<plist->Count;i++) {
				vector v1;
				v1.x = plist[i - 1]->x;
				v1.y = plist[i - 1]->y;
				vector v2 = *plist[i];
				mp1=cs->t->toscreen(v1);
				mp2=cs->t->toscreen(v2);
				pbox_g->DrawLine(pen,(float)mp1.x,(float)mp1.y,(float)mp2.x,(float)mp2.y);
			}
		} else {
			mp2=cs->t->toscreen(*plist[0]);
		}
		mp1=cs->t->toscreen(mousepos);
		pbox_g->DrawLine(pen,(float)mp1.x,(float)mp1.y,(float)mp2.x,(float)mp2.y);
		delete pen;
	}
	pbox_grafx->Render(pbox_real_g);

}

System::Void image_add_win::pbox_SizeChanged(System::Object^  sender, System::EventArgs^  e) {
	redraw();
}

System::Void image_add_win::pbox_Click(System::Object^  sender, System::EventArgs^  e) {
}

void image_add_win::setmode(int om) {
	opmode=om;
	ib_pointer->Checked=(opmode == 0)?true:false;
	ib_line->Checked=(opmode == 1)?true:false;
	ib_point->Checked=(opmode == 2)?true:false;
	ib_circle3p->Checked=(opmode == 3)?true:false;
	ib_angle->Checked=(opmode == 4)?true:false;
	ib_area->Checked=(opmode == 5)?true:false;
	ib_path->Checked=(opmode == 6)?true:false;
	ib_annot_box->Checked=(opmode == 7)?true:false;
	ib_annot_circle->Checked=(opmode == 8)?true:false;
	ib_annot_rect->Checked=(opmode == 9)?true:false;
	ib_annot_line->Checked=(opmode == 10)?true:false;
	ib_annot_arrow->Checked=(opmode == 11)?true:false;
	ib_profile->Checked=(opmode == 12)?true:false;
	ib_colorbar->Checked=(opmode == 13)?true:false;
	switch (opmode) {
	case 1: 
		if ((selection == nullptr) || (selection->Count == 0)) propgrid->SelectedObject=find_template(TEMPL_LINEAR);
		break;
	case 2: 
		if ((selection == nullptr) || (selection->Count == 0)) propgrid->SelectedObject=find_template(TEMPL_POINT);
		break;
	case 3: 
		if ((selection == nullptr) || (selection->Count == 0)) propgrid->SelectedObject=find_template(TEMPL_CIRCLE3P);
		break;
	case 4: 
		if ((selection == nullptr) || (selection->Count == 0)) propgrid->SelectedObject=find_template(TEMPL_ANGLE);
		break;
	case 5: 
		if ((selection == nullptr) || (selection->Count == 0)) propgrid->SelectedObject=find_template(TEMPL_AREA);
		break;
	case 6: 
		if ((selection == nullptr) || (selection->Count == 0)) propgrid->SelectedObject=find_template(TEMPL_PATH);
		break;
	case 7: 
		if ((selection == nullptr) || (selection->Count == 0)) propgrid->SelectedObject=find_template(TEMPL_ATEXTBOX);
		break;
	case 8: 
		if ((selection == nullptr) || (selection->Count == 0)) propgrid->SelectedObject=find_template(TEMPL_ACIRCLE);
		break;
	case 9: 
		if ((selection == nullptr) || (selection->Count == 0)) propgrid->SelectedObject=find_template(TEMPL_ARECT);
		break;
	case 10: 
		if ((selection == nullptr) || (selection->Count == 0)) propgrid->SelectedObject=find_template(TEMPL_ALINE);
		break;
	case 11: 
		if ((selection == nullptr) || (selection->Count == 0)) propgrid->SelectedObject=find_template(TEMPL_AARROW);
		break;
	case 12: 
		if ((selection == nullptr) || (selection->Count == 0)) propgrid->SelectedObject=find_template(TEMPL_PROFILE);
		break;
	case 13: 
		if ((selection == nullptr) || (selection->Count == 0)) propgrid->SelectedObject=nullptr;
		break;
	}
}

System::Void image_add_win::pbox_MouseDown(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {
	int i,t;
	int ctrldown;

	if (!imd) return;
	if (imd->width*imd->height <= 0) return;

	mousepos=cs->t->todata(vector(((System::Windows::Forms::MouseEventArgs^) e)->X,((System::Windows::Forms::MouseEventArgs^) e)->Y));
	mousedownpos=mousepos;

	if (mousemode == 5) {
		vector pl;
		pl.x = plist[plist->Count - 1]->x;
		pl.y = plist[plist->Count - 1]->y;
		vector dist=mousepos-pl;
		if (dist.len > 0) {
			plist->Add(gcnew vector(mousepos));
		}
		switch (opmode) {
		case 3: // Circle 3P
		case 4: // Angle
			if (plist->Count >= 3) {
				// Last point
				mousemode=0;
				if (opmode == 3) {
					ccircle3p ^ln=gcnew ccircle3p(*plist[0],*plist[1],*plist[2],cs);
					ln->update_from_template(find_template(TEMPL_CIRCLE3P));
					mdata->Add(ln);
					selection=gcnew List<cmeasurement^>();
					selection->Add(ln);
					reselect();
				} else if (opmode == 4) {
					cangle ^ln=gcnew cangle(*plist[0],*plist[1],*plist[2],cs);
					ln->update_from_template(find_template(TEMPL_ANGLE));
					mdata->Add(ln);
					selection=gcnew List<cmeasurement^>();
					selection->Add(ln);
					reselect();
				}
			}
			break;
		case 5: // Area
		case 6: // Path
			if (dist.len == 0) {
				if (plist->Count >= 3) {
					mousemode = 0;
					carea ^ca = gcnew carea(plist,cs);
					ca->update_from_template(find_template((opmode==5)?TEMPL_AREA:TEMPL_PATH));
					mdata->Add(ca);
					selection=gcnew List<cmeasurement^>();
					selection->Add(ca);
					reselect();
				}
			}
			break;
		}
		redraw();
		return;
	}

	if ((Control::ModifierKeys & Keys::Control) == Keys::Control)
		ctrldown=1;
	else
		ctrldown=0;

	List<cmeasurement^> ^l=gcnew List<cmeasurement^>();
	// New selection or hit with click
	for (i=0;i<mdata->Count;i++) {
		List<cmeasurement^> ^lin;
		lin=mdata[i]->touching(mousepos);
		if (lin != nullptr) { l->AddRange(lin); }
	}
	if (l->Count > 0) {
		// Hit something
		if (ctrldown) {
			// Add clicked stuff to selection
			if (selection == nullptr)
				selection=l;
			else {
				for (int i=0;i<l->Count;i++) {
					int found=0;
					for (int j=0;j<selection->Count;j++) 
						if (selection[j]==l[i]) found=1;
					if (!found) 
						selection->Add(l[i]);
				}
			}
			return;
		}
		// start move
		// check whether click was in existing selection
		if (selection == nullptr) {
			// New select
			selection=l;
		} else {
			int gfound=0;
			for (int i=0;i<l->Count;i++) {
				for (int j=0;j<selection->Count;j++) 
					if (selection[j]==l[i]) gfound=1;
			}
			if (!gfound) {
				selection=l;
			}
		}
		for (int i=0;i<mdata->Count;i++) mdata[i]->storestate(); 
		reselect();
		mousemode=2; // drag
		mousemoved=0;
	} else {
		// Nothing clicked --> Start rectangle selection
		if (!ctrldown) selection=nullptr;
		reselect();

		if ((opmode == 0) || (opmode == 7) || (opmode == 8) || (opmode == 9)) {
			mousemode=1; // Rectangular select
			mousemoved=0;
			return;
		} else if ((opmode == 1) || (opmode == 10) || (opmode == 11) || (opmode == 12) || (opmode == 13)) {
			mousemode=3; // Line mode
			mousemoved=0;
		} else if (opmode == 2) {
			mousemode=4; // Point mode
			mousemoved=0;
		} else if ((opmode == 3) || (opmode == 4) || (opmode == 5) || (opmode == 6)) {
			mousemode=5; // Multip Point mode
			plist=gcnew List<vector^>();
			plist->Add(gcnew vector(mousepos));
			mousemoved=0;
		}
	}
}

System::Void image_add_win::pbox_MouseMove(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {
	double cx,cy;

	if (!imd) return;
	if (imd->width*imd->height <= 0) return;

	mousepos=cs->t->todata(vector(((System::Windows::Forms::MouseEventArgs^) e)->X,((System::Windows::Forms::MouseEventArgs^) e)->Y));
	mousemoved=1;
	coordlabel->Text=cs->f->format(cs->c->getreal(mousepos));
	
	if (mousemode == 1) {
		redraw();
	}
	if (mousemode == 2) {
		// dragging
		for (int i=0;i<selection->Count;i++) {
			selection[i]->moveto(mousedownpos,mousepos);
		}
		redraw();
		if (selection->Count==1)
			propgrid->Refresh();
	}
	if (mousemode == 3) {
		// Line Mode
		redraw();
	}
	if (mousemode == 4) {
		// Point Mode
		redraw();
	}
	if (mousemode == 5) {
		// multipoint Mode
		redraw();
	}

}


System::Void image_add_win::pbox_MouseUp(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {
	if (!imd) return;
	if (imd->width*imd->height <= 0) return;

	mousepos=cs->t->todata(vector(((System::Windows::Forms::MouseEventArgs^) e)->X,((System::Windows::Forms::MouseEventArgs^) e)->Y));

	if (mousemode==1) { // Rectangular select
		if (mousemoved==0) {
			// was just a click --> Nothing to do
			mousemode=0;
			reselect();
		} else {
			// real select
			if (opmode == 0) {
				List<cmeasurement^> ^l=gcnew List<cmeasurement^>();
				// New selection or hit with click
				for (int i=0;i<mdata->Count;i++) {
					List<cmeasurement^> ^lin;
					lin=mdata[i]->isinside(mousedownpos,mousepos);
					if (lin != nullptr) { l->AddRange(lin); }
				}
				if (l->Count > 0) {
					// add to selection
					if (selection==nullptr) 
						selection=l;
					else {
						for (int i=0;i<l->Count;i++) {
							int found=0;
							for (int j=0;j<selection->Count;j++) 
								if (selection[j]==l[i]) found=1;
							if (!found) 
								selection->Add(l[i]);
						}
					}
				}
				mousemode=0;
				reselect();
			} else if ((opmode == 7) || (opmode == 8)  || (opmode == 9)) {
				cannotation ^ln=gcnew cannotation(mousedownpos,mousepos,cs);
				switch (opmode) {
				case 7: ln->update_from_template(find_template(TEMPL_ATEXTBOX));break;
				case 8: ln->update_from_template(find_template(TEMPL_ACIRCLE));break;
				case 9: ln->update_from_template(find_template(TEMPL_ARECT));break;
				}
				mdata->Add(ln);
				selection=gcnew List<cmeasurement^>();
				selection->Add(ln);
				mousemode=0;
				reselect();
			}
		}
	}
	if (mousemode==2) { // drag
		for (int i=0;i<selection->Count;i++) {
			selection[i]->moveto(mousedownpos,mousepos);
		}
		mousemode=0;
		reselect();
	}
	if (mousemode==3) { // Line
		vector v=mousepos-mousedownpos;
		if (v.len > 0) {
			if (opmode == 1) {
				clinear ^ln=gcnew clinear(mousedownpos,mousepos,cs);
				ln->update_from_template(find_template(TEMPL_LINEAR));
				mdata->Add(ln);
				selection=gcnew List<cmeasurement^>();
				selection->Add(ln);
			}
			if (opmode == 12) {
				cprofile ^ln=gcnew cprofile(mousedownpos,mousepos,cs);
				ln->update_from_template(find_template(TEMPL_PROFILE));
				mdata->Add(ln);
				selection=gcnew List<cmeasurement^>();
				selection->Add(ln);
			}
			if (opmode == 13) {
				if (cs->cbar == nullptr) {
					ccolorbar ^ln=gcnew ccolorbar(mousedownpos,mousepos,cs);
					mdata->Add(ln);
					cs->cbar=ln;
					selection=gcnew List<cmeasurement^>();
					selection->Add(ln);
					setmode(0);
				}
			}
			if ((opmode == 10) || (opmode == 11)) {
				cannotation ^ln=gcnew cannotation(mousedownpos,mousepos,cs);
				switch (opmode) {
				case 10: ln->update_from_template(find_template(TEMPL_ALINE));break;
				case 11: ln->update_from_template(find_template(TEMPL_AARROW));break;
				}
				mdata->Add(ln);
				selection=gcnew List<cmeasurement^>();
				selection->Add(ln);
			}
		}
		mousemode=0;
		reselect();
	}
	if (mousemode==4) { // Point
		vector v=mousepos;
		cpoint ^ln=gcnew cpoint(mousepos,cs);
		ln->update_from_template(find_template(TEMPL_POINT));
		mdata->Add(ln);
		selection=gcnew List<cmeasurement^>();
		selection->Add(ln);
		mousemode=0;
		reselect();
	}
	if (mousemode==5) { // Multipoint
		vector pl;
		pl.x = plist[plist->Count - 1]->x;
		pl.y = plist[plist->Count - 1]->y;
		vector dist=mousepos-pl;
		if (dist.len > 0)
			plist->Add(gcnew vector(mousepos));
	}
}

void image_add_win::reselect(void) {

	if (!imd) return;
	if (imd->width*imd->height <= 0) return;

	for (int i=0;i<mdata->Count;i++) {
		mdata[i]->unselect();
	}
	if (selection!=nullptr) {
		for (int i=0;i<selection->Count;i++)
			selection[i]->select();
		//propgrid->SelectedObject=selection;
		if (selection->Count == 1) 
			propgrid->SelectedObject=selection[0];
		else
			propgrid->SelectedObject=nullptr;
	} else 
		propgrid->SelectedObject=nullptr;
	redraw();
}

Bitmap ^image_add_win::create_color_bm(Color c) {
	Bitmap ^b = gcnew Bitmap(16,16,PixelFormat::Format32bppArgb);
	Color frcol=Color::FromArgb( 64, 64, 64 );
	Color trcol=Color::FromArgb(0, 0, 0, 0 );
	int x,y;	
	for (x=0;x<16;x++) {
		for (y=0;y<16;y++) {
			if ((x == 0) || (x == 15) || (y == 0) || (y == 15)) {
				b->SetPixel( x, y, trcol );
			} else if ((x == 1) || (x == 14) || (y == 1) || (y == 14)) {
				b->SetPixel( x, y, frcol );
			} else {
				b->SetPixel( x, y, c );
			}
		}
	}
	return b;
}

ToolStripButton ^image_add_win::create_color_tsb(int r, int g, int b, String ^name) {
	Color c = Color::FromArgb(r,g,b);
	ToolStripButton ^tsb = gcnew ToolStripButton(create_color_bm(c));
	tsb->ToolTipText = name;
	tsb->Tag = c;
	tsb->Size = Drawing::Size(16,16);
	tsb->Click += gcnew System::EventHandler(this, &image_add_win::ColorSel_Click);
	tsb->Padding=Forms::Padding(0,0,0,0);
	tsb->Margin=Forms::Padding(0,0,0,0);
	// tsb->MinSize=Drawing::Size(16,16);
	return tsb;
}

System::Void image_add_win::ColorSel_Click(System::Object^  sender, System::EventArgs^  e) {
	ToolStripButton ^tsb=(ToolStripButton^)sender;
	quick_linecol((Color)tsb->Tag);
}

System::Void image_add_win::image_add_win_Load(System::Object^  sender, System::EventArgs^  e) {
	imd=nullptr; 
	this->MouseWheel += gcnew System::Windows::Forms::MouseEventHandler(this, &image_add_win::pbox_MouseWheel);

	

	this->pbox2 = (gcnew imagedisplaywin());
	this->pbox2->Cursor = System::Windows::Forms::Cursors::Cross;
	this->pbox2->Size = System::Drawing::Size(401, 361);
	this->pbox2->Location = System::Drawing::Point(0, 49);
	this->pbox2->Dock = System::Windows::Forms::DockStyle::Fill;
	this->pbox2->Name = L"pbox2";
	this->pbox2->SizeChanged += gcnew System::EventHandler(this, &image_add_win::pbox_SizeChanged);
	this->pbox2->Click += gcnew System::EventHandler(this, &image_add_win::pbox_Click);
	this->pbox2->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &image_add_win::pbox_Paint);
	this->pbox2->MouseDown += gcnew System::Windows::Forms::MouseEventHandler(this, &image_add_win::pbox_MouseDown);
	this->pbox2->MouseEnter += gcnew System::EventHandler(this, &image_add_win::pbox_MouseEnter);
	this->pbox2->MouseLeave += gcnew System::EventHandler(this, &image_add_win::pbox_MouseLeave);
	this->pbox2->MouseMove += gcnew System::Windows::Forms::MouseEventHandler(this, &image_add_win::pbox_MouseMove);
	this->pbox2->MouseUp += gcnew System::Windows::Forms::MouseEventHandler(this, &image_add_win::pbox_MouseUp);
	this->pbox2->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &image_add_win::image_add_win_KeyPress);
	this->pbox2->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &image_add_win::image_add_win_KeyDown);

	//this->Controls->Add(this->pbox2);
	this->splitter->Panel1->Controls->Add(this->pbox2);

	this->statusStrip1->ResumeLayout(false);
	this->statusStrip1->PerformLayout();
	this->toolStrip1->ResumeLayout(false);
	this->toolStrip1->PerformLayout();
	this->menuStrip1->ResumeLayout(false);
	this->menuStrip1->PerformLayout();
	this->splitter->Panel2->ResumeLayout(false);
	this->splitter->Panel2->PerformLayout();
	(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->splitter))->EndInit();
	this->splitter->ResumeLayout(false);
	this->ResumeLayout(false);
	this->PerformLayout();

	
	this->toolStrip2->Items->Add(create_color_tsb(255,0,0,"Red"));
	this->toolStrip2->Items->Add(create_color_tsb(255,128,0,"Orange"));
	this->toolStrip2->Items->Add(create_color_tsb(255,255,0,"Yellow"));
	this->toolStrip2->Items->Add(create_color_tsb(128,255,0,"LightGreen"));
	this->toolStrip2->Items->Add(create_color_tsb(0,255,0,"Green"));
	this->toolStrip2->Items->Add(create_color_tsb(0,255,128,"BlueGreen"));
	this->toolStrip2->Items->Add(create_color_tsb(0,255,255,"Cyan"));
	this->toolStrip2->Items->Add(create_color_tsb(0,128,255,"LightBlue"));
	this->toolStrip2->Items->Add(create_color_tsb(0,0,255,"Blue"));
	this->toolStrip2->Items->Add(create_color_tsb(128,0,255,"DarkPink"));
	this->toolStrip2->Items->Add(create_color_tsb(255,0,255,"Pink"));
	this->toolStrip2->Items->Add(create_color_tsb(255,0,128,"BrightPink"));
	this->toolStrip2->Items->Add(create_color_tsb(0,0,0,"Black"));
	this->toolStrip2->Items->Add(create_color_tsb(128,128,128,"Gray"));
	this->toolStrip2->Items->Add(create_color_tsb(255,255,255,"White"));

	ctxt=BufferedGraphicsManager::Current;
	ctxt->MaximumBuffer = System::Drawing::Size(pbox2->Size.Width+1, pbox2->Size.Height+1);

	//this->pbox2->TabIndex = 5;
	//this->pbox2->TabStop = false;

	pbox_real_g=pbox2->CreateGraphics();
	pbox_grafx = ctxt->Allocate(pbox_real_g, Rectangle( 0, 0, pbox2->Size.Width, pbox2->Size.Height ));
	pbox_g=pbox_grafx->Graphics;
	pbox_g->SmoothingMode = System::Drawing::Drawing2D::SmoothingMode::AntiAlias;

	cs=gcnew csys(gcnew formatter(), gcnew transformer());
	coordinate ^coord = gcnew coordinate(cs,1);
	cs->c=coord;
	coord->readonly=1;

	mdata=gcnew List<cmeasurement^>();
	mdata->Add(coord);

	cframe ^frame=gcnew cframe(cs);
	cs->frm=frame;
	mdata->Add(frame);
	frame->readonly=1;

	templates=nullptr;
	create_templates();

	selection=nullptr;
	mousemode=0;
	opmode=0;

	ib_coord->Checked=true;
	//showref->Checked=true;showrefpoints=1;

	//this->KeyPreview = true;




	// Work with Command Line
	if (cmdline->Length > 0) {
		open_image(cmdline[0]);
	}
}

void image_add_win::create_templates() {
	cpoint ^point_template;
	clinear ^linear_template;
	ccircle3p ^circle3p_template;
	cangle ^angle_template;
	carea ^area_template;
	carea ^path_template;
	cprofile ^profile_template;
	cannotation ^atextbox_template;
	cannotation ^acircle_template;
	cannotation ^arect_template;
	cannotation ^aline_template;
	cannotation ^aarrow_template;

	point_template = gcnew cpoint(vector(0,0),cs);
	point_template->id=TEMPL_POINT;

	linear_template = gcnew clinear(vector(0,0), vector(1,0), cs);
	linear_template->id=TEMPL_LINEAR;

	circle3p_template = gcnew ccircle3p(vector(0,0), vector(1,0), vector(1,1), cs);
	circle3p_template->id=TEMPL_CIRCLE3P;
	
	angle_template = gcnew cangle(vector(0,0), vector(1,0), vector(1,1), cs);
	angle_template->id=TEMPL_ANGLE;
	
	area_template = gcnew carea(gcnew List<vector^>(), cs);
	area_template->id=TEMPL_AREA;
		
	path_template = gcnew carea(gcnew List<vector^>(), cs);
	path_template->fill=0;
	path_template->showarea=0;
	path_template->showlength=1;
	path_template->closed=0;
	path_template->id=TEMPL_PATH;

	profile_template = gcnew cprofile(vector(0,0), vector(1,0), cs);
	profile_template->id=TEMPL_PROFILE;
	
	atextbox_template = gcnew cannotation(vector(0,0),vector(1,1), cs);
	atextbox_template->bgfill=1;
	atextbox_template->lm=LineMode::None;
	atextbox_template->shp=Shape::Rectangle;
	atextbox_template->id=TEMPL_ATEXTBOX;
	
	acircle_template = gcnew cannotation(vector(0,0),vector(1,1), cs);
	acircle_template->bgfill=0;
	acircle_template->lm=LineMode::Solid;
	acircle_template->shp=Shape::Ellipse;
	acircle_template->id=TEMPL_ACIRCLE;
	
	arect_template = gcnew cannotation(vector(0,0),vector(1,1), cs);
	arect_template->bgfill=0;
	arect_template->lm=LineMode::Solid;
	arect_template->shp=Shape::Rectangle;
	arect_template->id=TEMPL_ARECT;
	
	aline_template = gcnew cannotation(vector(0,0),vector(1,1), cs);
	aline_template->bgfill=0;
	aline_template->lm=LineMode::Solid;
	aline_template->shp=Shape::Line;
	aline_template->textonline=1;
	aline_template->fntfrmuse=1;
	aline_template->fntcol=Color::FromArgb(255,255,255);
	aline_template->id=TEMPL_ALINE;
	
	aarrow_template = gcnew cannotation(vector(0,0),vector(1,1), cs);
	aarrow_template->bgfill=0;
	aarrow_template->lm=LineMode::Solid;
	aarrow_template->shp=Shape::Arrow;
	aarrow_template->textonline=0;
	aarrow_template->fntfrmuse=1;
	aarrow_template->fntcol=Color::FromArgb(255,255,255);
	aarrow_template->id=TEMPL_AARROW;

	if (templates != nullptr) {
		while (templates->Count > 0) {
			delete templates[0];
			templates->RemoveAt(0);
		}
		delete templates;
		templates=nullptr;
	}
	templates=gcnew List<cmeasurement^>();
	templates->Add(point_template);
	templates->Add(linear_template);
	templates->Add(circle3p_template);
	templates->Add(angle_template);
	templates->Add(area_template);
	templates->Add(path_template);
	templates->Add(profile_template);
	templates->Add(atextbox_template);
	templates->Add(acircle_template);
	templates->Add(arect_template);
	templates->Add(aline_template);
	templates->Add(aarrow_template);
}

cmeasurement ^image_add_win::find_template(int id) {
	if (templates==nullptr) return nullptr;
	for (int i=0;i<templates->Count;i++) {
		if (templates[i]->id == id) return templates[i];
	}
	return nullptr;
}

System::Void image_add_win::image_add_win_KeyPress(System::Object^  sender, System::Windows::Forms::KeyPressEventArgs^  e) {
	if (!imd) return;
	if (imd->width*imd->height <= 0) return;
	if (e->KeyChar == (char)27) {
		if (mousemode != 0) {
			if (mousemode == 2) {
				for (int i=0;i<mdata->Count;i++) mdata[i]->restorestate(); 
			}
			mousemode=0;
			redraw();
		}
	}
}

System::Void image_add_win::image_add_win_KeyDown(System::Object^  sender, System::Windows::Forms::KeyEventArgs^  e) {
	if (e->KeyCode == Keys::Delete) {
		ib_delete_Click(nullptr,nullptr);
	}
}

System::Void image_add_win::ib_delete_Click(System::Object^  sender, System::EventArgs^  e) {
	if (selection != nullptr) {
		int i;
		propgrid->SelectedObject=nullptr;
		i=0;
		while (i<mdata->Count) {
			int found=0;
			for (int j=0;j<selection->Count;j++) {
				if (selection[j]==mdata[i]) 
					found=1;
			}
			if (found && mdata[i]->readonly == 0) {
				mdata->RemoveAt(i);
			} else i++;					
		}
		selection=nullptr;
		reselect();
	}
}

System::Void image_add_win::ib_colorbar_Click(System::Object^  sender, System::EventArgs^  e) {
	if (cs->cbar) {
		if (cs->cbar->show) {
			cs->cbar->show=0;
			ib_colorbar->Checked=false;
			redraw();
		} else {
			cs->cbar->show=1;
			ib_colorbar->Checked=true;
			redraw();
		}
	} else {
		ib_colorbar->Checked=true;
		setmode(13);
	}
}

System::Void image_add_win::ib_coord_Click(System::Object^  sender, System::EventArgs^  e) {
	if (cs->c->drawcoord) { cs->c->drawcoord=0; ib_coord->Checked=false; } else 
		{ cs->c->drawcoord=1; ib_coord->Checked=true; }
	redraw();
}

System::Void image_add_win::pbox_MouseWheel(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {
	Point rpnt=this->PointToScreen(Point(e->X,e->Y));
	rpnt=this->pbox2->PointToClient(rpnt);

	if (!imd) return;
	if (imd->width*imd->height <= 0) return;

	if ((Control::ModifierKeys & Keys::Control) == Keys::Control) { // Control
		vector os=vector(rpnt.X,rpnt.Y)-cs->t->screen/2-cs->t->center;
		os/=cs->t->scale;
		if (e->Delta > 0) 
			cs->t->scale*=1.2;
		else
			cs->t->scale/=1.2;
		os*=cs->t->scale;
		cs->t->center=vector(rpnt.X,rpnt.Y)-cs->t->screen/2-os;
		redraw();
	} else if ((Control::ModifierKeys & Keys::Shift) == Keys::Shift) { // Shift
		if (e->Delta > 0) 
			cs->t->center.x+=40;
		else
			cs->t->center.x-=40;
		redraw();
	} else { // no modifer
		if (e->Delta > 0) 
			cs->t->center.y+=40;
		else
			cs->t->center.y-=40;
		redraw();
	}


}

void image_add_win::quick_linecol(Color c) {
	if (selection != nullptr) {
		for (int i=0;i<selection->Count;i++) {
			selection[i]->linecol=c;
		}
	}
	redraw();
}

// EOF