/*===================================================================================
                                       Converter
                              Copyright Kerry R. Loux 2013

     No requirement for distribution of wxWidgets libraries, source, or binaries.
                             (http://www.wxwidgets.org/)

===================================================================================*/

// File:  xmlConversionFactors.h
// Created:  5/31/2013
// Author:  K. Loux
// Description:  Interface and container for xml-based conversion definitions.
// History:

#ifndef _XML_CONVERSION_FACTORS_H_
#define _XML_CONVERSION_FACTORS_H_

// Standard C++ headers
#include <vector>

// wxWidgets headers
#include <wx/wx.h>

// wxWidgets forward declarations
class wxXmlDocument;
class wxXmlNode;

class XMLConversionFactors
{
public:
	XMLConversionFactors(const wxString &fileName);
	~XMLConversionFactors();

	bool Load(void);
	void Save(void) const;

	class Equivalence
	{
	public:
		wxString aUnit, bUnit;
		wxString equation;

		wxXmlNode* ToXmlNode(void) const;
	};

	class FactorGroup
	{
	public:
		wxString name;
		bool display;

		std::vector<Equivalence> equiv;

		wxArrayString GetUnitList(const bool &sort = true) const;
	};

	unsigned int GroupCount(void) const { return groups.size(); };
	FactorGroup GetGroup(const unsigned int &i) const { return groups[i]; };
	FactorGroup GetGroup(const wxString &name) const;

	void AddGroup(const wxString &name);
	void AddEquivalence(const wxString &name, const Equivalence &e);
	void SetGroupVisibility(const wxString &name, const bool &visible);

private:
	const wxString fileName;
	wxXmlDocument *document;

	void ResetForLoad(void);
	void AddNodePreserveFormatting(wxXmlNode *parent, wxXmlNode *child, const bool &alphabetize = false) const;
	bool GetNodeAfterAlphabetically(const wxString &name, wxXmlNode *parent, wxXmlNode *&nodeAfterChild) const;

	static wxXmlNode* GetNewLineNode(void);
	static wxXmlNode* GetIndentNode(const unsigned int &level);

	wxXmlNode* GetGroupNode(const wxString &name);

	std::vector<FactorGroup> groups;

	bool ReadGroupNode(wxXmlNode *node);
	bool ReadEquivNode(wxXmlNode *node, Equivalence &equiv);

	// XML Tags
	static const wxString rootName;
	static const wxString groupNode;
	static const wxString nameAttr;
	static const wxString displayAttr;
	static const wxString equivNode;
	static const wxString aUnitAttr;
	static const wxString bUnitAttr;
	static const wxString equationAttr;

	bool DuplicateGroupsExist(void) const;

	void DoErrorMessage(const wxString &message) const;
};

#endif// _XML_CONVERSION_FACTORS_H_