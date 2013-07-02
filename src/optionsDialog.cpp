/*===================================================================================
                                       Converter
                              Copyright Kerry R. Loux 2013

     No requirement for distribution of wxWidgets libraries, source, or binaries.
                             (http://www.wxwidgets.org/)

===================================================================================*/

// File:  optionsDialog.cpp
// Created:  6/5/2013
// Author:  K. Loux
// Description:  Dialog for configuring application options.
// History:

// wxWidgets headers
#include <wx/notebook.h>
#include <wx/clipbrd.h>

// Local headers
#include "optionsDialog.h"

#ifdef __WXMSW__
#pragma warning(disable:4428)// Compiler bug erroneously flags strings containing unicode characters
#endif

//==========================================================================
// Class:			OptionsDialog
// Function:		OptionsDialog
//
// Description:		Constructor for OptionsDialog class.
//
// Input Arguments:
//		parent	= wxWindow*
//		xml		= XMLConversionFactors&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
OptionsDialog::OptionsDialog(wxWindow *parent, XMLConversionFactors &xml)
	: wxDialog(parent, wxID_ANY, _T("Options")), xml(xml)
{
	CreateControls();
	CenterOnParent();
}

//==========================================================================
// Class:			OptionsDialog
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
BEGIN_EVENT_TABLE(OptionsDialog, wxDialog)
	EVT_LISTBOX(idGroups,	OptionsDialog::OnSelectedGroupChange)
	EVT_BUTTON(idNewGroup,	OptionsDialog::OnNewGroup)
	EVT_BUTTON(idNewUnit,	OptionsDialog::OnNewUnit)
END_EVENT_TABLE();

//==========================================================================
// Class:			OptionsDialog
// Function:		CreateControls
//
// Description:		Creates controls for the dialog.
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
void OptionsDialog::CreateControls(void)
{
	wxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
	wxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
	topSizer->Add(mainSizer, 0, wxALL, 5);

	wxFlexGridSizer *controlSizer = new wxFlexGridSizer(2, 5, 5);
	mainSizer->Add(controlSizer, 1, wxGROW);

	controlSizer->Add(new wxStaticText(this, wxID_ANY, _T("Visible Groups")));
	controlSizer->Add(new wxStaticText(this, wxID_ANY, _T("Units")));

	int minWidth(150);
	groupList = new wxCheckListBox(this, idGroups);
	groupList->SetMinSize(wxSize(minWidth, -1));
	AddGroupsToControl();
	unitList = new wxListBox(this, wxID_ANY);
	unitList->SetMinSize(wxSize(minWidth, -1));
	controlSizer->Add(groupList, 0, wxGROW);
	controlSizer->Add(unitList, 0, wxGROW);
	controlSizer->AddGrowableRow(1);

	controlSizer->Add(new wxButton(this, idNewGroup, _T("New Group")));
	controlSizer->Add(new wxButton(this, idNewUnit, _T("New Unit")));

	// TODO:  Allow specifying tab order?

	wxSizer *buttons = CreateButtonSizer(wxOK | wxCANCEL);
	if (buttons)
	{
		mainSizer->AddSpacer(15);
		mainSizer->Add(buttons, 0, wxGROW);
	}

	groupList->SetSelection(0);
	UpdateUnitList();

	SetSizerAndFit(topSizer);
}

//==========================================================================
// Class:			OptionsDialog
// Function:		TransferDataFromWindow
//
// Description:		Determines if the data on the control is acceptable and
//					updates the xml object as required.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, false if problems found, true otherwise
//
//==========================================================================
bool OptionsDialog::TransferDataFromWindow(void)
{
	unsigned int i, j;
	bool hasUnits;
	for (i = 0; i < newGroups.size(); i++)
	{
		hasUnits = false;
		for (j = 0; j < newUnits.size(); j++)
		{
			if (newGroups[i].Cmp(newUnits[j].first) == 0)
			{
				hasUnits = true;
				break;
			}
		}

		if (!hasUnits)
		{
			wxMessageBox(_T("Group '") + newGroups[i] + _T("' has no unit definitions!"), _T("Error"), wxICON_ERROR, this);
			return false;
		}
	}

	for (i = 0; i < newGroups.size(); i++)
		xml.AddGroup(newGroups[i]);

	for (i = 0; i < newUnits.size(); i++)
		xml.AddEquivalence(newUnits[i].first, newUnits[i].second);

	for (i = 0; i < groupList->GetCount(); i++)
		xml.SetGroupVisibility(groupList->GetString(i), groupList->IsChecked(i));

	xml.Save();

	return true;
}

