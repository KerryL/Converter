/*===================================================================================
                                       Converter
                              Copyright Kerry R. Loux 2013

  This code is licensed under the MIT License (http://opensource.org/licenses/MIT).

===================================================================================*/

// File:  convertMath.cpp
// Created:  6/4/2013
// Author:  K. Loux
// Description:  Math helpers.
// History:

// wxWidgets headers
#include <wx/wx.h>

// Local headers
#include "convertMath.h"

//==========================================================================
// Namespace:		ConvertMath
// Function:		CountSignificantDigits
//
// Description:		Returns the number of significant digits in the string.
//
// Input Arguments:
//		valueString	= const wxString&
//
// Output Arguments:
//		None
//
// Return Value:
//		unsigned int
//
//==========================================================================
unsigned int ConvertMath::CountSignificantDigits(const wxString &valueString)
{
	double value;
	if (!valueString.ToDouble(&value))
		return 0;

	wxString trimmedValueString = valueString;
	if (trimmedValueString.Contains(_T("e")))
		trimmedValueString = trimmedValueString.Mid(0, trimmedValueString.Find(_T("e")));
	else if (trimmedValueString.Contains(_T("E")))
		trimmedValueString = trimmedValueString.Mid(0, trimmedValueString.Find(_T("E")));

	int firstDigit, lastDigit;
	for (firstDigit = 0; firstDigit < (int)trimmedValueString.Len(); firstDigit++)
	{
		if (trimmedValueString[firstDigit] != '0' && trimmedValueString[firstDigit] != '.' &&
			trimmedValueString[firstDigit] != '+' && trimmedValueString[firstDigit] != '-')
			break;
	}

	for (lastDigit = trimmedValueString.Len() - 1; lastDigit > firstDigit; lastDigit--)
	{
		if (trimmedValueString[lastDigit] != '0' && trimmedValueString[lastDigit] != '.' &&
			trimmedValueString[lastDigit] != '+' && trimmedValueString[lastDigit] != '-')
			break;
	}

	int i;
	for (i = firstDigit + 1; i < lastDigit; i++)
	{
		if (trimmedValueString[i] == '.')
		{
			firstDigit++;
			break;
		}
	}

	return lastDigit - firstDigit + 1;
}

//==========================================================================
// Namespace:		ConvertMath
// Function:		GetPrecision
//
// Description:		Determines the best number of digits after the decimal place
//					for a string representation of the specified value (for
//					use with printf-style %0.*f formatting.
//
// Input Arguments:
//		value				= const double&
//		significantDigits	= const unsigned int&
//		dropTrailingZeros	= const bool&
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true if the x-data spacing is within the tolerance
//
//==========================================================================
unsigned int ConvertMath::GetPrecision(const double &value,
	const unsigned int &significantDigits, const bool &dropTrailingZeros)
{
	int precision(significantDigits - (unsigned int)floor(log10(value)) - 1);
	if (precision < 0)
		precision = 0;
	if (!dropTrailingZeros)
		return precision;

	const unsigned int sSize(512);
	char s[sSize];
	KRLsprintf(s, sSize, "%0.*f", precision, value);

	std::string number(s);
	unsigned int i;
	for (i = number.size() - 1; i > 0; i--)
	{
		if (s[i] == '0')
			precision--;
		else
			break;
	}

	if (precision < 0)
		precision = 0;

	return precision;
}

//==========================================================================
// Namespace:		ConvertMath
// Function:		KRLsprintf
//
// Description:		Cross-platform friendly sprintf(_s) macro.  Calls sprintf_s
//					under MSW, sprintf otherwise.
//
// Input Arguments:
//		dest	= char*
//		size	= const unsigned int&
//		format	= const char*
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void ConvertMath::KRLsprintf(char *dest, const unsigned int &size, const char *format, ...)
{
	va_list list;
	va_start(list, format);

#ifdef __WXMSW__
	vsprintf_s(dest, size, format, list);
#else
	vsprintf(dest, format, list);
#endif

	va_end(list);
}