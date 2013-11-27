/*===================================================================================
                                       Converter
                              Copyright Kerry R. Loux 2013

  This code is licensed under the MIT License (http://opensource.org/licenses/MIT).

===================================================================================*/

// File:  xmlConversionFactors.cpp
// Created:  5/31/2013
// Author:  K. Loux
// Description:  Interface and container for xml-based conversion definitions.
// History:

// Standard C++ headers
#include <stdexcept>

// wxWidgets
#include <wx/xml/xml.h>

// Local headers
#include "xmlConversionFactors.h"

//==========================================================================
// Class:			XMLConversionFactors
// Function:		Constant Definitions
//
// Description:		Constants for the XMLConversionFactors class.
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
const wxString XMLConversionFactors::rootName(_T("CONVERSIONS"));
const wxString XMLConversionFactors::groupNode(_T("GROUP"));
const wxString XMLConversionFactors::nameAttr(_T("NAME"));
const wxString XMLConversionFactors::displayAttr(_T("DISPLAY"));
const wxString XMLConversionFactors::equivNode(_T("EQUIV"));
const wxString XMLConversionFactors::aUnitAttr(_T("A_UNIT"));
const wxString XMLConversionFactors::bUnitAttr(_T("B_UNIT"));
const wxString XMLConversionFactors::equationAttr(_T("RELATION"));

//==========================================================================
// Class:			XMLConversionFactors
// Function:		XMLConversionFactors
//
// Description:		Constructor for XMLConversionFactors class.
//
// Input Arguments:
//		fileName	= const wxString&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
XMLConversionFactors::XMLConversionFactors(const wxString &fileName) : fileName(fileName), document(NULL)
{
}

//==========================================================================
// Class:			XMLConversionFactors
// Function:		~XMLConversionFactors
//
// Description:		Destructor for XMLConversionFactors class.
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
XMLConversionFactors::~XMLConversionFactors()
{
	delete document;
}

//==========================================================================
// Class:			XMLConversionFactors
// Function:		Load
//
// Description:		Method for initializing this object.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true for success, false otherwise
//
//==========================================================================
bool XMLConversionFactors::Load(void)
{
	ResetForLoad();

	if (!document->Load(fileName, _T("UTF-8"), wxXMLDOC_KEEP_WHITESPACE_NODES))
		return false;// Error message generated in wx call - don't create additional message

	if (document->GetRoot()->GetName().Cmp(rootName) != 0)
	{
		DoErrorMessage(_T("Root node expected to be '") + rootName + _T("'"));
		return false;
	}

	wxXmlNode *child = document->GetRoot()->GetChildren();
	while (child)
	{
		if (child->GetName().Cmp(groupNode) == 0)
		{
			if (!ReadGroupNode(child))
				return false;
		}
		/*else// Don't do this to allow comments
		{
			DoErrorMessage(_T("Unknown node '") + child->GetName() + _T("'"));
			return false;
		}*/

		child = child->GetNext();
	}

	if (DuplicateGroupsExist())
		return false;

	return groups.size() > 0;
}

//==========================================================================
// Class:			XMLConversionFactors
// Function:		ReadGroupNode
//
// Description:		Reads the specified group node and it's children.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true for success, false otherwise
//
//==========================================================================
bool XMLConversionFactors::ReadGroupNode(wxXmlNode *node)
{
	FactorGroup newGroup;
	if (!node->GetAttribute(nameAttr, &newGroup.name))
	{
		DoErrorMessage(_T("Cannot read '") + nameAttr + _T("' attribute for '") + groupNode + _T("' node"));
		return false;
	}

	newGroup.display = node->GetAttribute(displayAttr, _T("1")).Cmp(_T("1")) == 0;

	wxXmlNode *child = node->GetChildren();
	while (child)
	{
		if (child->GetName().Cmp(equivNode) == 0)
		{
			Equivalence equiv;
			if (!ReadEquivNode(child, equiv))
				return false;
			newGroup.equiv.push_back(equiv);
		}
		/*else// Don't do this to allow comments
		{
			DoErrorMessage(_T("Unknown node '") + child->GetName() + _T("'"));
			return false;
		}*/

		child = child->GetNext();
	}

	if (newGroup.equiv.size() == 0)
	{
		DoErrorMessage(_T("Group '") + newGroup.name + _T("' has no equivalence definitions"));
		return false;
	}

	groups.push_back(newGroup);
	return true;
}

