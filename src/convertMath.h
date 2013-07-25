/*===================================================================================
                                       Converter
                              Copyright Kerry R. Loux 2013

  This code is licensed under the MIT License (http://opensource.org/licenses/MIT).

===================================================================================*/

// File:  convertMath.h
// Created:  6/4/2013
// Author:  K. Loux
// Description:  Math helpers.
// History:

#ifndef _CONVERT_MATH_H_
#define _CONVERT_MATH_H_

// wxWidgets forward declarations
class wxString;

namespace ConvertMath
{
	unsigned int CountSignificantDigits(const wxString &valueString);

	unsigned int GetPrecision(const double &value,
		const unsigned int &significantDigits = 2, const bool &dropTrailingZeros = true);

	void KRLsprintf(char *dest, const unsigned int &size, const char *format, ...);
}

#endif// _CONVERT_MATH_H_