// image_add.cpp : main project file.

#include "stdafx.h"
#include "image_add_win.h"

using namespace image_add;

[STAThreadAttribute]
int main(array<System::String ^> ^args)
{
	// Enabling Windows XP visual effects before any controls are created
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false); 

	// Create the main window and run it
	Application::Run(gcnew image_add_win(args));
	return 0;
}
