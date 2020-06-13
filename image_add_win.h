#pragma once

#include "stdafx.h"
#include "vectormatrix.h"
#include "transformer.h"
#include "imagedata.h"
#include "measurements.h"
#include "imagedisplaywin.h"
#include "about_win.h"

#define IA_VERSION "0.0.4 (8/24/2014)"

namespace image_add {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::Drawing::Drawing2D;
	using namespace System::Windows; 
	using namespace System::Collections::Generic;
	/// <summary>

	/// Summary for Form1
	/// </summary>
	public ref class image_add_win : public System::Windows::Forms::Form
	{
	public:
		imagedata ^imd;


		Graphics ^pbox_g;
		Graphics ^pbox_real_g;
		BufferedGraphicsContext ^ctxt;
	    BufferedGraphics ^pbox_grafx;

		//Graphics ^zoombox_g;
		//Graphics ^zoombox_real_g;
	    //BufferedGraphics ^zoombox_grafx;
		
		List<cmeasurement^>^ mdata;
		csys ^cs;

		List<cmeasurement^> ^templates;
#define TEMPL_POINT		0x1000
#define TEMPL_LINEAR	0x1001
#define TEMPL_CIRCLE3P	0x1002
#define TEMPL_ANGLE		0x1003
#define TEMPL_AREA		0x1004
#define TEMPL_PATH		0x1005
#define TEMPL_PROFILE	0x1006
#define TEMPL_ATEXTBOX	0x2000
#define TEMPL_ACIRCLE	0x2001
#define TEMPL_ARECT		0x2002
#define TEMPL_ALINE		0x2003
#define TEMPL_AARROW	0x2004
//		cpoint ^point_template;
//		clinear ^linear_template;
//		ccircle3p ^circle3p_template;
//		cangle ^angle_template;
//		carea ^area_template;
//		cannotation ^annotation_template;

		List<cmeasurement^>^ selection;
		int mousemode;	// 0: Nothing
						// 1: rectangular select
						// 2: dragging
		vector mousedownpos;
		vector mousepos;
		int mousemoved;
		int opmode;
		int submode;
		List<vector^>^ plist;

		array<System::String ^> ^cmdline;

		int showrefpoints;

		imagedisplaywin ^pbox2;

	

	public: 







	private: System::Windows::Forms::ToolStripStatusLabel^  coordlabel;
	private: System::Windows::Forms::ToolStripStatusLabel^  statuslabel;












	private: System::Windows::Forms::PropertyGrid^  propgrid;
	private: System::Windows::Forms::ToolStripButton^  ib_save;

	private: System::Windows::Forms::ToolStripSeparator^  toolStripSeparator1;
	private: System::Windows::Forms::ToolStripButton^  ib_pointer;
	private: System::Windows::Forms::ToolStripButton^  ib_line;
	private: System::Windows::Forms::ToolStripSeparator^  toolStripSeparator2;
	private: System::Windows::Forms::ToolStripButton^  ib_delete;
	private: System::Windows::Forms::ToolStripButton^  ib_paste;
	private: System::Windows::Forms::ToolStripButton^  ib_point;
	private: System::Windows::Forms::SplitContainer^  splitter;

	private: System::Windows::Forms::ToolStripSeparator^  toolStripSeparator3;
	private: System::Windows::Forms::ToolStripButton^  ib_coord;
	private: System::Windows::Forms::ToolStripSeparator^  toolStripSeparator4;
	private: System::Windows::Forms::ToolStripButton^  ib_zoomout;
	private: System::Windows::Forms::ToolStripMenuItem^  toolStripMenuItem2;
	private: System::Windows::Forms::ToolStripMenuItem^  aboutToolStripMenuItem;
	private: System::Windows::Forms::ToolStripStatusLabel^  xmlname;
	private: System::Windows::Forms::ToolStripButton^  ib_circle3p;



	private: System::Windows::Forms::ToolStripSeparator^  toolStripSeparator5;
	private: System::Windows::Forms::ToolStripButton^  ib_copyclip;
	private: System::Windows::Forms::ToolStripMenuItem^  toolStripMenuItem4;
	private: System::Windows::Forms::ToolStripMenuItem^  toolStripMenuItem3;
	private: System::Windows::Forms::ToolStripMenuItem^  toolStripMenuItem1;
	private: System::Windows::Forms::ToolStripMenuItem^  copyToClipboardToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  menu_save_meta_only;
	private: System::Windows::Forms::ToolStripButton^  ib_angle;
	private: System::Windows::Forms::ToolStripButton^  ib_area;
	private: System::Windows::Forms::ToolStripButton^  ib_path;
	private: System::Windows::Forms::ToolStripSeparator^  toolStripSeparator6;
	private: System::Windows::Forms::ToolStripButton^  ib_annot_box;
	private: System::Windows::Forms::ToolStripButton^  ib_annot_circle;
	private: System::Windows::Forms::ToolStripButton^  ib_annot_rect;
	private: System::Windows::Forms::ToolStripButton^  ib_annot_line;
	private: System::Windows::Forms::ToolStripButton^  ib_annot_arrow;
private: System::Windows::Forms::ToolStrip^  toolStrip2;









private: System::Windows::Forms::ToolStripButton^  ib_profile;
private: System::Windows::Forms::ToolStripButton^  ib_copyCSV;
private: System::Windows::Forms::ToolStripButton^  ib_copyclip_emf;
private: System::Windows::Forms::ToolStripButton^  ib_colorbar;


	private: System::Windows::Forms::ToolStripButton^  ob_export;









	public: 
		image_add_win(array<System::String ^> ^args) : System::Windows::Forms::Form()
		{
			cmdline=args;
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~image_add_win()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::StatusStrip^  statusStrip1;
	protected: 
	private: System::Windows::Forms::ToolStrip^  toolStrip1;
	private: System::Windows::Forms::ToolStripButton^  ib_open;


	private: System::Windows::Forms::MenuStrip^  menuStrip1;
	private: System::Windows::Forms::ToolStripMenuItem^  fileToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  menu_open_image;
	private: System::Windows::Forms::ToolStripSeparator^  exitToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  menu_exit;
	private: System::Windows::Forms::ToolStripMenuItem^  calibrationToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  menu_cal_load;
	private: System::Windows::Forms::ToolStripMenuItem^  menu_cal_save;




	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(image_add_win::typeid));
			this->statusStrip1 = (gcnew System::Windows::Forms::StatusStrip());
			this->statuslabel = (gcnew System::Windows::Forms::ToolStripStatusLabel());
			this->xmlname = (gcnew System::Windows::Forms::ToolStripStatusLabel());
			this->coordlabel = (gcnew System::Windows::Forms::ToolStripStatusLabel());
			this->toolStrip1 = (gcnew System::Windows::Forms::ToolStrip());
			this->ib_open = (gcnew System::Windows::Forms::ToolStripButton());
			this->ib_save = (gcnew System::Windows::Forms::ToolStripButton());
			this->ob_export = (gcnew System::Windows::Forms::ToolStripButton());
			this->toolStripSeparator5 = (gcnew System::Windows::Forms::ToolStripSeparator());
			this->ib_paste = (gcnew System::Windows::Forms::ToolStripButton());
			this->ib_copyclip = (gcnew System::Windows::Forms::ToolStripButton());
			this->ib_copyclip_emf = (gcnew System::Windows::Forms::ToolStripButton());
			this->ib_copyCSV = (gcnew System::Windows::Forms::ToolStripButton());
			this->toolStripSeparator1 = (gcnew System::Windows::Forms::ToolStripSeparator());
			this->ib_pointer = (gcnew System::Windows::Forms::ToolStripButton());
			this->ib_line = (gcnew System::Windows::Forms::ToolStripButton());
			this->ib_point = (gcnew System::Windows::Forms::ToolStripButton());
			this->ib_circle3p = (gcnew System::Windows::Forms::ToolStripButton());
			this->ib_angle = (gcnew System::Windows::Forms::ToolStripButton());
			this->ib_area = (gcnew System::Windows::Forms::ToolStripButton());
			this->ib_path = (gcnew System::Windows::Forms::ToolStripButton());
			this->ib_profile = (gcnew System::Windows::Forms::ToolStripButton());
			this->toolStripSeparator6 = (gcnew System::Windows::Forms::ToolStripSeparator());
			this->ib_annot_box = (gcnew System::Windows::Forms::ToolStripButton());
			this->ib_annot_circle = (gcnew System::Windows::Forms::ToolStripButton());
			this->ib_annot_rect = (gcnew System::Windows::Forms::ToolStripButton());
			this->ib_annot_line = (gcnew System::Windows::Forms::ToolStripButton());
			this->ib_annot_arrow = (gcnew System::Windows::Forms::ToolStripButton());
			this->ib_colorbar = (gcnew System::Windows::Forms::ToolStripButton());
			this->toolStripSeparator2 = (gcnew System::Windows::Forms::ToolStripSeparator());
			this->ib_delete = (gcnew System::Windows::Forms::ToolStripButton());
			this->toolStripSeparator3 = (gcnew System::Windows::Forms::ToolStripSeparator());
			this->ib_coord = (gcnew System::Windows::Forms::ToolStripButton());
			this->toolStripSeparator4 = (gcnew System::Windows::Forms::ToolStripSeparator());
			this->ib_zoomout = (gcnew System::Windows::Forms::ToolStripButton());
			this->menuStrip1 = (gcnew System::Windows::Forms::MenuStrip());
			this->fileToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->menu_open_image = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->menu_save_meta_only = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->toolStripMenuItem2 = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->toolStripMenuItem4 = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->exitToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripSeparator());
			this->menu_exit = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->toolStripMenuItem3 = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->toolStripMenuItem1 = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->copyToClipboardToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->calibrationToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->menu_cal_load = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->menu_cal_save = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->aboutToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->propgrid = (gcnew System::Windows::Forms::PropertyGrid());
			this->splitter = (gcnew System::Windows::Forms::SplitContainer());
			this->toolStrip2 = (gcnew System::Windows::Forms::ToolStrip());
			this->statusStrip1->SuspendLayout();
			this->toolStrip1->SuspendLayout();
			this->menuStrip1->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->splitter))->BeginInit();
			this->splitter->Panel2->SuspendLayout();
			this->splitter->SuspendLayout();
			this->SuspendLayout();
			// 
			// statusStrip1
			// 
			this->statusStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(3) {this->statuslabel, this->xmlname, 
				this->coordlabel});
			this->statusStrip1->Location = System::Drawing::Point(0, 533);
			this->statusStrip1->Name = L"statusStrip1";
			this->statusStrip1->Padding = System::Windows::Forms::Padding(1, 0, 19, 0);
			this->statusStrip1->Size = System::Drawing::Size(781, 25);
			this->statusStrip1->TabIndex = 0;
			this->statusStrip1->Text = L"status";
			// 
			// statuslabel
			// 
			this->statuslabel->Name = L"statuslabel";
			this->statuslabel->Size = System::Drawing::Size(0, 20);
			// 
			// xmlname
			// 
			this->xmlname->Name = L"xmlname";
			this->xmlname->Size = System::Drawing::Size(0, 20);
			// 
			// coordlabel
			// 
			this->coordlabel->Name = L"coordlabel";
			this->coordlabel->Size = System::Drawing::Size(761, 20);
			this->coordlabel->Spring = true;
			this->coordlabel->Text = L"0|0";
			this->coordlabel->TextAlign = System::Drawing::ContentAlignment::MiddleRight;
			// 
			// toolStrip1
			// 
			this->toolStrip1->GripStyle = System::Windows::Forms::ToolStripGripStyle::Hidden;
			this->toolStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(30) {this->ib_open, this->ib_save, 
				this->ob_export, this->toolStripSeparator5, this->ib_paste, this->ib_copyclip, this->ib_copyclip_emf, this->ib_copyCSV, this->toolStripSeparator1, 
				this->ib_pointer, this->ib_line, this->ib_point, this->ib_circle3p, this->ib_angle, this->ib_area, this->ib_path, this->ib_profile, 
				this->toolStripSeparator6, this->ib_annot_box, this->ib_annot_circle, this->ib_annot_rect, this->ib_annot_line, this->ib_annot_arrow, 
				this->toolStripSeparator2, this->ib_delete, this->toolStripSeparator3, this->ib_coord, this->ib_colorbar, this->toolStripSeparator4, 
				this->ib_zoomout});
			this->toolStrip1->Location = System::Drawing::Point(0, 28);
			this->toolStrip1->Name = L"toolStrip1";
			this->toolStrip1->Size = System::Drawing::Size(781, 25);
			this->toolStrip1->TabIndex = 2;
			this->toolStrip1->Text = L"mainmenu";
			// 
			// ib_open
			// 
			this->ib_open->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
			this->ib_open->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"ib_open.Image")));
			this->ib_open->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->ib_open->Name = L"ib_open";
			this->ib_open->Size = System::Drawing::Size(23, 22);
			this->ib_open->Text = L"Open Image File";
			this->ib_open->ToolTipText = L"Open Image File";
			this->ib_open->Click += gcnew System::EventHandler(this, &image_add_win::ib_open_Click);
			// 
			// ib_save
			// 
			this->ib_save->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
			this->ib_save->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"ib_save.Image")));
			this->ib_save->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->ib_save->Name = L"ib_save";
			this->ib_save->Size = System::Drawing::Size(23, 22);
			this->ib_save->Text = L"Save MetaData File";
			this->ib_save->ToolTipText = L"Save Settings";
			this->ib_save->Click += gcnew System::EventHandler(this, &image_add_win::ib_save_Click);
			// 
			// ob_export
			// 
			this->ob_export->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
			this->ob_export->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"ob_export.Image")));
			this->ob_export->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->ob_export->Name = L"ob_export";
			this->ob_export->Size = System::Drawing::Size(23, 22);
			this->ob_export->Text = L"Export Image";
			this->ob_export->ToolTipText = L"Export to Image File";
			this->ob_export->Click += gcnew System::EventHandler(this, &image_add_win::ib_export_Click);
			// 
			// toolStripSeparator5
			// 
			this->toolStripSeparator5->Name = L"toolStripSeparator5";
			this->toolStripSeparator5->Size = System::Drawing::Size(6, 25);
			// 
			// ib_paste
			// 
			this->ib_paste->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
			this->ib_paste->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"ib_paste.Image")));
			this->ib_paste->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->ib_paste->Name = L"ib_paste";
			this->ib_paste->Size = System::Drawing::Size(23, 22);
			this->ib_paste->Text = L"Paste from Clipboard";
			this->ib_paste->ToolTipText = L"Paste from Clipboard";
			this->ib_paste->Click += gcnew System::EventHandler(this, &image_add_win::ib_paste_Click);
			// 
			// ib_copyclip
			// 
			this->ib_copyclip->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
			this->ib_copyclip->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"ib_copyclip.Image")));
			this->ib_copyclip->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->ib_copyclip->Name = L"ib_copyclip";
			this->ib_copyclip->Size = System::Drawing::Size(23, 22);
			this->ib_copyclip->Text = L"Copy To Clipboard";
			this->ib_copyclip->ToolTipText = L"Copy To Clipboard";
			this->ib_copyclip->Click += gcnew System::EventHandler(this, &image_add_win::ib_copyclip_Click);
			// 
			// ib_copyclip_emf
			// 
			this->ib_copyclip_emf->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
			this->ib_copyclip_emf->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"ib_copyclip_emf.Image")));
			this->ib_copyclip_emf->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->ib_copyclip_emf->Name = L"ib_copyclip_emf";
			this->ib_copyclip_emf->Size = System::Drawing::Size(23, 22);
			this->ib_copyclip_emf->Text = L"Copy To Clipboard as EMF";
			this->ib_copyclip_emf->ToolTipText = L"Copy To Clipboard as EMF";
			this->ib_copyclip_emf->Click += gcnew System::EventHandler(this, &image_add_win::ib_copyclip_emf_Click);
			// 
			// ib_copyCSV
			// 
			this->ib_copyCSV->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
			this->ib_copyCSV->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"ib_copyCSV.Image")));
			this->ib_copyCSV->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->ib_copyCSV->Name = L"ib_copyCSV";
			this->ib_copyCSV->Size = System::Drawing::Size(23, 22);
			this->ib_copyCSV->Text = L"Copy to CSV";
			this->ib_copyCSV->Click += gcnew System::EventHandler(this, &image_add_win::ib_copyCSV_Click);
			// 
			// toolStripSeparator1
			// 
			this->toolStripSeparator1->Name = L"toolStripSeparator1";
			this->toolStripSeparator1->Size = System::Drawing::Size(6, 25);
			// 
			// ib_pointer
			// 
			this->ib_pointer->Checked = true;
			this->ib_pointer->CheckState = System::Windows::Forms::CheckState::Checked;
			this->ib_pointer->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
			this->ib_pointer->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"ib_pointer.Image")));
			this->ib_pointer->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->ib_pointer->Name = L"ib_pointer";
			this->ib_pointer->Size = System::Drawing::Size(23, 22);
			this->ib_pointer->Text = L"Select/Modify";
			this->ib_pointer->ToolTipText = L"Select";
			this->ib_pointer->Click += gcnew System::EventHandler(this, &image_add_win::ib_pointer_Click);
			// 
			// ib_line
			// 
			this->ib_line->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
			this->ib_line->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"ib_line.Image")));
			this->ib_line->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->ib_line->Name = L"ib_line";
			this->ib_line->Size = System::Drawing::Size(23, 22);
			this->ib_line->Text = L"Line";
			this->ib_line->ToolTipText = L"Measure Line";
			this->ib_line->Click += gcnew System::EventHandler(this, &image_add_win::ib_line_Click);
			// 
			// ib_point
			// 
			this->ib_point->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
			this->ib_point->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"ib_point.Image")));
			this->ib_point->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->ib_point->Name = L"ib_point";
			this->ib_point->Size = System::Drawing::Size(23, 22);
			this->ib_point->Text = L"Point";
			this->ib_point->Click += gcnew System::EventHandler(this, &image_add_win::ib_point_Click);
			// 
			// ib_circle3p
			// 
			this->ib_circle3p->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
			this->ib_circle3p->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"ib_circle3p.Image")));
			this->ib_circle3p->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->ib_circle3p->Name = L"ib_circle3p";
			this->ib_circle3p->Size = System::Drawing::Size(23, 22);
			this->ib_circle3p->Text = L"Three Point Circle";
			this->ib_circle3p->Click += gcnew System::EventHandler(this, &image_add_win::ib_circle3p_Click);
			// 
			// ib_angle
			// 
			this->ib_angle->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
			this->ib_angle->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"ib_angle.Image")));
			this->ib_angle->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->ib_angle->Name = L"ib_angle";
			this->ib_angle->Size = System::Drawing::Size(23, 22);
			this->ib_angle->Text = L"Angle";
			this->ib_angle->ToolTipText = L"Angle";
			this->ib_angle->Click += gcnew System::EventHandler(this, &image_add_win::ib_angle_Click);
			// 
			// ib_area
			// 
			this->ib_area->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
			this->ib_area->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"ib_area.Image")));
			this->ib_area->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->ib_area->Name = L"ib_area";
			this->ib_area->Size = System::Drawing::Size(23, 22);
			this->ib_area->Text = L"Area";
			this->ib_area->Click += gcnew System::EventHandler(this, &image_add_win::ib_area_Click);
			// 
			// ib_path
			// 
			this->ib_path->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
			this->ib_path->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"ib_path.Image")));
			this->ib_path->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->ib_path->Name = L"ib_path";
			this->ib_path->Size = System::Drawing::Size(23, 22);
			this->ib_path->Text = L"Path";
			this->ib_path->Click += gcnew System::EventHandler(this, &image_add_win::ib_path_Click);
			// 
			// ib_profile
			// 
			this->ib_profile->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
			this->ib_profile->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"ib_profile.Image")));
			this->ib_profile->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->ib_profile->Name = L"ib_profile";
			this->ib_profile->Size = System::Drawing::Size(23, 22);
			this->ib_profile->Text = L"Profile";
			this->ib_profile->Click += gcnew System::EventHandler(this, &image_add_win::ib_profile_Click);
			// 
			// toolStripSeparator6
			// 
			this->toolStripSeparator6->Name = L"toolStripSeparator6";
			this->toolStripSeparator6->Size = System::Drawing::Size(6, 25);
			// 
			// ib_annot_box
			// 
			this->ib_annot_box->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
			this->ib_annot_box->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"ib_annot_box.Image")));
			this->ib_annot_box->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->ib_annot_box->Name = L"ib_annot_box";
			this->ib_annot_box->Size = System::Drawing::Size(23, 22);
			this->ib_annot_box->Text = L"Annotation Box";
			this->ib_annot_box->Click += gcnew System::EventHandler(this, &image_add_win::ib_annot_box_Click);
			// 
			// ib_annot_circle
			// 
			this->ib_annot_circle->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
			this->ib_annot_circle->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"ib_annot_circle.Image")));
			this->ib_annot_circle->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->ib_annot_circle->Name = L"ib_annot_circle";
			this->ib_annot_circle->Size = System::Drawing::Size(23, 22);
			this->ib_annot_circle->Text = L"Annotation Circle";
			this->ib_annot_circle->Click += gcnew System::EventHandler(this, &image_add_win::ib_annot_circle_Click);
			// 
			// ib_annot_rect
			// 
			this->ib_annot_rect->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
			this->ib_annot_rect->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"ib_annot_rect.Image")));
			this->ib_annot_rect->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->ib_annot_rect->Name = L"ib_annot_rect";
			this->ib_annot_rect->Size = System::Drawing::Size(23, 22);
			this->ib_annot_rect->Text = L"Annotation Rectangle";
			this->ib_annot_rect->Click += gcnew System::EventHandler(this, &image_add_win::ib_annot_rect_Click);
			// 
			// ib_annot_line
			// 
			this->ib_annot_line->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
			this->ib_annot_line->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"ib_annot_line.Image")));
			this->ib_annot_line->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->ib_annot_line->Name = L"ib_annot_line";
			this->ib_annot_line->Size = System::Drawing::Size(23, 22);
			this->ib_annot_line->Text = L"Annotation Line";
			this->ib_annot_line->Click += gcnew System::EventHandler(this, &image_add_win::ib_annot_line_Click);
			// 
			// ib_annot_arrow
			// 
			this->ib_annot_arrow->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
			this->ib_annot_arrow->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"ib_annot_arrow.Image")));
			this->ib_annot_arrow->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->ib_annot_arrow->Name = L"ib_annot_arrow";
			this->ib_annot_arrow->Size = System::Drawing::Size(23, 22);
			this->ib_annot_arrow->Text = L"Annotation Arrow";
			this->ib_annot_arrow->Click += gcnew System::EventHandler(this, &image_add_win::ib_annot_arrow_Click);
			// 
			// ib_colorbar
			// 
			this->ib_colorbar->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
			this->ib_colorbar->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"ib_colorbar.Image")));
			this->ib_colorbar->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->ib_colorbar->Name = L"ib_colorbar";
			this->ib_colorbar->Size = System::Drawing::Size(23, 22);
			this->ib_colorbar->Text = L"ColorBar Sampler";
			this->ib_colorbar->Click += gcnew System::EventHandler(this, &image_add_win::ib_colorbar_Click);
			// 
			// toolStripSeparator2
			// 
			this->toolStripSeparator2->Name = L"toolStripSeparator2";
			this->toolStripSeparator2->Size = System::Drawing::Size(6, 25);
			// 
			// ib_delete
			// 
			this->ib_delete->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
			this->ib_delete->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"ib_delete.Image")));
			this->ib_delete->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->ib_delete->Name = L"ib_delete";
			this->ib_delete->Size = System::Drawing::Size(23, 22);
			this->ib_delete->Text = L"Delete Item";
			this->ib_delete->ToolTipText = L"Delete";
			this->ib_delete->Click += gcnew System::EventHandler(this, &image_add_win::ib_delete_Click);
			// 
			// toolStripSeparator3
			// 
			this->toolStripSeparator3->Name = L"toolStripSeparator3";
			this->toolStripSeparator3->Size = System::Drawing::Size(6, 25);
			// 
			// ib_coord
			// 
			this->ib_coord->Checked = true;
			this->ib_coord->CheckState = System::Windows::Forms::CheckState::Checked;
			this->ib_coord->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
			this->ib_coord->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"ib_coord.Image")));
			this->ib_coord->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->ib_coord->Name = L"ib_coord";
			this->ib_coord->Size = System::Drawing::Size(23, 22);
			this->ib_coord->Text = L"Display/Hide Axes";
			this->ib_coord->ToolTipText = L"Display/hide Coordinate System";
			this->ib_coord->Click += gcnew System::EventHandler(this, &image_add_win::ib_coord_Click);
			// 
			// toolStripSeparator4
			// 
			this->toolStripSeparator4->Name = L"toolStripSeparator4";
			this->toolStripSeparator4->Size = System::Drawing::Size(6, 25);
			// 
			// ib_zoomout
			// 
			this->ib_zoomout->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
			this->ib_zoomout->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"ib_zoomout.Image")));
			this->ib_zoomout->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->ib_zoomout->Name = L"ib_zoomout";
			this->ib_zoomout->Size = System::Drawing::Size(23, 22);
			this->ib_zoomout->Text = L"Zoom Out";
			this->ib_zoomout->Click += gcnew System::EventHandler(this, &image_add_win::ib_zoomout_Click);
			// 
			// menuStrip1
			// 
			this->menuStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(4) {this->fileToolStripMenuItem, 
				this->toolStripMenuItem3, this->calibrationToolStripMenuItem, this->aboutToolStripMenuItem});
			this->menuStrip1->Location = System::Drawing::Point(0, 0);
			this->menuStrip1->Name = L"menuStrip1";
			this->menuStrip1->Padding = System::Windows::Forms::Padding(8, 2, 0, 2);
			this->menuStrip1->Size = System::Drawing::Size(781, 28);
			this->menuStrip1->TabIndex = 1;
			this->menuStrip1->Text = L"menuStrip1";
			// 
			// fileToolStripMenuItem
			// 
			this->fileToolStripMenuItem->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(6) {this->menu_open_image, 
				this->menu_save_meta_only, this->toolStripMenuItem2, this->toolStripMenuItem4, this->exitToolStripMenuItem, this->menu_exit});
			this->fileToolStripMenuItem->Name = L"fileToolStripMenuItem";
			this->fileToolStripMenuItem->Size = System::Drawing::Size(44, 24);
			this->fileToolStripMenuItem->Text = L"File";
			// 
			// menu_open_image
			// 
			this->menu_open_image->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"menu_open_image.Image")));
			this->menu_open_image->Name = L"menu_open_image";
			this->menu_open_image->Size = System::Drawing::Size(259, 24);
			this->menu_open_image->Text = L"Open Image";
			this->menu_open_image->Click += gcnew System::EventHandler(this, &image_add_win::menu_open_image_Click);
			// 
			// menu_save_meta_only
			// 
			this->menu_save_meta_only->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"menu_save_meta_only.Image")));
			this->menu_save_meta_only->Name = L"menu_save_meta_only";
			this->menu_save_meta_only->Size = System::Drawing::Size(259, 24);
			this->menu_save_meta_only->Text = L"Save Image MetaData Only";
			this->menu_save_meta_only->Click += gcnew System::EventHandler(this, &image_add_win::menu_save_meta_only_Click);
			// 
			// toolStripMenuItem2
			// 
			this->toolStripMenuItem2->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"toolStripMenuItem2.Image")));
			this->toolStripMenuItem2->Name = L"toolStripMenuItem2";
			this->toolStripMenuItem2->Size = System::Drawing::Size(259, 24);
			this->toolStripMenuItem2->Text = L"Save Image";
			this->toolStripMenuItem2->Click += gcnew System::EventHandler(this, &image_add_win::toolStripMenuItem2_Click);
			// 
			// toolStripMenuItem4
			// 
			this->toolStripMenuItem4->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"toolStripMenuItem4.Image")));
			this->toolStripMenuItem4->Name = L"toolStripMenuItem4";
			this->toolStripMenuItem4->Size = System::Drawing::Size(259, 24);
			this->toolStripMenuItem4->Text = L"Export Image";
			this->toolStripMenuItem4->Click += gcnew System::EventHandler(this, &image_add_win::ib_export_Click);
			// 
			// exitToolStripMenuItem
			// 
			this->exitToolStripMenuItem->Name = L"exitToolStripMenuItem";
			this->exitToolStripMenuItem->Size = System::Drawing::Size(256, 6);
			// 
			// menu_exit
			// 
			this->menu_exit->Name = L"menu_exit";
			this->menu_exit->Size = System::Drawing::Size(259, 24);
			this->menu_exit->Text = L"Exit";
			this->menu_exit->Click += gcnew System::EventHandler(this, &image_add_win::menu_exit_Click);
			// 
			// toolStripMenuItem3
			// 
			this->toolStripMenuItem3->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(2) {this->toolStripMenuItem1, 
				this->copyToClipboardToolStripMenuItem});
			this->toolStripMenuItem3->Name = L"toolStripMenuItem3";
			this->toolStripMenuItem3->Size = System::Drawing::Size(87, 24);
			this->toolStripMenuItem3->Text = L"Clipboard";
			// 
			// toolStripMenuItem1
			// 
			this->toolStripMenuItem1->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"toolStripMenuItem1.Image")));
			this->toolStripMenuItem1->Name = L"toolStripMenuItem1";
			this->toolStripMenuItem1->Size = System::Drawing::Size(219, 24);
			this->toolStripMenuItem1->Text = L"Paste from Clipboard";
			this->toolStripMenuItem1->Click += gcnew System::EventHandler(this, &image_add_win::ib_paste_Click);
			// 
			// copyToClipboardToolStripMenuItem
			// 
			this->copyToClipboardToolStripMenuItem->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"copyToClipboardToolStripMenuItem.Image")));
			this->copyToClipboardToolStripMenuItem->Name = L"copyToClipboardToolStripMenuItem";
			this->copyToClipboardToolStripMenuItem->Size = System::Drawing::Size(219, 24);
			this->copyToClipboardToolStripMenuItem->Text = L"Copy to Clipboard";
			this->copyToClipboardToolStripMenuItem->Click += gcnew System::EventHandler(this, &image_add_win::ib_copyclip_Click);
			// 
			// calibrationToolStripMenuItem
			// 
			this->calibrationToolStripMenuItem->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(2) {this->menu_cal_load, 
				this->menu_cal_save});
			this->calibrationToolStripMenuItem->Name = L"calibrationToolStripMenuItem";
			this->calibrationToolStripMenuItem->Size = System::Drawing::Size(94, 24);
			this->calibrationToolStripMenuItem->Text = L"Calibration";
			// 
			// menu_cal_load
			// 
			this->menu_cal_load->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"menu_cal_load.Image")));
			this->menu_cal_load->Name = L"menu_cal_load";
			this->menu_cal_load->Size = System::Drawing::Size(111, 24);
			this->menu_cal_load->Text = L"Load";
			this->menu_cal_load->Click += gcnew System::EventHandler(this, &image_add_win::menu_cal_load_Click);
			// 
			// menu_cal_save
			// 
			this->menu_cal_save->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"menu_cal_save.Image")));
			this->menu_cal_save->Name = L"menu_cal_save";
			this->menu_cal_save->Size = System::Drawing::Size(111, 24);
			this->menu_cal_save->Text = L"Save";
			this->menu_cal_save->Click += gcnew System::EventHandler(this, &image_add_win::menu_cal_save_Click);
			// 
			// aboutToolStripMenuItem
			// 
			this->aboutToolStripMenuItem->Alignment = System::Windows::Forms::ToolStripItemAlignment::Right;
			this->aboutToolStripMenuItem->Name = L"aboutToolStripMenuItem";
			this->aboutToolStripMenuItem->Size = System::Drawing::Size(62, 24);
			this->aboutToolStripMenuItem->Text = L"About";
			this->aboutToolStripMenuItem->Click += gcnew System::EventHandler(this, &image_add_win::aboutToolStripMenuItem_Click);
			// 
			// propgrid
			// 
			this->propgrid->Dock = System::Windows::Forms::DockStyle::Fill;
			this->propgrid->HelpVisible = false;
			this->propgrid->Location = System::Drawing::Point(0, 0);
			this->propgrid->Margin = System::Windows::Forms::Padding(4);
			this->propgrid->Name = L"propgrid";
			this->propgrid->Size = System::Drawing::Size(260, 480);
			this->propgrid->TabIndex = 6;
			this->propgrid->ToolbarVisible = false;
			this->propgrid->PropertyValueChanged += gcnew System::Windows::Forms::PropertyValueChangedEventHandler(this, &image_add_win::propgrid_PropertyValueChanged);
			// 
			// splitter
			// 
			this->splitter->Dock = System::Windows::Forms::DockStyle::Fill;
			this->splitter->Location = System::Drawing::Point(26, 53);
			this->splitter->Margin = System::Windows::Forms::Padding(4);
			this->splitter->Name = L"splitter";
			// 
			// splitter.Panel2
			// 
			this->splitter->Panel2->Controls->Add(this->propgrid);
			this->splitter->Size = System::Drawing::Size(755, 480);
			this->splitter->SplitterDistance = 490;
			this->splitter->SplitterWidth = 5;
			this->splitter->TabIndex = 5;
			// 
			// toolStrip2
			// 
			this->toolStrip2->Dock = System::Windows::Forms::DockStyle::Left;
			this->toolStrip2->GripStyle = System::Windows::Forms::ToolStripGripStyle::Hidden;
			this->toolStrip2->Location = System::Drawing::Point(0, 53);
			this->toolStrip2->Name = L"toolStrip2";
			this->toolStrip2->Size = System::Drawing::Size(26, 480);
			this->toolStrip2->TabIndex = 6;
			this->toolStrip2->Text = L"toolStrip2";
			// 
			// image_add_win
			// 
			this->AllowDrop = true;
			this->AutoScaleDimensions = System::Drawing::SizeF(8, 16);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(781, 558);
			this->Controls->Add(this->splitter);
			this->Controls->Add(this->toolStrip2);
			this->Controls->Add(this->toolStrip1);
			this->Controls->Add(this->statusStrip1);
			this->Controls->Add(this->menuStrip1);
			this->Icon = (cli::safe_cast<System::Drawing::Icon^  >(resources->GetObject(L"$this.Icon")));
			this->MainMenuStrip = this->menuStrip1;
			this->Margin = System::Windows::Forms::Padding(4);
			this->MinimumSize = System::Drawing::Size(554, 568);
			this->Name = L"image_add_win";
			this->Text = L"ImageAnalyzer";
			this->Load += gcnew System::EventHandler(this, &image_add_win::image_add_win_Load);
			this->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &image_add_win::image_add_win_DragDrop);
			this->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &image_add_win::image_add_win_DragEnter);
			this->statusStrip1->ResumeLayout(false);
			this->statusStrip1->PerformLayout();
			this->toolStrip1->ResumeLayout(false);
			this->toolStrip1->PerformLayout();
			this->menuStrip1->ResumeLayout(false);
			this->menuStrip1->PerformLayout();
			this->splitter->Panel2->ResumeLayout(false);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->splitter))->EndInit();
			this->splitter->ResumeLayout(false);
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion

		void redraw(void);
		void repaint(void);
		vector getrealcoord(vector sc);
		void reselect(void);
		void clearmdata();
		void setmode(int om);
		cmeasurement ^find_template(int id);
		void create_templates();
		System::Void initialize_new_image();
		System::Void open_image(String ^filename);
		void read_xml_file(String ^filename, int asrefonly);
		void write_xml_file(String ^fn, int embed_image);
		void paint_to(Graphics ^g);
		void quick_linecol(Color c);
		Bitmap ^create_color_bm(Color c);
		ToolStripButton ^create_color_tsb(int r, int g, int b, String ^name);
		System::Void ColorSel_Click(System::Object^  sender, System::EventArgs^  e);

	private: System::Void menu_exit_Click(System::Object^  sender, System::EventArgs^  e) {
				 Application::Exit();
			 }
