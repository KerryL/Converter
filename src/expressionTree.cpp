/*===================================================================================
                                       Converter
                              Copyright Kerry R. Loux 2013

  This code is licensed under the MIT License (http://opensource.org/licenses/MIT).

===================================================================================*/

// File:  expressionTree.cpp
// Created:  5/6/2011
// Author:  K. Loux
// Description:  Handles user-specified mathematical operations on datasets.
// History:

// wxWidgets headers
#include <wx/wx.h>

// Local headers
#include "expressionTree.h"
#include "convertMath.h"

//==========================================================================
// Class:			ExpressionTree
// Function:		Constant Declarations
//
// Description:		Constant declarations for ExpressionTree class.
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
const unsigned int ExpressionTree::printfPrecision = 15;

//==========================================================================
// Class:			ExpressionTree
// Function:		Solve
//
// Description:		Main solving method for the tree.
//
// Input Arguments:
//		expression	= wxString containing the expression to parse
//
// Output Arguments:
//		result		= double& containing the evaluated data
//
// Return Value:
//		wxString, empty for success, error string if unsuccessful
//
//==========================================================================
wxString ExpressionTree::Solve(wxString expression, double &result)
{
	if (!ParenthesesBalanced(expression))
		return _T("Imbalanced parentheses!");

	wxString errorString;
	errorString = ParseExpression(expression);

	if (!errorString.IsEmpty())
		return errorString;

	errorString = EvaluateExpression(result);

	return errorString;
}

//==========================================================================
// Class:			ExpressionTree
// Function:		ParenthesesBalanced
//
// Description:		Checks to see if the expression has balanced parentheses.
//
// Input Arguments:
//		expression	= const wxString&
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true if parentheses are balanced, false otherwise
//
//==========================================================================
bool ExpressionTree::ParenthesesBalanced(const wxString &expression) const
{
	unsigned int leftCount(0), rightCount(0);
	int location = expression.find(_T("("));

	while (location != wxNOT_FOUND)
	{
		leftCount++;
		location = expression.find(_T("("), location + 1);
	}

	location = expression.find(_T(")"));

	while (location != wxNOT_FOUND)
	{
		rightCount++;
		location = expression.find(_T(")"), location + 1);
	}

	if (leftCount != rightCount)
		return false;

	return true;
}

//==========================================================================
// Class:			ExpressionTree
// Function:		ParseExpression
//
// Description:		Parses the expression and produces a queue of Reverse
//					Polish Notation values and operations.  Implements the
//					shunting-yard algorithm as described by Wikipedia.
//
// Input Arguments:
//		expression	= const wxString& to be parsed
//
// Output Arguments:
//		None
//
// Return Value:
//		wxString containing error descriptions or an empty string on success
//
//==========================================================================
wxString ExpressionTree::ParseExpression(const wxString &expression)
{
	std::stack<wxString> operatorStack;
	bool lastWasOperator(true);
	wxString errorString;

	for (size_t i = 0; i < expression.Len(); i++)
	{
		if (expression.Mid(i, 1).Trim().IsEmpty())
			continue;

		unsigned int advance;
		errorString = ParseNext(expression.Mid(i), lastWasOperator, advance, operatorStack);
		if (!errorString.IsEmpty())
			return errorString;
		i += advance - 1;
	}

	if (!EmptyStackToQueue(operatorStack))
		errorString = _T("Imbalanced parentheses!");

	return errorString;
}

//==========================================================================
// Class:			ExpressionTree
// Function:		ParseNext
//
// Description:		Parses the expression and processes the next item.
//
// Input Arguments:
//		expression	= const wxString&
//
// Output Arguments:
//		lastWasOperator	= bool&
//		advance			= unsigned int&
//		operatorStack	= std::stack<wxString>&
//
// Return Value:
//		wxString containing any errors
//
//==========================================================================
wxString ExpressionTree::ParseNext(const wxString &expression, bool &lastWasOperator,
	unsigned int &advance, std::stack<wxString> &operatorStack)
{
	bool thisWasOperator(false);
	if (NextIsNumber(expression, &advance, lastWasOperator))
		outputQueue.push(expression.Mid(0, advance));
	else if (NextIsVariable(expression, &advance))
		outputQueue.push(expression.Mid(0, advance));
	else if (NextIsOperator(expression, &advance))
	{
		ProcessOperator(operatorStack, expression.Mid(0, advance));
		thisWasOperator = true;
	}
	else if (expression[0] == '(')
	{
		if (!lastWasOperator)
			operatorStack.push(_T("*"));
		operatorStack.push(expression[0]);
		advance = 1;
		thisWasOperator = true;
	}
	else if (expression[0] == ')')
	{
		ProcessCloseParenthese(operatorStack);
		advance = 1;
	}
	else
		return _T("Unrecognized character:  '") + expression.Mid(0, 1) + _T("'.");
	lastWasOperator = thisWasOperator;
	return wxEmptyString;
}

