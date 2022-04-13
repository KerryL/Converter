/*===================================================================================
                                       Converter
                              Copyright Kerry R. Loux 2013

  This code is licensed under the MIT License (http://opensource.org/licenses/MIT).

===================================================================================*/

// File:  mainFrame.cpp
// Created:  5/31/2013
// Author:  K. Loux
// Description:  The main application window.
// History:

// Standard C++ headers
#include <cassert>

// wxWidgets headers
#include <wx/notebook.h>
#include <wx/clipbrd.h>
#include <wx/fileconf.h>

// Local headers
#include "mainFrame.h"
#include "converterApp.h"
#include "convertMath.h"
#include "optionsDialog.h"

// *nix Icons
#ifdef __WXGTK__
#include "../res/icons/icon16.xpm"
#include "../res/icons/icon24.xpm"
#include "../res/icons/icon32.xpm"
#include "../res/icons/icon48.xpm"
#include "../res/icons/icon64.xpm"
#include "../res/icons/icon128.xpm"
#endif

//==========================================================================
// Class:			MainFrame
// Function:		MainFrame
//
// Description:		Constructor for MainFrame class.
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
MainFrame::MainFrame() : wxFrame(NULL, wxID_ANY, wxEmptyString, wxDefaultPosition,
								 wxDefaultSize, wxDEFAULT_FRAME_STYLE),
								 xml(_T("conversions.xml")), converter(xml),
								 configFileName(_T("converterConfig.rc"))
{
	CreateControls();
	SetProperties();

	// Error messages handled by xml object
	if (xml.Load())
		EnforcePageConfiguration(false);
}

//==========================================================================
// Class:			MainFrame
// Function:		~MainFrame
//
// Description:		Destructor for MainFrame class.
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
MainFrame::~MainFrame()
{
}

//==========================================================================
// Class:			MainFrame
// Function:		Event Table
//
// Description:		Links GUI events with event handler functions.
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
BEGIN_EVENT_TABLE(MainFrame, wxFrame)
	EVT_TEXT(wxID_ANY,					MainFrame::OnTextChangeEvent)
	EVT_NOTEBOOK_PAGE_CHANGED(wxID_ANY,	MainFrame::OnTabChangeEvent)
	EVT_LISTBOX(wxID_ANY,				MainFrame::OnSelectionChangeEvent)
	EVT_BUTTON(idClipboard,				MainFrame::OnClipboardButton)
	EVT_BUTTON(idOptions,				MainFrame::OnOptionsButton)
	EVT_BUTTON(idSwap,					MainFrame::OnSwapButton)
	EVT_CLOSE(							MainFrame::OnClose)
	EVT_SIZE(							MainFrame::OnSize)
END_EVENT_TABLE();

