/*===================================================================================
                                       Converter
                              Copyright Kerry R. Loux 2013

  This code is licensed under the MIT License (http://opensource.org/licenses/MIT).

===================================================================================*/

// File:  converter.cpp
// Created:  6/3/2013
// Author:  K. Loux
// Description:  Object for performing conversions.

// Standard C++ headers
#include <queue>
#include <stdexcept>

// Local headers
#include "converter.h"
#include "expressionTree.h"

//==========================================================================
// Class:			Converter
// Function:		Converter
//
// Description:		Constructor for Converter class.
//
// Input Arguments:
//		xml	= const XMLConversionFactors&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
Converter::Converter(const XMLConversionFactors &xml) : xml(xml)
{
	setlocale(LC_ALL, "");// Do this to ensure we can convert unicode strings
}

//==========================================================================
// Class:			Converter
// Function:		~Converter
//
// Description:		Destructor for Converter class.
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
Converter::~Converter()
{
}

//==========================================================================
// Class:			Converter
// Function:		Convert
//
// Description:		Performs the specified consversion.
//
// Input Arguments:
//		group	= const wxString&
//		inUnit	= const wxString&
//		outUnit	= const wxString&
//		value	= const double&
//
// Output Arguments:
//		None
//
// Return Value:
//		double
//
//==========================================================================
double Converter::Convert(const wxString &group, const wxString &inUnit,
	const wxString &outUnit, const double &value)
{
	try
	{
		return EvaluateConversion(value, GetConversion(group, inUnit, outUnit));
	}
	catch (std::exception &e)
	{
		wxString message(_T("Error evaluating conversion!\n\n"));
		message.Append(wxString(e.what(), wxConvUTF8));
		wxMessageBox(message, _T("Error"), wxICON_ERROR);
		return value;
	}
}

//==========================================================================
// Class:			Converter
// Function:		EvaluateConversion
//
// Description:		Evaluates the specified conversion string with the specified
//					input value.
//
// Input Arguments:
//		value				= const double&
//		conversionString	= wxString
//
// Output Arguments:
//		None
//
// Return Value:
//		double
//
//==========================================================================
double Converter::EvaluateConversion(const double &value, wxString conversionString)
{
	ExpressionTree tree;
	double result;
	conversionString.Replace(_T("x"), wxString::Format(_T("%0.15f"), value));
	wxString errors = tree.Solve(conversionString, result);

	if (!errors.IsEmpty())
	{
		wxMessageBox(errors, _T("Evaluation Error"), wxICON_ERROR);
		result = 0.0;
	}

	return result;
}

//==========================================================================
// Class:			Converter
// Function:		GetConversion
//
// Description:		Gets the conversion string for the specified group and units.
//
// Input Arguments:
//		group	= const wxString&
//		inUnit	= const wxString&
//		outUnit	= const wxString&
//
// Output Arguments:
//		None
//
// Return Value:
//		wxString
//
//==========================================================================
wxString Converter::GetConversion(const wxString &group, const wxString &inUnit,
	const wxString &outUnit)
{
	std::string code = GetConversionCode(group, inUnit, outUnit);
	std::map<std::string, wxString>::const_iterator it = conversions.find(code);

	if (it != conversions.end())
		return it->second;

	wxString conversionString(CreateConversion(group, inUnit, outUnit));
	conversions.insert(std::make_pair(code, conversionString));

	return conversionString;
}

//==========================================================================
// Class:			Converter
// Function:		CreateConversion
//
// Description:		Creates a new conversion string for the specified group and units.
//
// Input Arguments:
//		group	= const wxString&
//		inUnit	= const wxString&
//		outUnit	= const wxString&
//
// Output Arguments:
//		None
//
// Return Value:
//		wxString
//
//==========================================================================
wxString Converter::CreateConversion(const wxString &group, const wxString &inUnit,
		const wxString &outUnit) const
{
	unsigned int i;
	wxString expression;
	for (i = 0; i < xml.GroupCount(); i++)
	{
		if (xml.GetGroup(i).name.Cmp(group) == 0)
		{
			expression = FindConversionPath(xml.GetGroup(i), inUnit, outUnit);
			break;
		}
	}

	return expression;
}