//==========================================================================
// Class:			ExpressionTree
// Function:		ProcessOperator
//
// Description:		Processes the next operator in the expression, adding it
//					to the appropriate stack.  This method enforces the order
//					of operations.
//
// Input Arguments:
//		operatorStack	= std::stack<wxString>&
//		s				= const wxString& representing the next operator
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void ExpressionTree::ProcessOperator(std::stack<wxString> &operatorStack, const wxString &s)
{
	// Handle operator precedence
	while (!operatorStack.empty())
	{
		if ((!NextIsOperator(operatorStack.top()) ||
			!OperatorShift(operatorStack.top(), s)))
			break;
		PopStackToQueue(operatorStack);
	}
	operatorStack.push(s);
}

//==========================================================================
// Class:			ExpressionTree
// Function:		ProcessCloseParenthese
//
// Description:		Adjusts the stacks in response to encountering a close
//					parenthese.
//
// Input Arguments:
//		operatorStack	= std::stack<wxString>&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void ExpressionTree::ProcessCloseParenthese(std::stack<wxString> &operatorStack)
{
	while (!operatorStack.empty())
	{
		if (operatorStack.top().Cmp(_T("(")) == 0)
			break;
		PopStackToQueue(operatorStack);
	}

	if (operatorStack.empty())
	{
		assert(false);
		// Should never happen due to prior parenthese balance checks
		//return _T("Imbalanced parentheses!");
	}

	operatorStack.pop();
}

//==========================================================================
// Class:			ExpressionTree
// Function:		EvaluateExpression
//
// Description:		Evaluates the expression in the queue using Reverse Polish
//					Notation.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		results	= double&
//
// Return Value:
//		wxString containing a description of any errors, or wxEmptyString on success
//
//==========================================================================
wxString ExpressionTree::EvaluateExpression(double &results)
{
	wxString next, errorString;

	std::stack<double> stack;

	while (!outputQueue.empty())
	{
		next = outputQueue.front();
		outputQueue.pop();

		if (!EvaluateNext(next, stack, errorString))
			return errorString;
	}

	if (stack.size() > 1)
		return _T("Not enough operators!");
	else if (stack.size() == 0)
		return _T("My numbers disappeared!");

	results = stack.top();

	return wxEmptyString;
}

//==========================================================================
// Class:			ExpressionTree
// Function:		PopStackToQueue
//
// Description:		Removes the top entry of the stack and puts it in the queue.
//
// Input Arguments:
//		stack	= std::stack<wxString>& to be popped
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void ExpressionTree::PopStackToQueue(std::stack<wxString> &stack)
{
	outputQueue.push(stack.top());
	stack.pop();
}

//==========================================================================
// Class:			ExpressionTree
// Function:		EmptyStackToQueue
//
// Description:		Empties the contents of the stack into the queue.
//
// Input Arguments:
//		stack	= std::stack<wxString>& to be emptied
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true for success, false otherwise (imbalance parentheses)
//
//==========================================================================
bool ExpressionTree::EmptyStackToQueue(std::stack<wxString> &stack)
{
	while (!stack.empty())
	{
		if (stack.top().Cmp(_T("(")) == 0)
			return false;
		PopStackToQueue(stack);
	}

	return true;
}

//==========================================================================
// Class:			ExpressionTree
// Function:		NextIsNumber
//
// Description:		Determines if the next portion of the expression is a number.
//					Some cleverness is required to tell the difference between
//					a minus sign and a negative sign (minus sign would return false).
//
// Input Arguments:
//		s				= const wxString& containing the expression
//		lastWasOperator	= const bool& indicating whether or not the last thing
//						  on the stack is an operator
//
// Output Arguments:
//		stop	= unsigned int* (optional) indicating length of number
//
// Return Value:
//		bool, true if a number is next in the expression
//
//==========================================================================
bool ExpressionTree::NextIsNumber(const wxString &s, unsigned int *stop, const bool &lastWasOperator)
{
	if (s.Len() == 0)
		return false;

	bool foundDecimal = s[0] == '.';
	if (foundDecimal ||
		(int(s[0]) >= int('0') && int(s[0]) <= int('9')) ||
		(s[0] == '-' && lastWasOperator && NextIsNumber(s.Mid(1), NULL, false)))
	{
		size_t i;
		for (i = 1; i < s.Len(); i++)
		{
			if (s[i] == '.')
			{
				if (foundDecimal)
					return false;
				foundDecimal = true;
			}
			else if (s[i] == 'e')
			{
				if (i + 1 < s.Len())
				{
					if ((s[i + 1] == '-' || s[i + 1] == '+'))
					{
						i++;
						continue;
					}
				}
				continue;
			}
			else if (int(s[i]) < int('0') || int(s[i]) > int('9'))
				break;
		}

		if (stop)
			*stop = i;
		return true;
	}

	return false;
}

