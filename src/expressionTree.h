/*===================================================================================
                                       Converter
                              Copyright Kerry R. Loux 2013

  This code is licensed under the MIT License (http://opensource.org/licenses/MIT).

===================================================================================*/

// File:  expressionTree.h
// Created:  5/6/2011
// Author:  K. Loux
// Description:  Handles user-specified mathematical operations on datasets.
// History:

#ifndef _EXPRESSION_TREE_H_
#define _EXPRESSION_TREE_H_

// Standard C++ headers
#include <queue>
#include <stack>
#include <string>

// wxWidgets headers
#include <wx/wx.h>

class ExpressionTree
{
public:
	// Main solver method
	wxString Solve(wxString expression, double &result);
	wxString SolveForString(wxString expression, const wxString &x, wxString &result);

	static bool Clean(wxString &term, const wxString &x);

private:
	static const unsigned int printfPrecision;

	std::queue<wxString> outputQueue;

	wxString ParseExpression(const wxString &expression);
	wxString ParseNext(const wxString &expression, bool &lastWasOperator,
		unsigned int &advance, std::stack<wxString> &operatorStack);
	wxString EvaluateExpression(double &results);

	void ProcessOperator(std::stack<wxString> &operatorStack, const wxString &s);
	void ProcessCloseParenthese(std::stack<wxString> &operatorStack);

	static bool NextIsNumber(const wxString &s, unsigned int *stop = NULL, const bool &lastWasOperator = true);
	static bool NextIsOperator(const wxString &s, unsigned int *stop = NULL);
	static bool NextIsVariable(const wxString &s, unsigned int *stop = NULL);

	static size_t FindEndOfNextTerm(const wxString &s, const unsigned int &start);

	bool IsLeftAssociative(const wxChar &c) const;
	bool OperatorShift(const wxString &stackString, const wxString &newString) const;

	void PopStackToQueue(std::stack<wxString> &stack);
	bool EmptyStackToQueue(std::stack<wxString> &stack);
	unsigned int GetPrecedence(const wxString &s) const;

	double PopFromStack(std::stack<double> &stack) const;
	double ApplyOperation(const wxString &operation, const double &first, const double &second) const;

	bool EvaluateNext(const wxString &next, std::stack<double> &stack, wxString &errorString) const;
	bool EvaluateNumber(const wxString &number, std::stack<double> &stack, wxString &errorString) const;
	bool EvaluateOperator(const wxString &operation, std::stack<double> &stack, wxString &errorString) const;
	bool EvaluateUnaryOperator(const wxString &operation, std::stack<double> &stack, wxString &errorString) const;

	bool ParenthesesBalanced(const wxString &expression) const;

	static wxArrayString BreakApartTerms(const wxString &s);

	bool SeparateSides(const wxString &e, wxString &lhs, wxString &rhs) const;
	bool CombinedTerm(const wxString &term) const;
	wxString InvertTerm(const wxString &term) const;
	wxString AssembleTerms(const wxArrayString &terms) const;
	wxString CrossMultiplySimplify(wxString &lhs, wxArrayString &rhs, const wxString &x) const;
};

#endif// _EXPRESSION_TREE_H_