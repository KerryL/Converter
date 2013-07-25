/*===================================================================================
                                       Converter
                              Copyright Kerry R. Loux 2013

  This code is licensed under the MIT License (http://opensource.org/licenses/MIT).

===================================================================================*/

// File:  mainFrame.h
// Created:  5/31/2013
// Author:  K. Loux
// Description:  The main application window.
// History:

#ifndef _MAIN_FRAME_H_
#define _MAIN_FRAME_H_

// wxWidgets headers
#include <wx/wx.h>

// Local headers
#include "xmlConversionFactors.h"
#include "converter.h"

// wxWidgets forward declarations
class wxNotebook;
class wxNotebookEvent;

class MainFrame : public wxFrame
{
public:
	// Constructor
	MainFrame();

	// Destructor
	~MainFrame();

private:
	void CreateControls(void);
	void SetProperties(void);

	void EnforcePageConfiguration(const bool &saveConfig = true);
	void AddNotebookPage(const XMLConversionFactors::FactorGroup &group);

	wxNotebook *notebook;
	wxTextCtrl *input;
	wxTextCtrl *output;
	wxStaticText *inUnits;
	wxStaticText *outUnits;

	wxListBox *GetInputUnitsBox(void) const;
	wxListBox *GetOutputUnitsBox(void) const;
	wxWindow *GetControlOnActiveTab(int id) const;

	enum ControlID
	{
		idInput = wxID_HIGHEST + 1,
		idOutput,
		idOptions,
		idClipboard,
		idSwap
	};

	void OnTextChangeEvent(wxCommandEvent &event);
	void OnSelectionChangeEvent(wxCommandEvent &event);
	void OnTabChangeEvent(wxNotebookEvent &event);
	void OnOptionsButton(wxCommandEvent &event);
	void OnClipboardButton(wxCommandEvent &event);
	void OnSwapButton(wxCommandEvent &event);
	void OnClose(wxCloseEvent &event);

	void UpdateConversion(void);

	XMLConversionFactors xml;
	Converter converter;

	const wxString configFileName;
	void LoadConfiguration(void);
	void SaveConfiguration(void);

	DECLARE_EVENT_TABLE();
};

#endif// _MAIN_FRAME_H_