//==========================================================================
// Class:			OptionsDialog
// Function:		OnNewGroup
//
// Description:		Handles New Group button click events.
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
void OptionsDialog::OnNewGroup(wxCommandEvent& event)
{
	wxString groupName = wxGetTextFromUser(_T("Enter new group name:"), _T("New Group"), wxEmptyString, this);
	if (groupName.IsEmpty())
		return;

	unsigned int i;
	for (i = 0; i < groupList->GetCount(); i++)
	{
		if (groupName.Cmp(groupList->GetString(i)) == 0)
		{
			wxMessageBox(_T("Group '") + groupName + _T("' already exists!"), _T("Error"), wxICON_ERROR, this);
			return;
		}
	}

	// Insert group alphabetically
	unsigned int newIndex = GetAlphabeticIndex(groupName);
	groupList->Insert(groupName, newIndex);
	groupList->Check(newIndex);
	newGroups.push_back(groupName);

	groupList->SetSelection(newIndex);
	UpdateUnitList();

	OnNewUnit(event);
}

//==========================================================================
// Class:			OptionsDialog
// Function:		OnNewUnit
//
// Description:		Handles OK button click events.
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
void OptionsDialog::OnNewUnit(wxCommandEvent& WXUNUSED(event))
{
	wxString groupName = groupList->GetString(groupList->GetSelection());
	AddUnitDialog dialog(this, groupName, GetCompleteUnitList(groupName));
	if (dialog.ShowModal() != wxID_OK)
		return;

	unitList->Append(dialog.GetUnitName());
	if (unitList->GetCount() == 1)
		unitList->Append(dialog.GetEquivalentUnitName());

	XMLConversionFactors::Equivalence equiv;
	equiv.aUnit = dialog.GetUnitName();
	equiv.bUnit = dialog.GetEquivalentUnitName();
	equiv.equation = _T("a=b") + dialog.GetConversionFactor();
	newUnits.push_back(std::make_pair(groupName, equiv));
}

//==========================================================================
// Class:			OptionsDialog
// Function:		OnSelectedGroupChange
//
// Description:		Handles group selection change events.
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
void OptionsDialog::OnSelectedGroupChange(wxCommandEvent& WXUNUSED(event))
{
	UpdateUnitList();
}

//==========================================================================
// Class:			OptionsDialog
// Function:		AddGroupsToControl
//
// Description:		Adds all of the groups to the control and sets the check
//					value according to it's visibility.
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
void OptionsDialog::AddGroupsToControl(void)
{
	unsigned int i;
	for (i = 0; i < xml.GroupCount(); i++)
	{
		groupList->Append(xml.GetGroup(i).name);
		groupList->Check(i, xml.GetGroup(i).display);
	}
}

//==========================================================================
// Class:			OptionsDialog
// Function:		UpdateUnitList
//
// Description:		Updates the list of units.
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
void OptionsDialog::UpdateUnitList(void)
{
	unitList->Clear();

	unsigned int i;
	wxArrayString units = GetCompleteUnitList(groupList->GetString(groupList->GetSelection()));
	for (i = 0; i < units.Count(); i++)
		unitList->Append(units[i]);
}

//==========================================================================
// Class:			OptionsDialog
// Function:		GetCompleteUnitList
//
// Description:		Returns a list of old and any new units for the specified group.
//
// Input Arguments:
//		groupName	= const wxString&
//
// Output Arguments:
//		None
//
// Return Value:
//		wxArrayString
//
//==========================================================================
wxArrayString OptionsDialog::GetCompleteUnitList(const wxString &groupName) const
{
	unsigned int i;
	bool isNewGroup(false);
	for (i = 0; i < newGroups.size(); i++)
	{
		if (newGroups[i].Cmp(groupName) == 0)
			isNewGroup = true;
	}

	wxArrayString units;
	if (!isNewGroup)
	{
		XMLConversionFactors::FactorGroup group = xml.GetGroup(groupName);
		units = group.GetUnitList();
	}

	wxArrayString newUnitList = GetNewUnits(groupName);
	for (i = 0; i < newUnitList.Count(); i++)
		units.Add(newUnitList[i]);

	return units;
}

