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
#include <map>
#include <string>
#include <set>

// Local headers
#include "xmlConversionFactors.h"

class Converter
{
public:
	Converter(const XMLConversionFactors &xml);
	~Converter() = default;

	double Convert(const wxString &group, const wxString &inUnit,
		const wxString &outUnit, const double &value);
	void ClearCache();

private:
	const XMLConversionFactors &xml;

	std::map<std::string, wxString> conversions;

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
			std::set<GraphNode*>& GetNeighbors() { return neighbors; };

		private:
			std::set<GraphNode*> neighbors;
		};

		GraphNode* GetOrCreateNode(const wxString &name);
		GraphNode* GetNode(const wxString &name) const;
		std::set<GraphNode*> GetNodes() const { return nodes; };

	private:
		std::set<GraphNode*> nodes;
	};
};

#endif// _CONVERTER_H_