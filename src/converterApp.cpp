/*===================================================================================
                                       Converter
                              Copyright Kerry R. Loux 2013

     No requirement for distribution of wxWidgets libraries, source, or binaries.
                             (http://www.wxwidgets.org/)

===================================================================================*/

// File:  converterApp.h
// Created:  5/31/2013
// Author:  K. Loux
// Description:  The application class.
// History:

// Local headers
#include "converterApp.h"
#include "mainFrame.h"

// Implement the application (have wxWidgets set up the appropriate entry points, etc.)
IMPLEMENT_APP(ConverterApp);

//==========================================================================
// Class:			ConverterApp
// Function:		Constant Declarations
//
// Description:		Constant declarations for the ConverterApp class.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
const wxString ConverterApp::title = _T("Converter");
const wxString ConverterApp::name = _T("ConverterApplication");
const wxString ConverterApp::creator = _T("Kerry Loux");
const wxString ConverterApp::versionString = _T("v1.0");

//==========================================================================
// Class:			ConverterApp
// Function:		OnInit
//
// Description:		Initializes the application window.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		bool = true for successful window initialization, false for error
//
//==========================================================================
bool ConverterApp::OnInit()
{
	// Set the application's name and the vendor's name
	SetAppName(name);
	SetVendorName(creator);

	// Create the MainFrame object - this is the parent for all other objects
	mainFrame = new MainFrame();

	// Make sure the MainFrame was successfully created
	if (mainFrame == NULL)
		return false;

	// Make the window visible
	mainFrame->Show(true);

	// Bring the window to the top
	//SetTopWindow(mainFrame);

	return true;
}