private: System::Void menu_open_image_Click(System::Object^  sender, System::EventArgs^  e);
private: System::Void pbox_SizeChanged(System::Object^  sender, System::EventArgs^  e);
private: System::Void pbox_Click(System::Object^  sender, System::EventArgs^  e);
private: System::Void image_add_win_Load(System::Object^  sender, System::EventArgs^  e);
private: System::Void pbox_MouseWheel(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e);
private: System::Void pbox_MouseMove(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e);
private: System::Void pbox_MouseEnter(System::Object^  sender, System::EventArgs^  e) {
			 pbox2->Focus();
		 }
private: System::Void pbox_MouseLeave(System::Object^  sender, System::EventArgs^  e) {
			 pbox2->Parent->Focus();
		 }
private: System::Void pbox_MouseDown(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e);
private: System::Void pbox_MouseUp(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e);
private: System::Void propgrid_PropertyValueChanged(System::Object^  s, System::Windows::Forms::PropertyValueChangedEventArgs^  e) {
		 redraw();
		propgrid->Refresh();

		 }
private: System::Void pbox_Paint(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e) {
			 repaint();
		 }

private: System::Void ib_pointer_Click(System::Object^  sender, System::EventArgs^  e) { setmode(0); }
private: System::Void ib_line_Click(System::Object^  sender, System::EventArgs^  e) { setmode(1); }
private: System::Void ib_point_Click(System::Object^  sender, System::EventArgs^  e) { setmode(2); }
private: System::Void ib_circle3p_Click(System::Object^  sender, System::EventArgs^  e) { setmode(3); }
private: System::Void ib_angle_Click(System::Object^  sender, System::EventArgs^  e) { setmode(4); }
private: System::Void ib_area_Click(System::Object^  sender, System::EventArgs^  e) { setmode(5); }
private: System::Void ib_path_Click(System::Object^  sender, System::EventArgs^  e) { setmode(6); }
private: System::Void ib_annot_box_Click(System::Object^  sender, System::EventArgs^  e) { setmode(7); }
private: System::Void ib_annot_circle_Click(System::Object^  sender, System::EventArgs^  e) { setmode(8); }
private: System::Void ib_annot_rect_Click(System::Object^  sender, System::EventArgs^  e) { setmode(9); }
private: System::Void ib_annot_line_Click(System::Object^  sender, System::EventArgs^  e) { setmode(10); }
private: System::Void ib_annot_arrow_Click(System::Object^  sender, System::EventArgs^  e) { setmode(11); }
private: System::Void ib_profile_Click(System::Object^  sender, System::EventArgs^  e) { setmode(12); }