//==========================================================================
// Class:			XMLConversionFactors
// Function:		ReadEquivNode
//
// Description:		Reads the specified equivalence node.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true for success, false otherwise
//
//==========================================================================
bool XMLConversionFactors::ReadEquivNode(wxXmlNode *node, Equivalence &equiv)
{
	if (!node->GetAttribute(aUnitAttr, &equiv.aUnit))
	{
		DoErrorMessage(_T("Cannot read '") + aUnitAttr + _T("' property from '") + equivNode + _T("' node"));
		return false;
	}

	if (!node->GetAttribute(bUnitAttr, &equiv.bUnit))
	{
		DoErrorMessage(_T("Cannot read '") + bUnitAttr + _T("' property from '") + equivNode + _T("' node"));
		return false;
	}

	if (!node->GetAttribute(equationAttr, &equiv.equation))
	{
		DoErrorMessage(_T("Cannot read '") + equationAttr + _T("' property from '") + equivNode + _T("' node"));
		return false;
	}
	if (!equiv.equation.Contains(_T("=")) ||
		!equiv.equation.Contains(_T("a")) ||
		!equiv.equation.Contains(_T("b")))
	{
		DoErrorMessage(_T("Relationship between '") + equiv.aUnit + _T("' and '")
			+ equiv.bUnit + _T("' must containt 'a', 'b', and '='"));
		return false;
	}

	if (equiv.aUnit.Cmp(equiv.bUnit) == 0)
	{
		DoErrorMessage(_T("Equivalence definition must have two unique unit strings"));
		return false;
	}

	return true;
}

//==========================================================================
// Class:			XMLConversionFactors
// Function:		DuplicateGroupsExist
//
// Description:		Determines uniqueness of the groups.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true if two groups have the same name
//
//==========================================================================
bool XMLConversionFactors::DuplicateGroupsExist(void) const
{
	unsigned int i, j;
	for (i = 1; i < groups.size(); i++)
	{
		for (j = i + 1; j < groups.size(); j++)
		{
			if (groups[i].name.CmpNoCase(groups[j].name) == 0)
			{
				DoErrorMessage(_T("Duplicate groups '") + groups[i - 1].name + _T("'"));
				return true;
			}
		}
	}

	return false;
}

//==========================================================================
// Class:			XMLConversionFactors
// Function:		GetGroup
//
// Description:		Gets the group by name.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		XMLConversionFactors::FactorGroup
//
//==========================================================================
XMLConversionFactors::FactorGroup XMLConversionFactors::GetGroup(const wxString &name) const
{
	unsigned int i;
	for (i = 0; i < groups.size(); i++)
	{
		if (groups[i].name.Cmp(name) == 0)
			return groups[i];
	}

	throw new std::runtime_error("Group not found");
}

//==========================================================================
// Class:			XMLConversionFactors
// Function:		DoErrorMessage
//
// Description:		Displays "Error reading..." message.
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
void XMLConversionFactors::DoErrorMessage(const wxString &message) const
{
	wxMessageBox(_T("Error reading XML document:  ") + message + _T("."), _T("Error"), wxICON_ERROR);
}

//==========================================================================
// Class:			XMLConversionFactors
// Function:		AddGroup
//
// Description:		Adds a group to the XML file.
//
// Input Arguments:
//		name	= const wxString &name
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void XMLConversionFactors::AddGroup(const wxString &name)
{
	wxXmlNode *node = new wxXmlNode(wxXML_ELEMENT_NODE, groupNode);
	node->AddAttribute(nameAttr, name);
	node->AddAttribute(displayAttr, _T("1"));
	AddNodePreserveFormatting(document->GetRoot(), node, true);
}

//==========================================================================
// Class:			XMLConversionFactors
// Function:		AddEquivalence
//
// Description:		Adds an equivalence to the specified group.
//
// Input Arguments:
//		name	= const wxString& specifying the group
//		e		= const Equivalence&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void XMLConversionFactors::AddEquivalence(const wxString &name, const Equivalence &e)
{
	wxXmlNode *groupNode = GetGroupNode(name);
	AddNodePreserveFormatting(groupNode, e.ToXmlNode());
}

