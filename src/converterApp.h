/*===================================================================================
                                       Converter
                              Copyright Kerry R. Loux 2013

  This code is licensed under the MIT License (http://opensource.org/licenses/MIT).

===================================================================================*/

// File:  converterApp.h
// Created:  5/31/2013
// Author:  K. Loux
// Description:  The application class.
// History:

#ifndef _CONVERTER_APP_H_
#define _CONVERTER_APP_H_

// wxWidgets headers
#include <wx/wx.h>

// Local forward declarations
class MainFrame;

// The application class
class ConverterApp : public wxApp
{
public:
	bool OnInit();

	static const wxString title;// As displayed
	static const wxString name;// Internal
	static const wxString creator;
	static const wxString versionString;

private:
	// The main class for the application - this object is the parent for all other objects
	MainFrame *mainFrame;
};

// Declare the application object (have wxWidgets create the wxGetApp() function)
DECLARE_APP(ConverterApp);

#endif// _CONVERTER_APP_H_