//==========================================================================
// Class:			ExpressionTree
// Function:		NextIsOperator
//
// Description:		Determines if the next portion of the expression is an operator.
//
// Input Arguments:
//		s		= const wxString& containing the expression
//
// Output Arguments:
//		stop	= unsigned int* (optional) indicating length of operator
//
// Return Value:
//		bool, true if an operator is next in the expression
//
//==========================================================================
bool ExpressionTree::NextIsOperator(const wxString &s, unsigned int *stop)
{
	if (s.Len() == 0)
		return false;

	if (s[0] == '+' ||// From least precedence
		s[0] == '-' ||
		s[0] == '*' ||
		s[0] == '/' ||
		s[0] == '%' ||
		s[0] == '^')// To most precedence
	{
		if (stop)
			*stop = 1;
		return true;
	}

	return false;
}

//==========================================================================
// Class:			ExpressionTree
// Function:		OperatorShift
//
// Description:		Determines if the new operator requires a shift in
//					operator placement.
//
// Input Arguments:
//		stackString	= const wxString& containing the expression
//		newString	= const wxString& containing the expression
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true if shifting needs to occur
//
//==========================================================================
bool ExpressionTree::OperatorShift(const wxString &stackString, const wxString &newString) const
{
	unsigned int stackPrecedence = GetPrecedence(stackString);
	unsigned int newPrecedence = GetPrecedence(newString);

	if (stackPrecedence == 0 || newPrecedence == 0)
		return false;

	if (IsLeftAssociative(newString[0]))
	{
		if (newPrecedence <= stackPrecedence)
			return true;
	}
	else if (newPrecedence < stackPrecedence)
		return true;

	return false;
}

//==========================================================================
// Class:			ExpressionTree
// Function:		GetPrecedence
//
// Description:		Determines the precedence of the specified operator
//					(higher values are performed first)
//
// Input Arguments:
//		s	= const wxString& containing the operator
//
// Output Arguments:
//		None
//
// Return Value:
//		unsigned int representing the precedence
//
//==========================================================================
unsigned int ExpressionTree::GetPrecedence(const wxString &s) const
{
	if (s.Len() != 1)
		return 0;

	if (s[0] == '+' ||
		s[0] == '-')
		return 2;
	else if (s[0] == '*' ||
		s[0] == '/' ||
		s[0] == '%')
		return 3;
	else if (s[0] == '^')
		return 4;

	return 0;
}

//==========================================================================
// Class:			ExpressionTree
// Function:		IsLeftAssociative
//
// Description:		Determines if the specified operator is left or right
//					associative.
//
// Input Arguments:
//		c	= const wxChar&
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true if left associative
//
//==========================================================================
bool ExpressionTree::IsLeftAssociative(const wxChar &c) const
{
	switch (c)
	{
	case '^':
		return false;

	default:
		return true;
	}
}

//==========================================================================
// Class:			ExpressionTree
// Function:		PopFromStack
//
// Description:		Pops the next value from the top of the appropriate stack.
//
// Input Arguments:
//		stack	= std::stack<double>&
//
// Output Arguments:
//		None
//
// Return Value:
//		double
//
//==========================================================================
double ExpressionTree::PopFromStack(std::stack<double> &stack) const
{
	assert(!stack.empty());
	double value = stack.top();
	stack.pop();

	return value;
}

//==========================================================================
// Class:			ExpressionTree
// Function:		ApplyOperation
//
// Description:		Applies the specified operation to the specified operands.
//
// Input Arguments:
//		operation	= const wxString& describing the function to apply
//		first		= const double&
//		second		= const double&
//
// Output Arguments:
//		None
//
// Return Value:
//		double containing the result of the operation
//
//==========================================================================
double ExpressionTree::ApplyOperation(const wxString &operation,
	const double &first, const double &second) const
{
	if (operation.Cmp(_T("+")) == 0)
		return second + first;
	else if (operation.Cmp(_T("-")) == 0)
		return second - first;
	else if (operation.Cmp(_T("*")) == 0)
		return second * first;
	else if (operation.Cmp(_T("/")) == 0)
		return second / first;
	else if (operation.Cmp(_T("^")) == 0)
		return pow(second, first);

	assert(false);
	return 0.0;
}

//==========================================================================
// Class:			ExpressionTree
// Function:		EvaluateOperator
//
// Description:		Evaluates the operator specified.
//
// Input Arguments:
//		operator	= const wxString& describing the function to apply
//		stack		= std::stack<double>&
//
// Output Arguments:
//		errorString		= wxString&
//
// Return Value:
//		bool, true for success, false otherwise
//
//==========================================================================
bool ExpressionTree::EvaluateOperator(const wxString &operation, std::stack<double> &stack,
	wxString &errorString) const
{
	double value1, value2;

	if (stack.size() < 2)
		return EvaluateUnaryOperator(operation, stack, errorString);
	value1 = PopFromStack(stack);
	value2 = PopFromStack(stack);
	stack.push(ApplyOperation(operation, value1, value2));

	return true;
}

//==========================================================================
// Class:			ExpressionTree
// Function:		EvaluateUnaryOperator
//
// Description:		Evaluates the operator specified.  The only unary operator
//					we recognize is minus (negation).
//
// Input Arguments:
//		operator		= const wxString& describing the function to apply
//		stack		= std::stack<double>&
//
// Output Arguments:
//		errorString		= wxString&
//
// Return Value:
//		bool, true for success, false otherwise
//
//==========================================================================
bool ExpressionTree::EvaluateUnaryOperator(const wxString &operation, std::stack<double> &stack,
	wxString &errorString) const
{
	if (operation.Cmp(_T("-")) != 0)
	{
		errorString = _T("Attempting to apply operator without two operands!");
		return false;
	}

	stack.push(ApplyOperation(_T("*"), -1.0, PopFromStack(stack)));

	return true;
}