//==========================================================================
// Class:			OptionsDialog
// Function:		GetNewUnits
//
// Description:		Returns a list of any new units for the specified group.
//
// Input Arguments:
//		groupName	= const wxString&
//
// Output Arguments:
//		None
//
// Return Value:
//		wxArrayString
//
//==========================================================================
wxArrayString OptionsDialog::GetNewUnits(const wxString &groupName) const
{
	wxArrayString newUnitList;
	unsigned int i;
	for (i = 0; i < newUnits.size(); i++)
	{
		if (newUnits[i].first.Cmp(groupName) == 0)
		{
			newUnitList.Add(newUnits[i].second.aUnit);
			newUnitList.Add(newUnits[i].second.bUnit);
		}
	}

	wxArrayString oldUnits;
	for (i = 0; i < xml.GroupCount(); i++)
	{
		if (xml.GetGroup(i).name.Cmp(groupName) == 0)
		{
			oldUnits = xml.GetGroup(i).GetUnitList();
			break;
		}
	}

	newUnitList.Sort();
	unsigned int j;
	for (i = 1; i < newUnitList.Count(); i++)
	{
		if (newUnitList[i].Cmp(newUnitList[i - 1]) == 0)
		{
			newUnitList.RemoveAt(i);
			i--;
			continue;
		}

		for (j = 0; j < oldUnits.Count(); j++)
		{
			if (newUnitList[i].Cmp(oldUnits[j]) == 0)
			{
				newUnitList.RemoveAt(i);
				i--;
				break;
			}
		}
	}

	return newUnitList;
}

//==========================================================================
// Class:			OptionsDialog
// Function:		GetAlphabeticIndex
//
// Description:		Returns the index where the group should be inserted in
//					order to maintain alphabetic order.
//
// Input Arguments:
//		groupName	= const wxString&
//
// Output Arguments:
//		None
//
// Return Value:
//		unsigned int
//
//==========================================================================
unsigned int OptionsDialog::GetAlphabeticIndex(const wxString &groupName) const
{
	unsigned int i;
	for (i = 0; i < groupList->GetCount(); i++)
	{
		if (groupName.CmpNoCase(groupList->GetString(i)) < 0)
			return i;
	}

	return groupList->GetCount();
}

//==========================================================================
// Class:			AddUnitDialog
// Function:		AddUnitDialog
//
// Description:		Constructor for AddUnitDialog class.
//
// Input Arguments:
//		parent		= wxWindow*
//		groupName	= const wxString&
//		unitList	= const wxArrayString&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
OptionsDialog::AddUnitDialog::AddUnitDialog(wxWindow *parent, const wxString &groupName,
	const wxArrayString &unitList)
	: wxDialog(parent, wxID_ANY, _T("Add ") + groupName + _T(" Unit"))
{
	// TODO:  Allow editing units by passing existing unit info into constructor
	CreateControls(unitList);
	CenterOnParent();
}