private: System::Void image_add_win_KeyPress(System::Object^  sender, System::Windows::Forms::KeyPressEventArgs^  e);
private: System::Void ib_delete_Click(System::Object^  sender, System::EventArgs^  e);

private: System::Void ib_paste_Click(System::Object^  sender, System::EventArgs^  e);
private: System::Void ib_coord_Click(System::Object^  sender, System::EventArgs^  e);
private: System::Void ib_colorbar_Click(System::Object^  sender, System::EventArgs^  e);
private: System::Void image_add_win_KeyDown(System::Object^  sender, System::Windows::Forms::KeyEventArgs^  e);
private: System::Void ib_zoomout_Click(System::Object^  sender, System::EventArgs^  e);
private: System::Void ib_save_Click(System::Object^  sender, System::EventArgs^  e);
private: System::Void ib_open_Click(System::Object^  sender, System::EventArgs^  e);
private: System::Void menu_cal_load_Click(System::Object^  sender, System::EventArgs^  e);
private: System::Void menu_cal_save_Click(System::Object^  sender, System::EventArgs^  e);
private: System::Void toolStripMenuItem2_Click(System::Object^  sender, System::EventArgs^  e);
private: System::Void aboutToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e);
private: System::Void image_add_win_DragDrop(System::Object^  sender, System::Windows::Forms::DragEventArgs^  e);
private: System::Void image_add_win_DragEnter(System::Object^  sender, System::Windows::Forms::DragEventArgs^  e);
private: System::Void ib_copyclip_Click(System::Object^  sender, System::EventArgs^  e);
private: System::Void ib_export_Click(System::Object^  sender, System::EventArgs^  e);
private: System::Void menu_save_meta_only_Click(System::Object^  sender, System::EventArgs^  e);
private: System::Void ib_copyCSV_Click(System::Object^  sender, System::EventArgs^  e);
private: System::Void ib_copyclip_emf_Click(System::Object^  sender, System::EventArgs^  e);
};
}

