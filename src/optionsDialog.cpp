/*===================================================================================
                                       Converter
                              Copyright Kerry R. Loux 2013

  This code is licensed under the MIT License (http://opensource.org/licenses/MIT).

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
	EVT_LISTBOX(idGroups,		OptionsDialog::OnSelectedGroupChange)
	EVT_BUTTON(idNewGroup,		OptionsDialog::OnNewGroup)
	EVT_BUTTON(idNewUnit,		OptionsDialog::OnNewUnit)
	EVT_LISTBOX_DCLICK(idUnits,	OptionsDialog::OnUnitDoubleClick)
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
	unitList = new wxListBox(this, idUnits);
	unitList->SetMinSize(wxSize(minWidth, -1));
	controlSizer->Add(groupList, 0, wxGROW);
	controlSizer->Add(unitList, 0, wxGROW);
	controlSizer->AddGrowableRow(1);

	controlSizer->Add(new wxButton(this, idNewGroup, _T("New Group")));
	controlSizer->Add(new wxButton(this, idNewUnit, _T("New Unit")));

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
			wxMessageBox(_T("Group '") + newGroups[i] +
				_T("' has no unit definitions!"), _T("Error"), wxICON_ERROR, this);
			return false;
		}
	}

	for (i = 0; i < newGroups.size(); i++)
		xml.AddGroup(newGroups[i]);

	try
	{
		for (i = 0; i < newUnits.size(); i++)
			xml.AddEquivalence(newUnits[i].first, newUnits[i].second);

		for (i = 0; i < changedUnits.size(); i++)
			xml.ChangeEquivalence(changedUnits[i].first, changedUnits[i].second);

		for (i = 0; i < groupList->GetCount(); i++)
			xml.SetGroupVisibility(groupList->GetString(i), groupList->IsChecked(i));
	}
	catch (std::exception &e)
	{
		wxString message(_T("Error adding to conversions file!\n\n"));
		message.Append(wxString(e.what(), wxConvUTF8));
		wxMessageBox(message, _T("Error"), wxICON_ERROR, this);
		return false;
	}

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
// Function:		OnUnitDoubleClick
//
// Description:		Handles unit list box double click events (allows editing
//					of existing units).
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
void OptionsDialog::OnUnitDoubleClick(wxCommandEvent& WXUNUSED(event))
{
	XMLConversionFactors::Equivalence unitInfo = GetUnitInfo(groupList->GetString(
		groupList->GetSelection()), unitList->GetString(unitList->GetSelection()));
	wxString groupName = groupList->GetString(groupList->GetSelection());
	wxString newConversion;

	try
	{
		AddUnitDialog dialog(this, groupName, GetCompleteUnitList(groupName), unitInfo);
		if (dialog.ShowModal() != wxID_OK)
			return;

		newConversion = _T("a=b") + dialog.GetConversionFactor();
	}
	catch (ErrorCodes &e)
	{
		if (e == errorHasErrors)
			wxMessageBox(_T("Expression contains errors.  This must be corrected by opening the XML definitions direclty."),
				_T("Cannot Edit Unit"), 5L, this);
		else if (e == errorTooComplicated)
			wxMessageBox(_T("Expression is too complicated.  Editing must be done by opening the XML definitions direclty."),
				_T("Cannot Edit Unit"), 5L, this);
		else
			wxMessageBox(wxString::Format(_T("Unknown error (%i) occured."), e),
				_T("Cannot Edit Unit"), 5L, this);

		return;
	}
	catch (...)
	{
		wxMessageBox(_T("Unknown error occured."), _T("Cannot Edit Unit"), 5L, this);
		return;
	}

	// Upon return, update values if necessary
	if (newConversion.Cmp(unitInfo.equation) != 0)
	{
		// Handle differently depending on whether or not the unit is already in the XML document
		unsigned int i;
		for (i = 0; i < newUnits.size(); i++)
		{
			if (newUnits[i].first.Cmp(groupName) == 0 &&
				newUnits[i].second.aUnit.Cmp(unitInfo.aUnit) == 0 &&
				newUnits[i].second.bUnit.Cmp(unitInfo.bUnit) == 0)
			{
				newUnits[i].second.equation = newConversion;
				return;
			}
		}

		// Check to see if we just changed our minds about how to change and existing unit
		for (i = 0; i < changedUnits.size(); i++)
		{
			if (changedUnits[i].first.Cmp(groupName) == 0 &&
				changedUnits[i].second.aUnit.Cmp(unitInfo.aUnit) == 0 &&
				changedUnits[i].second.bUnit.Cmp(unitInfo.bUnit) == 0)
			{
				changedUnits[i].second.equation = newConversion;
				return;
			}
		}

		// Didn't find anything in the new units list, need to update the existing unit in XML
		unitInfo.equation = newConversion;
		changedUnits.push_back(std::make_pair(groupName, unitInfo));
	}
}

//==========================================================================
// Class:			OptionsDialog
// Function:		GetUnitInfo
//
// Description:		Returns the information known about the specified unit.
//
// Input Arguments:
//		groupName	= const wxString&
//		unitName	= const wxString&
//
// Output Arguments:
//		None
//
// Return Value:
//		XMLConversionFactors::Equivalence
//
//==========================================================================
XMLConversionFactors::Equivalence OptionsDialog::GetUnitInfo(const wxString &groupName,
	const wxString &unitName) const
{
	// Check units that we've previously selected to change, first
	unsigned int i, j;
	for (i = 0; i < changedUnits.size(); i++)
	{
		if (changedUnits[i].first.Cmp(groupName) == 0 && 
			changedUnits[i].second.aUnit.Cmp(unitName) == 0)
			return changedUnits[i].second;
	}

	// or did we match B?
	for (i = 0; i < changedUnits.size(); i++)
	{
		if (changedUnits[i].first.Cmp(groupName) == 0 && 
			changedUnits[i].second.bUnit.Cmp(unitName) == 0)
			return changedUnits[i].second;
	}

	for (i = 0; i < xml.GroupCount(); i++)
	{
		if (xml.GetGroup(i).name.Cmp(groupName) == 0)
		{
			// Check all A units first
			for (j = 0; j < xml.GetGroup(i).equiv.size(); j++)
			{
				if (xml.GetGroup(i).equiv[j].aUnit.Cmp(unitName) == 0)
					return xml.GetGroup(i).equiv[j];
			}

			for (j = 0; j < xml.GetGroup(i).equiv.size(); j++)
			{
				if (xml.GetGroup(i).equiv[j].bUnit.Cmp(unitName) == 0)
					return xml.GetGroup(i).equiv[j];
			}
		}
	}

	// If we didn't return yet, the group and/or unit must be new
	for (i = 0; i < newUnits.size(); i++)
	{
		if (newUnits[i].first.Cmp(groupName) == 0 &&
			newUnits[i].second.aUnit.Cmp(unitName) == 0)
			return newUnits[i].second;
	}

	// or did we match B?
	for (i = 0; i < newUnits.size(); i++)
	{
		if (newUnits[i].first.Cmp(groupName) == 0 &&
			newUnits[i].second.bUnit.Cmp(unitName) == 0)
			return newUnits[i].second;
	}

	throw;
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
		XMLConversionFactors::FactorGroup group = xml.GetGroup(groupName);// TODO:  catch exceptions
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
	: wxDialog(parent, wxID_ANY, _T("Add ") + groupName + _T(" Unit")), isUnitUpdate(false)
{
	CreateControls(unitList);
	CenterOnParent();
}

//==========================================================================
// Class:			AddUnitDialog
// Function:		AddUnitDialog
//
// Description:		Constructor for AddUnitDialog class; additional arguments
//					for editing existing units.
//
// Input Arguments:
//		parent				= wxWindow*
//		groupName			= const wxString&
//		unitList			= const wxArrayString&
//		unitInfo			= const wxString&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
OptionsDialog::AddUnitDialog::AddUnitDialog(wxWindow *parent, const wxString &groupName,
			const wxArrayString &unitList, const XMLConversionFactors::Equivalence &unitInfo)
			: wxDialog(parent, wxID_ANY, _T("Add ") + groupName + _T(" Unit")), isUnitUpdate(true)
{
	CreateControls(unitList);

	unit->ChangeValue(unitInfo.aUnit);
	equivalentUnit->SetValue(unitInfo.bUnit);
	SetQuantitiesFromRelationshipString(unitInfo.equation);

	// We don't want the user changing the unit names here, in case we would end-up breaking links between two other types of units
	// FIXME:  Better way to handle this?
	unit->Enable(false);
	equivalentUnit->Enable(false);

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
// Class:			AddUnitDialog
// Function:		SetQuantitiesFromRelationshipString
//
// Description:		Parses the relationship string to extract quantities for
//					A and B text boxes.
//
// Input Arguments:
//		relationship	= const wxString&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void OptionsDialog::AddUnitDialog::SetQuantitiesFromRelationshipString(const wxString &relationship)
{
	if (RelationshipHasErrors(relationship))
		throw errorHasErrors;
	if (RelationshipIsTooComplicated(relationship))
		throw errorTooComplicated;

	// We already performed checks to ensure a and b are in numerator, and there are no parentheses
	// Find appropriate scaling factors based on this assumption
	wxString left = relationship.Mid(0, relationship.Find(_T("=")));
	if (left.Len() < 1)
		throw errorHasErrors;

	wxString right = relationship.Mid(relationship.Find(_T("=")) + 1);
	if (right.Len() < 1)
		throw errorHasErrors;

	std::vector<wxString> aTerms, bTerms;
	bool swap(false), foundA(false), foundB(false);
	wxChar found;

	found = ParseEquationHalf(left, aTerms, bTerms);

	if (found == 'a')
		foundA = true;
	else if (found == 'b')
	{
		foundB = true;
		swap = true;
	}
	else
		throw errorHasErrors;

	found = ParseEquationHalf(right, bTerms, aTerms);

	if ((foundA && found != 'b') ||
		(foundB && found != 'a'))
		throw errorHasErrors;

	if (swap)
	{
		aQtyText->SetValue(MultiplyTerms(bTerms));
		bQtyText->SetValue(MultiplyTerms(aTerms));
	}
	else
	{
		aQtyText->SetValue(MultiplyTerms(aTerms));
		bQtyText->SetValue(MultiplyTerms(bTerms));
	}
}

//==========================================================================
// Class:			AddUnitDialog
// Function:		ParseEquationHalf
//
// Description:		Parses half of the equation into the appropriate vectors.
//
// Input Arguments:
//		half	= const wxString&
//
// Output Arguments:
//		same	= std::vector<wxString>& for factors on the same side as the half
//		opp		= std::vector<wxString>& for factors on the opposite side from the half
//
// Return Value:
//		wxChar, either 'a', 'b', or ' ' to indicate whether this half found a, b, or neither
//
//==========================================================================
wxChar OptionsDialog::AddUnitDialog::ParseEquationHalf(const wxString &half,
	std::vector<wxString> &same, std::vector<wxString> &opp) const
{
	bool foundVar(false);
	wxChar lastOperator(' '), var(' ');
	unsigned int start(0), i;

	for (i = 0; i < half.Len(); i++)
	{
		// If the next character is an operator or end of string, process the number
		if (half[i] == 'a')
		{
			if (foundVar)
				throw errorHasErrors;

			var = 'a';
			foundVar = true;

			if (half.Len() - i < 3)
				break;

			i++;
			lastOperator = half[i];
			start = i + 1;
		}
		else if (half[i] == 'b')
		{
			if (foundVar)
				throw errorHasErrors;

			var = 'b';
			foundVar = true;

			if (half.Len() - i < 3)
				break;

			i++;
			lastOperator = half[i];
			start = i + 1;
		}
		else if (i == half.Len() - 1)// end of string
		{
			if (lastOperator == '/')
				same.push_back(half.Mid(start));
			else
				opp.push_back(half.Mid(start));
		}
		else if (half[i] == '*')
		{
			if (lastOperator == '/')
				same.push_back(half.Mid(start, i - start));
			else
				opp.push_back(half.Mid(start, i - start));

			start = i + 1;
			lastOperator = '*';
		}
		else if (half[i] == '/')
		{
			if (lastOperator == '/')
				same.push_back(half.Mid(start, i - start));
			else
				opp.push_back(half.Mid(start, i - start));

			start = i + 1;
			lastOperator = '/';
		}
	}

	return var;
}

//==========================================================================
// Class:			AddUnitDialog
// Function:		MultiplyTerms
//
// Description:		Generates a string representing the product of the vector elements.
//
// Input Arguments:
//		terms	= const std::vector<wxString>&
//
// Output Arguments:
//		None
//
// Return Value:
//		wxString
//
//==========================================================================
wxString OptionsDialog::AddUnitDialog::MultiplyTerms(const std::vector<wxString> &terms) const
{
	if (terms.size() == 0)
		return _T("1");
	else if (terms.size() == 1)
		return terms[0];

	double v, result(1.0);
	unsigned int i;
	for (i = 0; i < terms.size(); i++)
	{
		if (!terms[i].ToDouble(&v))
			throw errorHasErrors;

		result *= v;
	}

	return wxString::Format(_T("%f"), result);
}

//==========================================================================
// Class:			AddUnitDialog
// Function:		RelationshipHasErrors
//
// Description:		Checks for some simple errors.
//
// Input Arguments:
//		relationship	= const wxString&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
bool OptionsDialog::AddUnitDialog::RelationshipHasErrors(const wxString &relationship) const
{
	if (relationship.Contains(_T("**")) ||
		relationship.Contains(_T("//")) ||
		relationship.Contains(_T("*/")) ||
		relationship.Contains(_T("/*")) ||
		relationship.Contains(_T("\\")) ||
		!relationship.Contains(_T("=")) ||
		!relationship.Contains(_T("a")) ||
		!relationship.Contains(_T("b")))
		return true;

	return false;
}