//==========================================================================
// Class:			ExpressionTree
// Function:		EvaluateNumber
//
// Description:		Evaluates the number specified.
//
// Input Arguments:
//		number		= const wxString& containing the number
//		stack		= std::stack<double>&
//
// Output Arguments:
//		errorString		= wxString&
//
// Return Value:
//		bool, true for success, false otherwise
//
//==========================================================================
bool ExpressionTree::EvaluateNumber(const wxString &number, std::stack<double> &stack,
	wxString &errorString) const
{
	double value;

	if (!number.ToDouble(&value))
	{
		errorString = _T("Could not convert ") + number + _T(" to a number.");
		return false;
	}

	stack.push(value);

	return true;
}

//==========================================================================
// Class:			ExpressionTree
// Function:		EvaluateNext
//
// Description:		Determines how to evaluate the specified term and takes
//					appropriate action.
//
// Input Arguments:
//		next		= const wxString&
//		stack		= std::stack<double>&
//
// Output Arguments:
//		errorString		= wxString&
//
// Return Value:
//		bool, true for valid operation, false otherwise
//
//==========================================================================
bool ExpressionTree::EvaluateNext(const wxString &next, std::stack<double> &stack,
	wxString &errorString) const
{
	if (NextIsNumber(next))
		return EvaluateNumber(next, stack, errorString);
	else if(NextIsOperator(next))
		return EvaluateOperator(next, stack, errorString);
	else
		errorString = _T("Unable to evaluate '") + next + _T("'.");

	return false;
}

//==========================================================================
// Class:			ExpressionTree
// Function:		BreakApartTerms
//
// Description:		Breaks apart all the terms in the string expression.  Be
//					wary of negative signs preceded by another operator!
//
// Input Arguments:
//		s	= const wxString&
//
// Output Arguments:
//		None
//
// Return Value:
//		wxArrayString
//
//==========================================================================
wxArrayString ExpressionTree::BreakApartTerms(const wxString &s)
{
	wxArrayString terms;
	size_t start(0), end(0);
	while (end != static_cast<size_t>(wxNOT_FOUND))
	{
		end = FindEndOfNextTerm(s, start);

		if (start > 0 && s.Mid(start - 1, 1).Cmp(_T("-")) == 0)
		{
			if (end != static_cast<size_t>(wxNOT_FOUND))
				terms.Add(s.Mid(start - 1, end + 1));
			else
				terms.Add(s.Mid(start - 1));
		}
		else
			terms.Add(s.Mid(start, end));

		start += end + 1;
	}

	return terms;
}

//==========================================================================
// Class:			ExpressionTree
// Function:		FindEndOfNextTerm
//
// Description:		Finds the end of the next term in the string.
//
// Input Arguments:
//		s		= const wxString&
//		start	= const unsigned int&
//
// Output Arguments:
//		None
//
// Return Value:
//		size_t
//
//==========================================================================
size_t ExpressionTree::FindEndOfNextTerm(const wxString &s, const unsigned int &start)
{
	size_t end, plusEnd, minusEnd, parenCount(0);

	plusEnd = s.Mid(start).Find('+');
	minusEnd = s.Mid(start).Find('-');

	if (minusEnd < plusEnd && start + minusEnd > 0 && NextIsOperator(s[start + minusEnd - 1]))
	{
		unsigned int nextMinus = s.Mid(start + minusEnd + 1).Find('-');
		if (nextMinus != static_cast<size_t>(wxNOT_FOUND))
			minusEnd += nextMinus + 1;
		else
			minusEnd = nextMinus;
	}
	end = std::min(plusEnd, minusEnd);

	if (end != static_cast<size_t>(wxNOT_FOUND) && NextIsOperator(s.Mid(start + end - 1)))
	{
		plusEnd = s.Mid(start + end).Find('+');
		minusEnd = s.Mid(start + end).Find('-');
		end += std::min(plusEnd, minusEnd);
	}
	else if (end != static_cast<size_t>(wxNOT_FOUND) && s.Mid(start + end - 1)[0] == 'e')
	{
		plusEnd = s.Mid(start + end + 1).Find('+');
		minusEnd = s.Mid(start + end + 1).Find('-');
		size_t endAdjust = std::min(plusEnd, minusEnd);
		if (endAdjust != static_cast<size_t>(wxNOT_FOUND))
			end += endAdjust;
		else
			end = endAdjust;
	}

	if (end != static_cast<size_t>(wxNOT_FOUND))
	{
		for (size_t i = start; i < start + end; i++)
		{
			if (s[i] == '(')
				parenCount++;
			else if (s[i] == ')')
				parenCount--;
		}

		if (parenCount > 0)
		{
			while (parenCount > 0)
			{
				if (s[start + end] == '(')
					parenCount++;
				else if (s[start + end] == ')')
					parenCount--;
				end++;
			}

			auto newEnd = FindEndOfNextTerm(s, start + end);
			if (newEnd == static_cast<size_t>(wxNOT_FOUND))
				return newEnd;
			else
				end += newEnd;
		}
	}

	return end;
}

