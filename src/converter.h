/*===================================================================================
                                       Converter
                              Copyright Kerry R. Loux 2013

  This code is licensed under the MIT License (http://opensource.org/licenses/MIT).

===================================================================================*/

// File:  converter.h
// Created:  6/3/2013
// Author:  K. Loux
// Description:  Object for performing conversions.
// History:

#ifndef _CONVERTER_H_
#define _CONVERTER_H_

// Standard C++ headers
#include <unordered_map>
#include <string>
#include <set>

// Local headers
#include "xmlConversionFactors.h"

class Converter
{
public:
	Converter(const XMLConversionFactors &xml);
	~Converter();

	double Convert(const wxString &group, const wxString &inUnit,
		const wxString &outUnit, const double &value);

private:
	const XMLConversionFactors &xml;

	std::unordered_map<std::string, wxString> conversions;

	static double EvaluateConversion(const double &value, wxString conversionString);
	wxString GetConversion(const wxString &group, const wxString &inUnit,
		const wxString &outUnit);
	static std::string GetConversionCode(const wxString &group, const wxString &inUnit,
		const wxString &outUnit);

	wxString CreateConversion(const wxString &group, const wxString &inUnit,
		const wxString &outUnit) const;
	wxString FindConversionPath(const XMLConversionFactors::FactorGroup &group,
		const wxString &inUnit, const wxString &outUnit) const;

	static wxString SolveForA(const wxString &e);
	static wxString SolveForB(const wxString &e);

	class ConversionGraph
	{
	public:
		ConversionGraph(const XMLConversionFactors::FactorGroup &group);
		~ConversionGraph();

		class GraphNode
		{
		public:
			GraphNode(const wxString &name);
			wxString name;
			wxString path;
			bool visited;
			void AddNeighbor(GraphNode *n) { neighbors.insert(n); };
			std::set<GraphNode*>& GetNeighbors(void) { return neighbors; };

		private:
			std::set<GraphNode*> neighbors;
		};

		GraphNode* GetNode(const wxString &name);
		std::set<GraphNode*> GetNodes(void) const { return nodes; };

	private:
		std::set<GraphNode*> nodes;
	};
};

#endif// _CONVERTER_H_