//==========================================================================
// Class:			XMLConversionFactors
// Function:		ChangeEquivalence
//
// Description:		Changes an existing equivalence.
//
// Input Arguments:
//		name	= const wxString& specifying the group
//		e		= const Equivalence&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void XMLConversionFactors::ChangeEquivalence(const wxString &name, const Equivalence &e)
{
	wxXmlNode *groupNode = GetGroupNode(name);
	wxXmlNode *equiv = groupNode->GetChildren();

	wxString aName, bName;
	while (equiv)
	{
		if (equiv->GetAttribute(aUnitAttr, &aName) &&
			equiv->GetAttribute(bUnitAttr, &bName) &&
			aName.Cmp(e.aUnit) == 0 &&
			bName.Cmp(e.bUnit) == 0)
		{
			equiv->DeleteAttribute(equationAttr);
			equiv->AddAttribute(equationAttr, e.equation);
			return;
		}

		equiv = equiv->GetNext();
	}

	assert(false);// Didn't find a match!
}

//==========================================================================
// Class:			XMLConversionFactors
// Function:		SetGroupVisibility
//
// Description:		Sets the visibility attribute for the specified group.
//
// Input Arguments:
//		name	= const wxString&
//		visible	= const bool&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void XMLConversionFactors::SetGroupVisibility(const wxString &name, const bool &visible)
{
	wxXmlNode *groupNode = GetGroupNode(name);
	groupNode->DeleteAttribute(displayAttr);
	groupNode->AddAttribute(displayAttr, visible ? _T("1") : _T("0"));
}

//==========================================================================
// Class:			XMLConversionFactors
// Function:		AddNodePreserveFormatting
//
// Description:		Adds the specified node as a child to the specified parent,
//					and inserts indentation and newlines as necessary to make
//					it appear just like human-entered nodes.
//
// Input Arguments:
//		parent		= wxXmlNode*
//		child		= wxXmlNode*
//		alphabetize	= const bool&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void XMLConversionFactors::AddNodePreserveFormatting(wxXmlNode *parent, wxXmlNode *child, const bool &alphabetize) const
{
	bool needPreceedingNewLine = parent->GetChildren() == NULL;

	if (alphabetize)
	{
		wxXmlNode *nodeAfterChild(NULL);
		if (GetNodeAfterAlphabetically(child->GetAttribute(nameAttr, wxEmptyString), parent, nodeAfterChild))
			parent->InsertChild(child, nodeAfterChild);
		else
			parent->AddChild(child);
	}
	else
		parent->AddChild(child);

	if (needPreceedingNewLine)
		parent->InsertChild(GetNewLineNode(), child);

	unsigned int depth = child->GetDepth();
	// TODO:  If preceeding node is not a newline (or is whitespace?), adjust depth
	/*if ()
		depth--;*/
	// TODO:  The alphabetizing of groups makes the whitespace a little screwy, too

	parent->InsertChild(GetIndentNode(depth), child);
	parent->InsertChildAfter(GetIndentNode(depth - 1), child);
	parent->InsertChildAfter(GetNewLineNode(), child);
}

//==========================================================================
// Class:			XMLConversionFactors
// Function:		GetNodeAfterAlphabetically
//
// Description:		Finds the node following the node to be added in order to
//					maintain alphabetical entry of nodes.
//
// Input Arguments:
//		name			= const wxString&
//		parent			= wxXmlNode*
//
// Output Arguments:
//		nodeAfterChild	= wxXmlNode*&
//
// Return Value:
//		bool, true if a following node was found OR the child will be the first
//		child for the parent (in the case of this node being the first child,
//		nodeAfterChild will be NULL), false if no following node was found
//		(indicating that the new node should be appended to the end of the
//		parent's children)
//
//==========================================================================
bool XMLConversionFactors::GetNodeAfterAlphabetically(const wxString &name,
	wxXmlNode *parent, wxXmlNode *&nodeAfterChild) const
{
	bool hasChildren = false;
	wxXmlNode *child = parent->GetChildren();
	while (child)
	{
		hasChildren = true;

		if (name.CmpNoCase(child->GetAttribute(nameAttr, wxEmptyString)) < 0)
		{
			nodeAfterChild = child;
			return true;
		}

		child = child->GetNext();
	}

	if (!hasChildren)
	{
		nodeAfterChild = NULL;
		return true;
	}

	return false;
}