//==========================================================================
// Class:			ExpressionTree
// Function:		SolveForString
//
// Description:		Solves the expression algebraically for the specified string.
//
// Input Arguments:
//		expression	= wxString
//		x			= const wxString& to solve for
//
// Output Arguments:
//		result		= wxString&
//
// Return Value:
//		wxString, empty for success, error string otherwise
//
//==========================================================================
wxString ExpressionTree::SolveForString(wxString expression, const wxString &x, wxString &result)
{
	assert(expression.Contains(x));

	wxString lhs, rhs;
	if (!SeparateSides(expression, lhs, rhs))
		return _T("Could not separate LHS and RHS!");

	if (!ParenthesesBalanced(lhs))
		return _T("Imbalanced parentheses on left-hand side!");

	if (!ParenthesesBalanced(rhs))
		return _T("Imbalanced parentheses on right-hand side!");

	wxArrayString lhTerms, rhTerms;
	wxString errorString;
	bool changed(true);
	while ((lhs.Cmp(x) != 0 || rhs.Contains(x)) && changed)
	{
		changed = false;

		lhTerms = BreakApartTerms(lhs);
		rhTerms = BreakApartTerms(rhs);

		for (size_t i = 0; i < rhTerms.Count(); i++)
		{
			if (rhTerms[i].Contains(x))
			{
				lhTerms.Add(InvertTerm(rhTerms[i]));
				rhTerms.RemoveAt(i);
				i--;
				changed = true;
			}
			else if (CombinedTerm(rhTerms[i]))
				return _T("Expression is too complicated!");
		}

		for (size_t i = 0; i < lhTerms.Count(); i++)
		{
			if (!lhTerms[i].Contains(x))
			{
				rhTerms.Add(InvertTerm(lhTerms[i]));
				lhTerms.RemoveAt(i);
				i--;
				changed = true;
			}
			else if (CombinedTerm(lhTerms[i]))
				return _T("Expression is too complicated!");
		}

		if (lhTerms.Count() > 1)
		{
			// TODO:  Implement a solution for this
			// Look for common component to take out of all terms?
			return _T("Expression is too complicated!");
		}
		else if (lhTerms[0].Cmp(x) != 0)
		{
			changed = Clean(lhTerms[0], x) || changed;
			if (lhTerms[0][0] == '(' && lhTerms[0].Last() == ')')
			{
				lhTerms[0] = lhTerms[0].Mid(1, lhTerms[0].Len() - 2);
				changed = true;
			}
			else
			{
				assert(lhTerms.Count() == 1 &&
					(lhTerms[0].Contains(_T("*")) || lhTerms[0].Contains(_T("/"))));

				errorString = CrossMultiplySimplify(lhTerms[0], rhTerms, x);
				changed = true;
			}
		}

		lhs = AssembleTerms(lhTerms);
		rhs = AssembleTerms(rhTerms);

		if (!errorString.empty())
			return errorString;
	}

	result = rhs;

	return wxEmptyString;
}

//==========================================================================
// Class:			ExpressionTree
// Function:		Clean
//
// Description:		Handles cases where the left-hand side includes a negative
//					sign in front of the desired variable.
//
// Input Arguments:
//		term	= wxString&
//		x		= const wxString&
//
// Output Arguments:
//		term	= wxString&
//
// Return Value:
//		bool, true if term was changed
//
//==========================================================================
bool ExpressionTree::Clean(wxString &term, const wxString &x)
{
	int xLocation = term.Find(x);

	if (xLocation > 0)
	{
		if (term[xLocation - 1] == '-')
		{
			term = term.Mid(0, xLocation - 1) + _T("-1*") + term.Mid(xLocation);
			return true;
		}
	}

	return false;
}