//==========================================================================
// Class:			Converter
// Function:		SolveForA
//
// Description:		Solves the specified expression for "a".
//
// Input Arguments:
//		e	= const wxString& expression to solve
//
// Output Arguments:
//		None
//
// Return Value:
//		wxString, expression (not equation) representing "a"
//
//==========================================================================
wxString Converter::SolveForA(const wxString &e)
{
	ExpressionTree t;
	wxString result, errorString;
	errorString = t.SolveForString(e, _T("a"), result);

	if (!errorString.IsEmpty())
	{
		wxMessageBox(errorString, _T("Error"), wxICON_ERROR);
		return wxEmptyString;
	}

	return result;
}

//==========================================================================
// Class:			Converter
// Function:		SolveForB
//
// Description:		Solves the specified expression for "b".
//
// Input Arguments:
//		e	= const wxString& expression to solve
//
// Output Arguments:
//		None
//
// Return Value:
//		wxString, expression (not equation) representing "b"
//
//==========================================================================
wxString Converter::SolveForB(const wxString &e)
{
	ExpressionTree t;
	wxString result, errorString;
	errorString = t.SolveForString(e, _T("b"), result);

	if (!errorString.IsEmpty())
	{
		wxMessageBox(errorString, _T("Error"), wxICON_ERROR);
		return wxEmptyString;
	}

	return result;
}

//==========================================================================
// Class:			Converter
// Function:		FindConversionPath
//
// Description:		Performs a reverse breadth-first search to find the shortest
//					conversion path from inUnit to outUnit.
//
// Input Arguments:
//		group	= const wxString&
//		inUnit	= const wxString&
//		outUnit	= const wxString&
//
// Output Arguments:
//		None
//
// Return Value:
//		wxString
//
//==========================================================================
wxString Converter::FindConversionPath(const XMLConversionFactors::FactorGroup &group,
	const wxString &inUnit, const wxString &outUnit) const
{
	ConversionGraph graph(group);
	std::queue<Converter::ConversionGraph::GraphNode*> q;
	Converter::ConversionGraph::GraphNode* n;
	q.push(graph.GetNode(outUnit));
	wxASSERT(q.front());
	q.front()->visited = true;
	unsigned int j;
	std::set<Converter::ConversionGraph::GraphNode*>::iterator i;
	while (!q.empty())
	{
		n = q.front();
		q.pop();
		if (n->name.Cmp(inUnit) == 0)
			return n->path;

		for (i = n->GetNeighbors().begin(); i != n->GetNeighbors().end(); i++)
		{
			if (!(*i)->visited)
			{
				(*i)->visited = true;
				(*i)->path = n->path;
				for (j = 0; j < group.equiv.size(); j++)
				{
					if (group.equiv[j].aUnit.Cmp((*i)->name) == 0 &&
						group.equiv[j].bUnit.Cmp(n->name) == 0)
					{
						(*i)->path.Replace(_T("x"), _T("(") + SolveForB(group.equiv[j].equation) + _T(")"));
						(*i)->path.Replace(_T("a"), _T("x"));
						break;
					}
					else if (group.equiv[j].bUnit.Cmp((*i)->name) == 0 &&
						group.equiv[j].aUnit.Cmp(n->name) == 0)
					{
						(*i)->path.Replace(_T("x"), _T("(") + SolveForA(group.equiv[j].equation) + _T(")"));
						(*i)->path.Replace(_T("b"), _T("x"));
						break;
					}
				}

				ExpressionTree::Clean((*i)->path, _T("x"));
				q.push(*i);
			}
		}
	}

	wxString errorMessage(_T("Could not find path from '") + inUnit
		+ _T("' to '") + outUnit + _T("'.\nCheck that ") + xml.GetFileName()
		+ _T(" is encoded as ") + xml.xmlEncoding + _T("."));
	throw std::runtime_error(std::string(errorMessage.mb_str()));
}