//==========================================================================
// Class:			AddUnitDialog
// Function:		CreateControls
//
// Description:		Creates dialog controls.
//
// Input Arguments:
//		unitList	= const wxArrayString&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void OptionsDialog::AddUnitDialog::CreateControls(const wxArrayString &unitList)
{
	wxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
	wxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
	topSizer->Add(mainSizer, 0, wxALL, 5);

	wxSizer *inputSizer = new wxBoxSizer(wxHORIZONTAL);
	mainSizer->Add(inputSizer);

	aQtyText = new wxTextCtrl(this, wxID_ANY, _T("1"));
	unit = new wxTextCtrl(this, idUnit);
	bQtyText = new wxTextCtrl(this, wxID_ANY, _T("1"));
	if (unitList.Count() == 0)
		equivalentUnit = new wxComboBox(this, idEquivUnit, wxEmptyString, wxDefaultPosition, wxDefaultSize, unitList, wxCB_DROPDOWN);
	else
		equivalentUnit = new wxComboBox(this, wxID_ANY, unitList[0], wxDefaultPosition, wxDefaultSize, unitList, wxCB_READONLY);

	int padding(5);
	inputSizer->Add(aQtyText, 0, wxALL, padding);
	inputSizer->Add(unit, 0, wxALL, padding);
	inputSizer->Add(new wxStaticText(this, wxID_ANY, _T("is equal to")), 0, wxALL, padding);
	inputSizer->Add(bQtyText, 0, wxALL, padding);
	inputSizer->Add(equivalentUnit, 0, wxALL, padding);

	wxSizer *powerSizer = new wxGridSizer(6, 5, 5);
	mainSizer->Add(powerSizer);
	const int btnWidth = 30;
#ifdef _UNICODE
	powerSizer->Add(new wxButton(this, idNegative, _T("\u207b"), wxDefaultPosition, wxSize(btnWidth, -1)));
	powerSizer->Add(new wxButton(this, idFirst, _T("\u00b9"), wxDefaultPosition, wxSize(btnWidth, -1)));
	powerSizer->Add(new wxButton(this, idSecond, _T("\u00b2"), wxDefaultPosition, wxSize(btnWidth, -1)));
	powerSizer->Add(new wxButton(this, idThird, _T("\u00b3"), wxDefaultPosition, wxSize(btnWidth, -1)));
	powerSizer->Add(new wxButton(this, idFourth, _T("\u2074"), wxDefaultPosition, wxSize(btnWidth, -1)));
	powerSizer->Add(new wxButton(this, idFifth, _T("\u2075"), wxDefaultPosition, wxSize(btnWidth, -1)));
	powerSizer->AddSpacer(0);
	powerSizer->Add(new wxButton(this, idSixth, _T("\u2076"), wxDefaultPosition, wxSize(btnWidth, -1)));
	powerSizer->Add(new wxButton(this, idSeventh, _T("\u2077"), wxDefaultPosition, wxSize(btnWidth, -1)));
	powerSizer->Add(new wxButton(this, idEigth, _T("\u2078"), wxDefaultPosition, wxSize(btnWidth, -1)));
	powerSizer->Add(new wxButton(this, idNinth, _T("\u2079"), wxDefaultPosition, wxSize(btnWidth, -1)));
	powerSizer->Add(new wxButton(this, idZeroth, _T("\u2070"), wxDefaultPosition, wxSize(btnWidth, -1)));
#else
	//powerSizer->Add(new wxButton(this, idFirst, _T("\u00b9"), wxDefaultPosition, wxSize(btnWidth, -1)));
	powerSizer->Add(new wxButton(this, idSecond, _T("\u00b2"), wxDefaultPosition, wxSize(btnWidth, -1)));
	powerSizer->Add(new wxButton(this, idThird, _T("\u00b3"), wxDefaultPosition, wxSize(btnWidth, -1)));
#endif

	wxSizer *buttons = CreateButtonSizer(wxOK | wxCANCEL);
	if (buttons)
	{
		mainSizer->AddSpacer(15);
		mainSizer->Add(buttons, 0, wxGROW);
	}

	aQtyText->SetFocus();
	focusedControl = idAny;
	cursorPosition = 0;

	SetSizerAndFit(topSizer);
}

//==========================================================================
// Class:			OptionsDialog
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
BEGIN_EVENT_TABLE(OptionsDialog::AddUnitDialog, wxDialog)
	EVT_BUTTON(idZeroth,		OptionsDialog::AddUnitDialog::OnButton)
	EVT_BUTTON(idFirst,			OptionsDialog::AddUnitDialog::OnButton)
	EVT_BUTTON(idSecond,		OptionsDialog::AddUnitDialog::OnButton)
	EVT_BUTTON(idThird,			OptionsDialog::AddUnitDialog::OnButton)
	EVT_BUTTON(idFourth,		OptionsDialog::AddUnitDialog::OnButton)
	EVT_BUTTON(idFifth,			OptionsDialog::AddUnitDialog::OnButton)
	EVT_BUTTON(idSixth,			OptionsDialog::AddUnitDialog::OnButton)
	EVT_BUTTON(idSeventh,		OptionsDialog::AddUnitDialog::OnButton)
	EVT_BUTTON(idEigth,			OptionsDialog::AddUnitDialog::OnButton)
	EVT_BUTTON(idNinth,			OptionsDialog::AddUnitDialog::OnButton)
	EVT_BUTTON(idNegative,		OptionsDialog::AddUnitDialog::OnButton)
	EVT_TEXT(idUnit,			OptionsDialog::AddUnitDialog::OnTextChange)
	EVT_TEXT(idEquivUnit,		OptionsDialog::AddUnitDialog::OnTextChange)
END_EVENT_TABLE();