//==========================================================================
// Class:			ExpressionTree
// Function:		CrossMultiplySimplify
//
// Description:		Cross multiplies to get the left-hand side to equal x.
//
// Input Arguments:
//		lhs	= wxString&
//		rhs	= wxArrayString&
//		x	= const wxString& to solve for
//
// Output Arguments:
//		lhs	= wxString&
//		rhs	= wxArrayString&
//
// Return Value:
//		wxString, empty for success, error string otherwise
//
//==========================================================================
wxString ExpressionTree::CrossMultiplySimplify(wxString &lhs, wxArrayString &rhs, const wxString &x) const
{
	// FIXME:  Would be good to add parentheses checks here!
	if (!lhs.Contains(x))
		return _T("Left-hand side must contain '") + x + _T("'.");

	wxString start, end;
	int xLocation = lhs.Find(x);
	start = lhs.Mid(0, xLocation);
	if (lhs.Last() == x)
		end = wxEmptyString;
	else
		end = lhs.Mid(xLocation + x.Len());

	if (end.Len() > 0)
	{
		if (end[0] == '*')
		{
			for (size_t i = 0; i < rhs.Count(); i++)
			{
				rhs[i].Prepend(_T("("));
				rhs[i].Append(_T(")/(") + end.Mid(1) + _T(")"));
			}
		}
		else if (end[0] == '/')
		{
			for (size_t i = 0; i < rhs.Count(); i++)
			{
				rhs[i].Prepend(_T("("));
				rhs[i].Append(_T(")*(") + end.Mid(1) + _T(")"));
			}
		}
		else
			return _T("Could not extract '") + x + _T("' from left-hand side!");
	}

	if (start.Len() > 0)
	{
		if (start.Last() == '*')
		{
			for (size_t i = 0; i < rhs.Count(); i++)
			{
				rhs[i].Prepend(_T("("));
				rhs[i].Append(_T(")/(") + start.Mid(0, start.Len() - 1) + _T(")"));
			}
		}
		else if (start.Last() == '/')
		{
			wxString rhsAssembled(AssembleTerms(rhs));
			rhsAssembled.Prepend(_T("(") + start.Mid(0, start.Len() - 1) + _T(")/("));
			rhsAssembled.Append(_T(")"));
			rhs = BreakApartTerms(rhsAssembled);
		}
		else
			return _T("Could not extract '") + x + _T("' from left-hand side!");
	}

	lhs = x;

	return wxEmptyString;
}

//==========================================================================
// Class:			ExpressionTree
// Function:		AssembleTerms
//
// Description:		Assembles all terms into a single expression string.
//
// Input Arguments:
//		terms	= const wxArrayString&
//
// Output Arguments:
//		None
//
// Return Value:
//		wxString
//
//==========================================================================
wxString ExpressionTree::AssembleTerms(const wxArrayString &terms) const
{
	wxString e(terms[0]);
	for (size_t i = 1; i < terms.Count(); i++)
	{
		if (terms[i][0] == '-' || terms[i][0] == '+')
			e += terms[i];
		else
			e += _T("+") + terms[i];
	}

	return e;
}

//==========================================================================
// Class:			ExpressionTree
// Function:		CombinedTerm
//
// Description:		Determines if the specified term includes more than one
//					algebraic variable.
//
// Input Arguments:
//		term	= const wxString&
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true multiple variables, false otherwise
//
//==========================================================================
bool ExpressionTree::CombinedTerm(const wxString &term) const
{
	wxString variables;
	for (size_t i = 0; i < term.Len(); i++)
	{
		if ((term[i] == 'a' || term[i] == 'b' || term[i] == 'x') &&
			!variables.Contains(term.Mid(i, 1)))
			variables.Append(term.Mid(i, 1));
	}

	return variables.Len() > 1;
}

//==========================================================================
// Class:			ExpressionTree
// Function:		SeparateSides
//
// Description:		Separates an equation into right- and left-hand sides.
//
// Input Arguments:
//		e	= const wxString&
//
// Output Arguments:
//		lhs	= wxString&
//		rhs	= wxString&
//
// Return Value:
//		bool, true for success, false otherwise
//
//==========================================================================
bool ExpressionTree::SeparateSides(const wxString &e, wxString &lhs, wxString &rhs) const
{
	if (!e.Contains(_T("=")))
		return false;

	lhs = e.Mid(0, e.Find(_T("=")));
	rhs = e.Mid(e.Find(_T("=")) + 1);

	return true;
}

//==========================================================================
// Class:			ExpressionTree
// Function:		InvertTerm
//
// Description:		Inverts the sign in front of the term.
//
// Input Arguments:
//		term	= const wxString&
//
// Output Arguments:
//		None
//
// Return Value:
//		wxString
//
//==========================================================================
wxString ExpressionTree::InvertTerm(const wxString &term) const
{
	if (term[0] == '-')
		return _T("+") + term.Mid(1);
	else if (term[0] == '+')
		return _T("-") + term.Mid(1);

	return _T("-") + term;
}
/*
//==========================================================================
// Class:			ExpressionTree
// Function:		EvaluateExpression
//
// Description:		Evaluates the expression in the queue using Reverse Polish
//					Notation.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		results	= std::string&
//
// Return Value:
//		std::string containing a description of any errors, or empty string on success
//
//==========================================================================
wxString ExpressionTree::EvaluateExpression(wxString &results)
{
	wxString next, errorString;

	std::stack<double> doubleStack;
	std::stack<wxString> stringStack;
	std::stack<bool> useDoubleStack;

	while (!outputQueue.empty())
	{
		next = outputQueue.front();
		outputQueue.pop();

		if (!EvaluateNext(next, doubleStack, stringStack, useDoubleStack, errorString))
			return errorString;
	}

	if (useDoubleStack.size() > 1)
		return "Not enough operators!";

	if (useDoubleStack.top())
		results = wxString::Format("%0.*f",
			ConvertMath::GetPrecision(doubleStack.top(), printfPrecision),
			doubleStack.top());
	else
		results = stringStack.top();

	return wxEmptyString;
}

//==========================================================================
// Class:			ExpressionTree
// Function:		EvaluateNext
//
// Description:		Determines how to evaluate the specified term and takes
//					appropriate action.
//
// Input Arguments:
//		next			= const wxString&
//		doubleStack		= std::stack<double>&
//		stringStack		= std::stack<wxString>&
//		useDoubleStack	= std::stack<bool>&
//
// Output Arguments:
//		errorString		= wxString&
//
// Return Value:
//		bool, true for valid operation, false otherwise
//
//==========================================================================
bool ExpressionTree::EvaluateNext(const wxString &next, std::stack<double> &doubleStack,
		std::stack<wxString> &stringStack, std::stack<bool> &useDoubleStack, wxString &errorString) const
{
	if (NextIsNumber(next))
		return EvaluateNumber(next, doubleStack, useDoubleStack, errorString);
	else if(NextIsOperator(next))
		return EvaluateOperator(next, doubleStack, stringStack, useDoubleStack, errorString);
	else if (NextIsVariable(next))
	{
		PushToStack(next, stringStack, useDoubleStack);
		return true;
	}
	else
		errorString = _T("Unable to evaluate '") + next + _T("'.");

	return false;
}*/

