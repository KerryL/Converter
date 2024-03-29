/*===================================================================================
                                       Converter
                              Copyright Kerry R. Loux 2013

  This code is licensed under the MIT License (http://opensource.org/licenses/MIT).

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
#include <memory>

// wxWidgets headers
#include <wx/wx.h>
#include <wx/xml/xml.h>

class XMLConversionFactors
{
public:
	XMLConversionFactors(const wxString &fileName);
	~XMLConversionFactors() = default;

	bool Load();
	void Save() const;

	class Equivalence
	{
	public:
		wxString aUnit, bUnit;
		wxString equation;

		wxXmlNode* ToXmlNode() const;
	};

	class FactorGroup
	{
	public:
		wxString name;
		bool display;

		std::vector<Equivalence> equiv;

		wxArrayString GetUnitList(const bool &sort = true) const;
	};

	unsigned int GroupCount() const { return groups.size(); };
	FactorGroup GetGroup(const unsigned int &i) const { return groups[i]; };
	FactorGroup GetGroup(const wxString &name) const;

	void AddGroup(const wxString &name);
	void AddEquivalence(const wxString &name, const Equivalence &e);
	void ChangeEquivalence(const wxString &name, const Equivalence &e);
	void SetGroupVisibility(const wxString &name, const bool &visible);

	static const wxString xmlEncoding;
	wxString GetFileName() const { return fileName; };

private:
	const wxString fileName;
	std::unique_ptr<wxXmlDocument> document;

	void ResetForLoad();
	void AddNodePreserveFormatting(wxXmlNode *parent, wxXmlNode *child, const bool &alphabetize = false) const;
	bool GetNodeAfterAlphabetically(const wxString &name, wxXmlNode *parent, wxXmlNode *&nodeAfterChild) const;

	static wxXmlNode* GetNewLineNode();
	static wxXmlNode* GetIndentNode(const unsigned int &level);
	
	bool CreateEmptyDocument();

	wxXmlNode* GetGroupNode(const wxString &name);

	std::vector<FactorGroup> groups;

	bool ReadGroupNode(wxXmlNode *node);
	bool ReadEquivNode(wxXmlNode *node, Equivalence &equiv);

	// XML Tags
	static const wxString rootName;
	static const wxString groupNodeStr;
	static const wxString nameAttr;
	static const wxString displayAttr;
	static const wxString equivNode;
	static const wxString aUnitAttr;
	static const wxString bUnitAttr;
	static const wxString equationAttr;

	bool DuplicateGroupsExist() const;

	void DoErrorMessage(const wxString &message) const;
};

#endif// _XML_CONVERSION_FACTORS_H_