//==========================================================================
// Class:			Converter
// Function:		GetConversionCode
//
// Description:		Generates the conversion code from the group and units.
//
// Input Arguments:
//		group	= const wxString&
//		inUnit	= const wxString&
//		outUnit	= const wxString&
//
// Output Arguments:
//		None
//
// Return Value:
//		wxString
//
//==========================================================================
std::string Converter::GetConversionCode(const wxString &group, const wxString &inUnit,
	const wxString &outUnit)
{
	wxString code(group + _T(":") + inUnit + _T("->") + outUnit);
	wxASSERT(!std::string(code.mb_str()).empty());
	return std::string(code.mb_str());
}

//==========================================================================
// Class:			Converter::ConversionGraph
// Function:		ConversionGraph
//
// Description:		Constructor; object representing the relationships from
//					one unit to another.
//
// Input Arguments:
//		group	= const XMLConversionFactors::FactorGroup&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
Converter::ConversionGraph::ConversionGraph(const XMLConversionFactors::FactorGroup &group)
{
	GraphNode *node;
	wxArrayString unitList = group.GetUnitList();
	unsigned int i, j;
	for (i = 0; i < unitList.Count(); i++)
	{
		node = GetOrCreateNode(unitList[i]);

		for (j = 0; j < group.equiv.size(); j++)
		{
			if (group.equiv[j].aUnit.Cmp(node->name) == 0)
				node->AddNeighbor(GetOrCreateNode(group.equiv[j].bUnit));
			else if (group.equiv[j].bUnit.Cmp(node->name) == 0)
				node->AddNeighbor(GetOrCreateNode(group.equiv[j].aUnit));
		}
	}
}

//==========================================================================
// Class:			Converter::ConversionGraph
// Function:		~ConversionGraph
//
// Description:		Destructor; object representing the relationships from
//					one unit to another.
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
Converter::ConversionGraph::~ConversionGraph()
{
	std::set<GraphNode*>::iterator i;
	for (i = nodes.begin(); i != nodes.end(); i++)
		delete *i;
}

//==========================================================================
// Class:			Converter::ConversionGraph
// Function:		GetOrCreateNode
//
// Description:		Finds node with matching name.  Creates a new node if no
//					match is found.
//
// Input Arguments:
//		name	= const wxString&
//
// Output Arguments:
//		None
//
// Return Value:
//		Converter::ConversionGraph::GraphNode*
//
//==========================================================================
Converter::ConversionGraph::GraphNode*
	Converter::ConversionGraph::GetOrCreateNode(const wxString &name)
{
	GraphNode *node = GetNode(name);

	if (!node)
	{
		node = new GraphNode(name);
		nodes.insert(node);
	}

	return node;
}

//==========================================================================
// Class:			Converter::ConversionGraph
// Function:		GetNode
//
// Description:		Finds node with matching name.
//
// Input Arguments:
//		name	= const wxString&
//
// Output Arguments:
//		None
//
// Return Value:
//		Converter::ConversionGraph::GraphNode*
//
//==========================================================================
Converter::ConversionGraph::GraphNode*
	Converter::ConversionGraph::GetNode(const wxString &name) const
{
	std::set<GraphNode*>::iterator i;
	for (i = nodes.begin(); i != nodes.end(); i++)
	{
		if ((*i)->name.Cmp(name) == 0)
			return *i;
	}

	return NULL;
}

//==========================================================================
// Class:			Converter::ConversionGraph::GraphNode
// Function:		GraphNode
//
// Description:		Constructor for GraphNode class.
//
// Input Arguments:
//		name	= const wxString&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
Converter::ConversionGraph::GraphNode::GraphNode(const wxString &name) : name(name)
{
	visited = false;
	path = _T("x");
}