//==========================================================================
// Class:			ExpressionTree
// Function:		NextIsVariable
//
// Description:		Determines if the next portion of the expression is
//					an algebraic variable.
//
// Input Arguments:
//		s		= const wxString& containing the expression
//
// Output Arguments:
//		stop	= unsigned int* (optional) indicating length
//
// Return Value:
//		bool, true if a dataset is next in the expression
//
//==========================================================================
bool ExpressionTree::NextIsVariable(const wxString &s, unsigned int *stop)
{
	if (s[0] == 'a' || s[0] == 'b' || s[0] == 'x')
	{
		if (stop)
			*stop = 1;
		return true;
	}

	return false;
}
/*
//==========================================================================
// Class:			ExpressionTree
// Function:		EvaluateNumber
//
// Description:		Evaluates the number specified.
//
// Input Arguments:
//		number			= const wxString& describing the function to apply
//		doubleStack		= std::stack<double>&
//		setStack		= std::stack<Dataset2D>&
//		useDoubleStack	= std::stack<bool>&
//
// Output Arguments:
//		errorString		= wxString&
//
// Return Value:
//		bool, true for success, false otherwise
//
//==========================================================================
bool ExpressionTree::EvaluateNumber(const wxString &number, std::stack<double> &doubleStack,
	std::stack<bool> &useDoubleStack, wxString &errorString) const
{
	double value;

	if (!number.ToDouble(&value))
	{
		errorString = _T("Could not convert ") + number + _T(" to a number.");
		return false;
	}

	PushToStack(value, doubleStack, useDoubleStack);

	return true;
}

//==========================================================================
// Class:			ExpressionTree
// Function:		EvaluateOperator
//
// Description:		Evaluates the operator specified.
//
// Input Arguments:
//		operator		= const wxString& describing the function to apply
//		doubleStack		= std::stack<double>&
//		stringStack		= std::stack<wxString>&
//		useDoubleStack	= std::stack<bool>&
//
// Output Arguments:
//		errorString		= wxString&
//
// Return Value:
//		bool, true for success, false otherwise
//
//==========================================================================
bool ExpressionTree::EvaluateOperator(const wxString &operation, std::stack<double> &doubleStack,
	std::stack<wxString> &stringStack, std::stack<bool> &useDoubleStack, wxString &errorString) const
{
	double value1, value2;
	wxString string1, string2;

	if (useDoubleStack.size() < 2)
		return EvaluateUnaryOperator(operation, doubleStack, stringStack, useDoubleStack, errorString);
	else if (PopFromStack(doubleStack, stringStack, useDoubleStack, string1, value1))
	{
		if (PopFromStack(doubleStack, stringStack, useDoubleStack, string2, value2))
			PushToStack(ApplyOperation(operation, value1, value2), doubleStack, useDoubleStack);
		else
			PushToStack(ApplyOperation(operation, value1, string2), stringStack, useDoubleStack);
	}
	else if (PopFromStack(doubleStack, stringStack, useDoubleStack, string2, value2))
		PushToStack(ApplyOperation(operation, string1, value2), stringStack, useDoubleStack);
	else
		PushToStack(ApplyOperation(operation, string1, string2), stringStack, useDoubleStack);

	return true;
}

//==========================================================================
// Class:			ExpressionTree
// Function:		EvaluateUnaryOperator
//
// Description:		Evaluates the operator specified.  The only unary operator
//					we recognize is minus (negation).
//
// Input Arguments:
//		operator		= const wxString& describing the function to apply
//		doubleStack		= std::stack<double>&
//		stringStack		= std::stack<wxString>&
//		useDoubleStack	= std::stack<bool>&
//
// Output Arguments:
//		errorString		= wxString&
//
// Return Value:
//		bool, true for success, false otherwise
//
//==========================================================================
bool ExpressionTree::EvaluateUnaryOperator(const wxString &operation, std::stack<double> &doubleStack,
	std::stack<wxString> &stringStack, std::stack<bool> &useDoubleStack, wxString &errorString) const
{
	if (operation.Cmp(_T("-")) != 0)
	{
		errorString = _T("Attempting to apply operator without two operands!");
		return false;
	}

	double value;
	wxString string;
	if (PopFromStack(doubleStack, stringStack, useDoubleStack, string, value))
		PushToStack(ApplyOperation(_T("*"), -1.0, value), doubleStack, useDoubleStack);
	else
		PushToStack(ApplyOperation(_T("*"), -1.0, string), stringStack, useDoubleStack);

	return true;
}

//==========================================================================
// Class:			ExpressionTree
// Function:		PushToStack
//
// Description:		Pushes the specified value onto the stack.
//
// Input Arguments:
//		value			= const double&
//		doubleStack		= std::stack<double>&
//		useDoubleStack	= std::stack<bool>&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void ExpressionTree::PushToStack(const double &value, std::stack<double> &doubleStack,
	std::stack<bool> &useDoubleStack) const
{
	doubleStack.push(value);
	useDoubleStack.push(true);
}

//==========================================================================
// Class:			ExpressionTree
// Function:		PushToStack
//
// Description:		Pushes the specified dataset onto the stack.
//
// Input Arguments:
//		s				= const wxString&
//		stringStack		= std::stack<wxString>&
//		useDoubleStack	= std::stack<bool>&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void ExpressionTree::PushToStack(const wxString &s, std::stack<wxString> &stringStack,
	std::stack<bool> &useDoubleStack) const
{
	stringStack.push(s);
	useDoubleStack.push(false);
}

//==========================================================================
// Class:			ExpressionTree
// Function:		PopFromStack
//
// Description:		Pops the next value from the top of the appropriate stack.
//
// Input Arguments:
//		doubleStack		= std::stack<double>&
//		stringStack		= std::stack<wxString>&
//		useDoubleStack	= std::stack<bool>&
//
// Output Arguments:
//		string			= wxString&
//		value			= double&
//
// Return Value:
//		bool, true if a double was popped, false otherwise
//
//==========================================================================
bool ExpressionTree::PopFromStack(std::stack<double> &doubleStack, std::stack<wxString> &stringStack,
	std::stack<bool> &useDoubleStack, wxString& string, double &value) const
{
	bool useDouble = useDoubleStack.top();
	useDoubleStack.pop();

	if (useDouble)
	{
		assert(!doubleStack.empty());
		value = doubleStack.top();
		doubleStack.pop();
	}
	else
	{
		assert(!stringStack.empty());
		string = stringStack.top();
		stringStack.pop();
	}

	return useDouble;
}

//==========================================================================
// Class:			ExpressionTree
// Function:		ApplyOperation
//
// Description:		Applies the specified operation to the specified operands.
//
// Input Arguments:
//		operation	= const wxString& describing the function to apply
//		first		= const wxString&
//		second		= const wxString&
//
// Output Arguments:
//		None
//
// Return Value:
//		wxString containing the result of the operation
//
//==========================================================================
wxString ExpressionTree::ApplyOperation(const wxString &operation,
	const wxString &first, const wxString &second) const
{
	if (operation.Cmp(_T("*")) == 0)
		return StringMultiply(first, second);
	return second + operation.c_str() + first;
}

//==========================================================================
// Class:			ExpressionTree
// Function:		ApplyOperation
//
// Description:		Applies the specified operation to the specified operands.
//
// Input Arguments:
//		operation	= const wxString& describing the function to apply
//		first		= const wxString&
//		second		= const double&
//
// Output Arguments:
//		None
//
// Return Value:
//		wxString containing the result of the operation
//
//==========================================================================
wxString ExpressionTree::ApplyOperation(const wxString &operation,
	const wxString &first, const double &second) const
{
	if (operation.Cmp(_T("+")) == 0)
		return StringAdd(first, second);
	else if (operation.Cmp(_T("-")) == 0)
		return StringSubtract(first, second);
	else if (operation.Cmp(_T("*")) == 0)
		return StringMultiply(first, second);

	assert(false);
	return "";
}

//==========================================================================
// Class:			ExpressionTree
// Function:		ApplyOperation
//
// Description:		Applies the specified operation to the specified operands.
//
// Input Arguments:
//		operation	= const wxString& describing the function to apply
//		first		= const double&
//		second		= const wxString&
//
// Output Arguments:
//		None
//
// Return Value:
//		wxString containing the result of the operation
//
//==========================================================================
wxString ExpressionTree::ApplyOperation(const wxString &operation,
	const double &first, const wxString &second) const
{
	if (operation.Cmp(_T("+")) == 0)
		return StringAdd(first, second);
	else if (operation.Cmp(_T("-")) == 0)
		return StringSubtract(first, second);
	else if (operation.Cmp(_T("*")) == 0)
		return StringMultiply(first, second);
	else if (operation.Cmp(_T("/")) == 0)
		return StringDivide(first, second);
	else if (operation.Cmp(_T("^")) == 0)
		return StringPower(first, second);

	assert(false);
	return "";
}*/