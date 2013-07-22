/*===================================================================================
                                       Converter
                              Copyright Kerry R. Loux 2013

     No requirement for distribution of wxWidgets libraries, source, or binaries.
                             (http://www.wxwidgets.org/)

===================================================================================*/

// File:  optionsDialog.h
// Created:  6/5/2013
// Author:  K. Loux
// Description:  Dialog for configuring application options.
// History:

#ifndef _OPTIONS_DIALOG_H_
#define _OPTIONS_DIALOG_H_

// Standard C++ headers
#include <vector>

// wxWidgets headers
#include <wx/wx.h>

// Local headers
#include "xmlConversionFactors.h"

// wxWidgets forward declarations
class wxNotebook;

class OptionsDialog : public wxDialog
{
public:
	OptionsDialog(wxWindow *parent, XMLConversionFactors &xml);

private:
	XMLConversionFactors &xml;
	void CreateControls(void);

	wxCheckListBox *groupList;
	wxListBox *unitList;

	enum ControlIds
	{
		idNewGroup = wxID_HIGHEST + 100,
		idNewUnit,
		idGroups,
		idUnits
	};

	enum ErrorCodes
	{
		errorHasErrors,
		errorTooComplicated
	};

	void OnNewGroup(wxCommandEvent &event);
	void OnNewUnit(wxCommandEvent &event);
	void OnSelectedGroupChange(wxCommandEvent &event);
	void OnUnitDoubleClick(wxCommandEvent &event);

	virtual bool TransferDataFromWindow(void);

	void AddGroupsToControl(void);
	void UpdateUnitList(void);
	wxArrayString GetCompleteUnitList(const wxString &groupName) const;
	wxArrayString GetNewUnits(const wxString &groupName) const;

	unsigned int GetAlphabeticIndex(const wxString &groupName) const;

	std::vector<wxString> newGroups;
	std::vector<std::pair<wxString, XMLConversionFactors::Equivalence> > newUnits;
	std::vector<std::pair<wxString, XMLConversionFactors::Equivalence> > changedUnits;

	XMLConversionFactors::Equivalence GetUnitInfo(const wxString &groupName,
		const wxString &unitName) const;

	DECLARE_EVENT_TABLE();

	class AddUnitDialog : public wxDialog
	{
	public:
		AddUnitDialog(wxWindow *parent, const wxString &groupName,
			const wxArrayString &unitList);
		AddUnitDialog(wxWindow *parent, const wxString &groupName,
			const wxArrayString &unitList, const XMLConversionFactors::Equivalence &unitInfo);

		wxString GetUnitName(void) const;
		wxString GetConversionFactor(void) const;
		wxString GetEquivalentUnitName(void) const;

	private:
		void CreateControls(const wxArrayString &unitList);

		wxTextCtrl *unit;
		wxTextCtrl *aQtyText, *bQtyText;
		wxComboBox *equivalentUnit;
		const bool isUnitUpdate;

		void SetQuantitiesFromRelationshipString(const wxString &relationship);
		bool RelationshipHasErrors(const wxString &relationship) const;
		bool RelationshipIsTooComplicated(const wxString &relationship) const;
		wxChar ParseEquationHalf(const wxString &half, std::vector<wxString> &same,
			std::vector<wxString> &opp) const;
		wxString MultiplyTerms(const std::vector<wxString> &terms) const;

		enum ControlIds
		{
			idAny = wxID_ANY,
			idZeroth = wxID_HIGHEST + 200,
			idFirst,
			idSecond,
			idThird,
			idFourth,
			idFifth,
			idSixth,
			idSeventh,
			idEigth,
			idNinth,
			idNegative,
			idUnit,
			idEquivUnit
		};

		void OnButton(wxCommandEvent &event);
		void OnTextChange(wxCommandEvent &event);

		ControlIds focusedControl;
		long cursorPosition;

		virtual bool TransferDataFromWindow(void);

		DECLARE_EVENT_TABLE();
	};
};

#endif// _OPTIONS_DIALOG_H_