//==========================================================================
// Class:			MainFrame
// Function:		CreateControls
//
// Description:		Creates all controls for the main frame.
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
void MainFrame::CreateControls()
{
	wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
	wxPanel *mainPanel = new wxPanel(this);
	topSizer->Add(mainPanel, 1, wxGROW);

	wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
	mainPanel->SetSizer(mainSizer);

	notebook = new wxNotebook(mainPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_TOP | wxNB_MULTILINE);
	notebook->SetMinSize(wxSize(400, 200));
	mainSizer->Add(notebook, 1, wxGROW | wxALL, 5);

	const int spacing(5);
	wxBoxSizer *topLowerSizer = new wxBoxSizer(wxHORIZONTAL);
	wxButton *swapButton = new wxButton(mainPanel, idSwap, _T("Swap"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	topLowerSizer->Add(swapButton, 0, wxRIGHT | wxGROW, spacing);

	wxFlexGridSizer *lowerSizer = new wxFlexGridSizer(4, spacing, spacing);
	topLowerSizer->Add(lowerSizer, 1, wxALL | wxGROW);

	int textBoxWidth(120);
	lowerSizer->Add(new wxStaticText(mainPanel, wxID_ANY, _T("Input:")));
	input = new wxTextCtrl(mainPanel, wxID_ANY, _T("1"), wxDefaultPosition, wxSize(textBoxWidth, -1));
	inUnits = new wxStaticText(mainPanel, wxID_ANY, wxEmptyString);
	lowerSizer->Add(input, 1, wxGROW);
	lowerSizer->Add(inUnits);
	lowerSizer->Add(new wxButton(mainPanel, idOptions, _T("Options")), 0, wxALIGN_RIGHT);

	lowerSizer->Add(new wxStaticText(mainPanel, wxID_ANY, _T("Output:")));
	output = new wxTextCtrl(mainPanel, wxID_ANY, _T("1"), wxDefaultPosition, wxSize(textBoxWidth, -1), wxTE_READONLY);
	//output->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));// This is the right color under MSW, not sure if it's necessary, though
	outUnits = new wxStaticText(mainPanel, wxID_ANY, wxEmptyString);
	lowerSizer->Add(output, 1);
	lowerSizer->Add(outUnits);
	lowerSizer->Add(new wxButton(mainPanel, idClipboard, _T("Clipboard")), 0, wxALIGN_RIGHT);
	lowerSizer->AddGrowableCol(3);

	mainSizer->Add(topLowerSizer, 0, wxALL | wxGROW, spacing);

	SetSizerAndFit(topSizer);
}

//==========================================================================
// Class:			MainFrame
// Function:		SetProperties
//
// Description:		Sets window properties.
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
void MainFrame::SetProperties()
{
	SetTitle(ConverterApp::title + _T(" ") + ConverterApp::versionString);
	SetName(ConverterApp::name);
	Center();

#ifdef __WXMSW__
	SetIcon(wxIcon(_T("ICON_ID_MAIN"), wxBITMAP_TYPE_ICO_RESOURCE));
#elif __WXGTK__
	SetIcon(wxIcon(icon16_xpm));
	SetIcon(wxIcon(icon24_xpm));
	SetIcon(wxIcon(icon32_xpm));
	SetIcon(wxIcon(icon48_xpm));
	SetIcon(wxIcon(icon64_xpm));
	SetIcon(wxIcon(icon128_xpm));
#endif
}

//==========================================================================
// Class:			MainFrame
// Function:		EnforcePageConfiguration
//
// Description:		Ensures that the proper tabs are displayed in the notebook.
//
// Input Arguments:
//		saveConfig	= const bool&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::EnforcePageConfiguration(const bool &saveConfig)
{
	if (saveConfig)
		SaveConfiguration();
	notebook->DeleteAllPages();

	unsigned int i;
	for (i = 0; i < xml.GroupCount(); i++)
		AddNotebookPage(xml.GetGroup(i));

	// This hack makes the notebook's multiple tabs proper (not sure why it's needed)
	SetSize(GetSize() + wxSize(1, 0));
	SetSize(GetSize() - wxSize(1, 0));

	LoadConfiguration();
	UpdateConversion();
}

//==========================================================================
// Class:			MainFrame
// Function:		OnTextChangeEvent
//
// Description:		Handles input text change events.
//
// Input Arguments:
//		event	= wxCommandEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::OnTextChangeEvent(wxCommandEvent& WXUNUSED(event))
{
	UpdateConversion();
}

//==========================================================================
// Class:			MainFrame
// Function:		OnSelectionChangeEvent
//
// Description:		Handles list box selection change events.
//
// Input Arguments:
//		event	= wxCommandEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::OnSelectionChangeEvent(wxCommandEvent& WXUNUSED(event))
{
	UpdateConversion();
}

//==========================================================================
// Class:			MainFrame
// Function:		OnTabChangeEvent
//
// Description:		Handles notebook tab change events.
//
// Input Arguments:
//		event	= wxNotebookEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::OnTabChangeEvent(wxNotebookEvent& WXUNUSED(event))
{
	UpdateConversion();
}

//==========================================================================
// Class:			MainFrame
// Function:		OnOptionsButton
//
// Description:		Handles options button click events.
//
// Input Arguments:
//		event	= wxCommandEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::OnOptionsButton(wxCommandEvent& WXUNUSED(event))
{
	OptionsDialog dialog(this, xml);
	if (dialog.ShowModal() != wxID_OK)
		return;

	if (xml.Load())
	{
		converter.ClearCache();
		EnforcePageConfiguration();
	}
}

//==========================================================================
// Class:			MainFrame
// Function:		OnClipboardButton
//
// Description:		Handles clipboard button click events.
//
// Input Arguments:
//		event	= wxCommandEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::OnClipboardButton(wxCommandEvent& WXUNUSED(event))
{
	if (wxTheClipboard->Open())
	{
		wxTheClipboard->SetData(new wxTextDataObject(output->GetValue()));
		wxTheClipboard->Close();
	}
}

//==========================================================================
// Class:			MainFrame
// Function:		OnSwapButton
//
// Description:		Handles swap button click events.
//
// Input Arguments:
//		event	= wxCommandEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::OnSwapButton(wxCommandEvent& WXUNUSED(event))
{
	wxListBox *in = GetInputUnitsBox();
	wxListBox *out = GetOutputUnitsBox();

	unsigned int inputSelection = in->GetSelection();
	in->SetSelection(out->GetSelection());
	out->SetSelection(inputSelection);

	input->ChangeValue(output->GetValue());

	UpdateConversion();
}

//==========================================================================
// Class:			MainFrame
// Function:		OnClose
//
// Description:		Handles window close events.
//
// Input Arguments:
//		event	= wxCloseEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::OnClose(wxCloseEvent& event)
{
	if (!IsActive())
		wxQueueEvent(this, new wxActivateEvent());// fix for application not closing if closed from taskbar when not focused; see https://forums.wxwidgets.org/viewtopic.php?t=43498

	SaveConfiguration();
	event.Skip();
}

//==========================================================================
// Class:			MainFrame
// Function:		OnSize
//
// Description:		Handles window size events.
//
// Input Arguments:
//		event	= wxSizeEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::OnSize(wxSizeEvent& event)
{
	// This fixes a bug where the list box contents don't render properly on MSW if
	// the list box position changes (due to the number of rows of tab controls changing).
	wxFrame::OnSize(event);
	auto inputBox(GetInputUnitsBox());
	auto outputBox(GetOutputUnitsBox());

	if (inputBox)
		inputBox->Refresh();

	if (outputBox)
		outputBox->Refresh();
}

//==========================================================================
// Class:			MainFrame
// Function:		UpdateConversion
//
// Description:		Updates the conversion output and unit labels.
//
// Input Arguments:
//		event	= wxCommandEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::UpdateConversion()
{
	if (notebook->GetPageCount() == 0)
		return;

	unsigned int i;
	wxString groupName, inUnit, outUnit;
	for (i = 0; i < notebook->GetPageCount(); i++)
	{
		if (notebook->GetCurrentPage() == notebook->GetPage(i))
		{
			groupName = notebook->GetPageText(i);
			break;
		}
	}

	wxListBox *box = GetInputUnitsBox();
	if (!box || box->GetSelection() < 0)
		return;
	inUnit = box->GetString(box->GetSelection());

	box = GetOutputUnitsBox();
	if (!box || box->GetSelection() < 0)
		return;
	outUnit = box->GetString(box->GetSelection());

	double inValue;
	if (!input->GetValue().ToDouble(&inValue))
		return;

	inUnits->SetLabel(inUnit);
	outUnits->SetLabel(outUnit);

	double outValue = converter.Convert(groupName, inUnit, outUnit, inValue);
	int orderOfMagnitude((unsigned int)floor(log10(outValue)));

	if (orderOfMagnitude < -3 ||// Value is very small
		orderOfMagnitude > 6)// Value is very large
		output->ChangeValue(wxString::Format(_T("%e"), outValue));
		/*output->ChangeValue(wxString::Format(_T("%0.*e"),
		ConvertMath::CountSignificantDigits(input->GetValue()) - 1, outValue));*/// FIXME:  Fix precision
	else
		output->ChangeValue(wxString::Format(_T("%f"), outValue));
		/*output->ChangeValue(wxString::Format(_T("%0.*f"),
		std::max<int>(0, ConvertMath::CountSignificantDigits(input->GetValue()) - orderOfMagnitude - 1), outValue));*/
}

//==========================================================================
// Class:			MainFrame
// Function:		GetInputUnitsBox
//
// Description:		Gets the input unit list box for the current tab.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		wxListBox*
//
//==========================================================================
wxListBox* MainFrame::GetInputUnitsBox() const
{
	return static_cast<wxListBox*>(GetControlOnActiveTab(idInput));
}

//==========================================================================
// Class:			MainFrame
// Function:		GetOutputUnitsBox
//
// Description:		Gets the output unit list box for the current tab.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		wxListBox*
//
//==========================================================================
wxListBox* MainFrame::GetOutputUnitsBox() const
{
	return static_cast<wxListBox*>(GetControlOnActiveTab(idOutput));
}

//==========================================================================
// Class:			MainFrame
// Function:		GetControlOnActiveTab
//
// Description:		Gets the specified control for the current tab.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		wxWindow*
//
//==========================================================================
wxWindow* MainFrame::GetControlOnActiveTab(int id) const
{
	if (notebook->GetPageCount() == 0)
		return NULL;

	unsigned int i;
	wxString groupName;
	for (i = 0; i < notebook->GetPageCount(); i++)
	{
		if (notebook->GetCurrentPage() == notebook->GetPage(i))
		{
			groupName = notebook->GetPageText(i);
			break;
		}
	}

	return notebook->GetCurrentPage()->FindWindow(id);
}

//==========================================================================
// Class:			MainFrame
// Function:		AddNotebookPage
//
// Description:		Adds a new group tab to the notebook.
//
// Input Arguments:
//		group	= const XMLConversionFactors::FactorGroup &group
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::AddNotebookPage(const XMLConversionFactors::FactorGroup &group)
{
	if (!group.display)
		return;

	wxArrayString unitList = group.GetUnitList();

	wxPanel *panel = new wxPanel(notebook);
	wxSizer *mainSizer = new wxBoxSizer(wxHORIZONTAL);
	panel->SetSizer(mainSizer);

	wxListBox *in = new wxListBox(panel, idInput, wxDefaultPosition, wxDefaultSize, unitList);
	wxListBox *out = new wxListBox(panel, idOutput, wxDefaultPosition, wxDefaultSize, unitList);
	mainSizer->Add(in, 1, wxGROW);
	mainSizer->Add(out, 1, wxGROW);

	in->SetSelection(0);
	out->SetSelection(0);

	notebook->AddPage(panel, group.name);
}

//==========================================================================
// Class:			MainFrame
// Function:		LoadConfiguration
//
// Description:		Loads the current configuration from file.
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
void MainFrame::LoadConfiguration()
{
	wxFileConfig *config = new wxFileConfig(_T(""), _T(""), configFileName, _T(""),
		wxCONFIG_USE_RELATIVE_PATH);

	wxWindow *page;
	wxString tabText;
	wxString selectedTab = config->Read(_T("Tabs/Selected"), wxEmptyString);
	unsigned int i;
	long selection(0);
	for (i = 0; i < notebook->GetPageCount(); i++)
	{
		page = notebook->GetPage(i);
		tabText = notebook->GetPageText(i);
		if (config->HasGroup(_T("/Tabs/") + tabText))
		{
			if (config->Read(_T("/Tabs/") + tabText + _T("/Input"), &selection))
			{
				if ((unsigned long)selection < static_cast<wxListBox*>(page->FindWindow(idInput))->GetCount())
					static_cast<wxListBox*>(page->FindWindow(idInput))->SetSelection(selection);
			}
			if (config->Read(_T("/Tabs/") + tabText + _T("/Output"), &selection))
			{
				if ((unsigned long)selection < static_cast<wxListBox*>(page->FindWindow(idOutput))->GetCount())
					static_cast<wxListBox*>(page->FindWindow(idOutput))->SetSelection(selection);
			}
		}

		if (tabText.Cmp(selectedTab) == 0)
			notebook->SetSelection(i);
	}

	int x(0), y(0), w(0), h(0);
	if (config->Read(_T("/Window/Width"), &w) &&
		config->Read(_T("/Window/Height"), &h) &&
		config->Read(_T("/Window/XPosition"), &x) &&
		config->Read(_T("/Window/YPosition"), &y))
		SetSize(x, y, w, h);

	input->ChangeValue(config->Read(_T("/Other/InputValue"), _T("1")));

	delete config;
}

//==========================================================================
// Class:			MainFrame
// Function:		SaveConfiguration
//
// Description:		Saves the current configuration to file.
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
void MainFrame::SaveConfiguration()
{
	if (!::wxDirExists(::wxGetCwd()))
	{
		wxMessageBox(_T("Working directory is inaccessible; configuration cannot be saved."),
			_T("Error"), wxOK | wxICON_WARNING, this);
		return;
	}

	const wxString transactionFileName(_T("~") + configFileName);
	wxFileConfig *config = new wxFileConfig(_T(""), _T(""), transactionFileName, _T(""),
		wxCONFIG_USE_RELATIVE_PATH);

	wxWindow *page;
	unsigned int i;
	for (i = 0; i < notebook->GetPageCount(); i++)
	{
		page = notebook->GetPage(i);
		config->Write(_T("/Tabs/") + notebook->GetPageText(i) + _T("/Input"),
					 static_cast<wxListBox*>(page->FindWindow(idInput))->GetSelection());
		config->Write(_T("/Tabs/") + notebook->GetPageText(i) + _T("/Output"),
					 static_cast<wxListBox*>(page->FindWindow(idOutput))->GetSelection());
	}

	if (notebook->GetPageCount() > 0)
		config->Write(_T("/Tabs/Selected"), notebook->GetPageText(notebook->GetSelection()));

	config->Write(_T("/Window/Width"), GetSize().GetWidth());
	config->Write(_T("/Window/Height"), GetSize().GetHeight());
	config->Write(_T("/Window/XPosition"), GetPosition().x);
	config->Write(_T("/Window/YPosition"), GetPosition().y);

	config->Write(_T("/Other/InputValue"), input->GetValue());

	delete config;

	wxCopyFile(transactionFileName, configFileName);
	wxRemoveFile(transactionFileName);
}