//==========================================================================
// Class:			AddUnitDialog
// Function:		OnButton
//
// Description:		Event handler for "power" button clicks.
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
void OptionsDialog::AddUnitDialog::OnButton(wxCommandEvent &event)
{
	wxString value;
	if (focusedControl == idUnit)
		value = unit->GetValue();
	else if (focusedControl == idEquivUnit)
		value = equivalentUnit->GetValue();
	else
		return;

	wxString power;
	switch (event.GetId())
	{
#ifdef UNICODE
	case idZeroth:
		power = _T("\u2070");
		break;

	case idFourth:
		power = _T("\u2074");
		break;

	case idFifth:
		power = _T("\u2075");
		break;

	case idSixth:
		power = _T("\u2076");
		break;

	case idSeventh:
		power = _T("\u2077");
		break;

	case idEigth:
		power = _T("\u2078");
		break;

	case idNinth:
		power = _T("\u2079");
		break;

	case idNegative:
		power = _T("\u207b");
		break;
#endif

	case idFirst:
		power = _T("\u00b9");
		break;

	case idSecond:
		power = _T("\u00b2");
		break;

	case idThird:
		power = _T("\u00b3");
		break;

	default:
		assert(false);
	}

	value.insert(cursorPosition, power);
	cursorPosition++;

	if (focusedControl == idUnit)
	{
		unit->ChangeValue(value);
		unit->SetFocus();
		unit->SetInsertionPoint(cursorPosition);
	}
	else
	{
		long tempCursorPosition = cursorPosition;
		equivalentUnit->SetValue(value);
		cursorPosition = tempCursorPosition;
		equivalentUnit->SetFocus();
		equivalentUnit->SetInsertionPoint(cursorPosition);
	}
}

//==========================================================================
// Class:			AddUnitDialog
// Function:		OnTextChange
//
// Description:		Event handler for text change events.
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
void OptionsDialog::AddUnitDialog::OnTextChange(wxCommandEvent &event)
{
	focusedControl = (ControlIds)event.GetId();
	if (focusedControl == idUnit)
	{
		wxTextCtrl *box = static_cast<wxTextCtrl*>(FindWindow(focusedControl));
		cursorPosition = box->GetInsertionPoint();
	}
	else if (focusedControl == idEquivUnit)
	{
		wxComboBox *box = static_cast<wxComboBox*>(FindWindow(focusedControl));
		cursorPosition = box->GetInsertionPoint();
	}
}

//==========================================================================
// Class:			AddUnitDialog
// Function:		GetUnitName
//
// Description:		Returns the specified name for the new unit.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		wxString
//
//==========================================================================
wxString OptionsDialog::AddUnitDialog::GetUnitName(void) const
{
	return unit->GetValue();
}

//==========================================================================
// Class:			AddUnitDialog
// Function:		GetConversionFactor
//
// Description:		Returns the specified conversion factor for the new unit.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		wxString
//
//==========================================================================
wxString OptionsDialog::AddUnitDialog::GetConversionFactor(void) const
{
	wxString factor(wxEmptyString);
	if (aQtyText->GetValue().Cmp(_T("1")) != 0)
		factor.Append(_T("*")).Append(aQtyText->GetValue());
		
	if (bQtyText->GetValue().Cmp(_T("1")) != 0)
		factor.Append(_T("/")).Append(bQtyText->GetValue());

	return factor;
}

//==========================================================================
// Class:			AddUnitDialog
// Function:		GetEquivalentUnitName
//
// Description:		Returns the unit used to define the new unit.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		wxString
//
//==========================================================================
wxString OptionsDialog::AddUnitDialog::GetEquivalentUnitName(void) const
{
	return equivalentUnit->GetValue();
}

//==========================================================================
// Class:			AddUnitDialog
// Function:		TransferDataFromWindow
//
// Description:		Determines if input values are OK or not.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, false if problems found, true otherwise
//
//==========================================================================
bool OptionsDialog::AddUnitDialog::TransferDataFromWindow(void)
{
	if (unit->GetValue().IsEmpty() || equivalentUnit->GetValue().IsEmpty())
	{
		wxMessageBox(_T("Units name cannot be empty!"), _T("Error"), wxICON_ERROR, this);
		return false;
	}

	if (unit->GetValue().Cmp(equivalentUnit->GetValue()) == 0)
	{
		wxMessageBox(_T("Unit names must be unique!"), _T("Error"), wxICON_ERROR, this);
		return false;
	}

	double factor;
	if (!aQtyText->GetValue().ToDouble(&factor) || !bQtyText->GetValue().ToDouble(&factor))
	{
		wxMessageBox(_T("Could not convert conversion factor to value!"), _T("Error"), wxICON_ERROR, this);
		return false;
	}

	unsigned int i;
	for (i = 0; i < equivalentUnit->GetCount(); i++)
	{
		if (unit->GetValue().Cmp(equivalentUnit->GetString(i)) == 0)
		{
			wxMessageBox(_T("Unit '") + unit->GetValue() + _T("' already exists!"), _T("Error"), wxICON_ERROR, this);
			return false;
		}
	}

	return true;
}