//==========================================================================
// Class:			AddUnitDialog
// Function:		RelationshipIsTooComplicated
//
// Description:		Determines if the relationship is parseable with our simple logic.
//
// Input Arguments:
//		relationship	= const wxString&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
bool OptionsDialog::AddUnitDialog::RelationshipIsTooComplicated(const wxString &relationship) const
{
	if (relationship.Contains(_T("/a")) ||
		relationship.Contains(_T("/b")) ||
		relationship.Contains(_T(" ")) ||
		relationship.Contains(_T("(")) ||
		relationship.Contains(_T(")")))
		return true;

	if (relationship.Contains(_T("+")))
	{
		int plus((unsigned int)-1);
		while (plus = relationship.find(_T("+"), plus + 1), plus != wxNOT_FOUND)
		{
			if (plus > 0 && relationship.Mid(plus - 1, 1).CmpNoCase(_T("e")) != 0)
				return true;
		}
	}

	if (relationship.Contains(_T("-")))
	{
		int minus((unsigned int)-1);
		while (minus = relationship.find(_T("-"), minus + 1), minus != wxNOT_FOUND)
		{
			if (minus > 0 && relationship.Mid(minus - 1, 1).CmpNoCase(_T("e")) != 0)
				return true;
		}
	}

	return false;
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

	if (!isUnitUpdate)
	{
		unsigned int i;
		for (i = 0; i < equivalentUnit->GetCount(); i++)
		{
			if (unit->GetValue().Cmp(equivalentUnit->GetString(i)) == 0)
			{
				wxMessageBox(_T("Unit '") + unit->GetValue() + _T("' already exists!"), _T("Error"), wxICON_ERROR, this);
				return false;
			}
		}
	}

	return true;
}