//==========================================================================
// Class:			XMLConversionFactors
// Function:		GetNewLineNode
//
// Description:		Generates a new text node representing a new line.
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
wxXmlNode* XMLConversionFactors::GetNewLineNode(void)
{
	return new wxXmlNode(wxXML_TEXT_NODE, wxEmptyString, _T("\n"));
}

//==========================================================================
// Class:			XMLConversionFactors
// Function:		GetIndentNode
//
// Description:		Generates a new text node representing an indent.
//
// Input Arguments:
//		level	= const unsigned int&
//
// Output Arguments:
//		None
//
// Return Value:
//		wxXmlNode*
//
//==========================================================================
wxXmlNode* XMLConversionFactors::GetIndentNode(const unsigned int &level)
{
	unsigned int tabSize(2);
	return new wxXmlNode(wxXML_TEXT_NODE, wxEmptyString, wxString(' ', tabSize * level));
}

//==========================================================================
// Class:			XMLConversionFactors
// Function:		GetGroupNode
//
// Description:		Loads the xml document in a manner that preserves current formatting.
//
// Input Arguments:
//		name	= const wxString&
//
// Output Arguments:
//		None
//
// Return Value:
//		wxXmlNode* pointing to the appropriate node, throws an exception if not found
//
//==========================================================================
wxXmlNode* XMLConversionFactors::GetGroupNode(const wxString &name)
{
	wxXmlNode *child = document->GetRoot()->GetChildren();
	while (child)
	{
		if (child->GetType() == wxXML_ELEMENT_NODE &&
			child->GetName().Cmp(groupNode) == 0)
		{
			if (child->GetAttribute(nameAttr, wxEmptyString).Cmp(name) == 0)
				return child;
		}

		child = child->GetNext();
	}

	wxString errorMessage(_T("Could not find group node for '") + name + _T("' in XML file"));
	throw new std::runtime_error(std::string(errorMessage.mb_str()));

	return NULL;
}

//==========================================================================
// Class:			XMLConversionFactors::Equivalence
// Function:		ToXmlNode
//
// Description:		Converts this object to it's XML representation.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		wxXmlNode*
//
//==========================================================================
wxXmlNode* XMLConversionFactors::Equivalence::ToXmlNode(void) const
{
	wxXmlNode *node = new wxXmlNode(wxXML_ELEMENT_NODE, equivNode);
	node->AddAttribute(aUnitAttr, aUnit);
	node->AddAttribute(bUnitAttr, bUnit);
	node->AddAttribute(equationAttr, equation);

	return node;
}

//==========================================================================
// Class:			XMLConversionFactors::FactorGroup
// Function:		GetUnitList
//
// Description:		Gets an optionally sorted list of all of the unit types in the group.
//
// Input Arguments:
//		sort	= const bool&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
wxArrayString XMLConversionFactors::FactorGroup::GetUnitList(const bool &sort) const
{
	wxArrayString unitList;
	unsigned int i;
	for (i = 0; i < equiv.size(); i++)
	{
		unitList.Add(equiv[i].aUnit);
		unitList.Add(equiv[i].bUnit);
	}
	if (sort)
		unitList.Sort();
	assert(unitList.Count() > 0);
	for (i = 1; i < unitList.Count(); i++)
	{
		if (unitList[i].Cmp(unitList[i - 1]) == 0)
		{
			unitList.RemoveAt(i);
			i--;
		}
	}

	return unitList;
}

//==========================================================================
// Class:			XMLConversionFactors
// Function:		Save
//
// Description:		Saves the specified document using transaction to prevent
//					lost data in the event of an interrupted write.
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
void XMLConversionFactors::Save(void) const
{
	const wxString transactionFileName(_T("~") + fileName);
	document->Save(transactionFileName, wxXML_NO_INDENTATION);
	wxCopyFile(transactionFileName, fileName);
	wxRemoveFile(transactionFileName);
}

//==========================================================================
// Class:			XMLConversionFactors
// Function:		ResetForLoad
//
// Description:		Deletes/clears all objects that are created/populated during the load.
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
void XMLConversionFactors::ResetForLoad(void)
{
	delete document;
	document = new wxXmlDocument;

	groups.clear();
}
