/*
** Copyright 2007-2018 RTE
** Authors: Antares_Simulator Team
**
** This file is part of Antares_Simulator.
**
** Antares_Simulator is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** There are special exceptions to the terms and conditions of the
** license as they are applied to this software. View the full text of
** the exceptions in file COPYING.txt in the directory of this software
** distribution
**
** Antares_Simulator is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Antares_Simulator. If not, see <http://www.gnu.org/licenses/>.
**
** SPDX-License-Identifier: licenceRef-GPL3_WITH_RTE-Exceptions
*/

#include "string-to-double.h"

/*
** strtod.c --
**
**	Source code for the "strtod" library procedure.
**
** Copyright (c) 1988-1993 The Regents of the University of California.
** Copyright (c) 1994 Sun Microsystems, Inc.
**
** Permission to use, copy, modify, and distribute this
** software and its documentation for any purpose and without
** fee is hereby granted, provided that the above copyright
** notice appear in all copies.  The University of California
** makes no representations about the suitability of this
** software for any purpose.  It is provided "as is" without
** express or implied warranty.
**
** RCS: @(#) $Id: strtod.c,v 1.3 2000/02/17 07:11:22 matz Exp $
*/

#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

enum
{
    /*!
    ** \brief Largest possible base 10 exponent.
    **
    ** Any exponent larger than this will already
    ** produce underflow or overflow, so there's
    ** no need to worry about additional digits.
    */
    maxExponent = 511,
};

static double powersOf10[] = /* Table giving binary powers of 10.  Entry */
  {10.,                      /* is 10^2^i.  Used to convert decimal */
   100.,                     /* exponents into floating-point numbers. */
   1.0e4,
   1.0e8,
   1.0e16,
   1.0e32,
   1.0e64,
   1.0e128,
   1.0e256};

/*
**----------------------------------------------------------------------
**
** strtod --
**
**	This procedure converts a floating-point number from an ASCII
**	decimal representation to internal double-precision format.
**
** Results:
**	The return value is the double-precision floating-point
**	representation of the characters in string.  If endPtr isn't
**	NULL, then *endPtr is filled in with the address of the
**	next character after the last one that was part of the
**	floating-point number.
**
** Side effects:
**	None.
**
**----------------------------------------------------------------------
*/

/*const char *string;*/    /* A decimal ASCII floating-point number,
                            * optionally preceded by white space.
                            * Must have form "-I.FE-X", where I is the
                            * integer part of the mantissa, F is the
                            * fractional part of the mantissa, and X
                            * is the exponent.  Either of the signs
                            * may be "+", "-", or omitted.  Either I
                            * or F may be omitted, or both.  The decimal
                            * point isn't necessary unless F is present.
                            * The "E" may actually be an "e".  E and X
                            * may both be omitted (but not just one).
                            */
/*  char **endPtr;	*/ /* If non-NULL, store terminating character's
                            * address here. */

double mystrtod(const char* string, char** endPtr)
{
    int sign, expSign = FALSE;
    double fraction, dblExp, *d;
    register const char* p;
    register int c;
    int exp = 0;      /* Exponent read from "EX" field. */
    int fracExp = 0;  /* Exponent that derives from the fractional
                       ** part.  Under normal circumstatnces, it is
                       ** the negative of the number of digits in F.
                       ** However, if I is very long, the last digits
                       ** of I get dropped (otherwise a long I with a
                       ** large negative exponent could cause an
                       ** unnecessary overflow on I alone).  In this
                       ** case, fracExp is incremented one for each
                       ** dropped digit. */
    int mantSize;     /* Number of digits in mantissa. */
    int decPt;        /* Number of mantissa digits BEFORE decimal
                       ** point. */
    const char* pExp; /* Temporarily holds location of exponent
                       ** in string. */

    /* Strip off leading blanks and check for a sign. */

    p = string;
    while (isspace(*p))
        p += 1;

    if (*p == '-')
    {
        sign = TRUE;
        p += 1;
    }
    else
    {
        if (*p == '+')
            p += 1;
        sign = FALSE;
    }

    /*
    ** Count the number of digits in the mantissa (including the decimal
    ** point), and also locate the decimal point.
    */

    decPt = -1;
    for (mantSize = 0;; mantSize += 1)
    {
        c = *p;
        if (!isdigit(c))
        {
            if ((c != '.') || (decPt >= 0))
                break;
            decPt = mantSize;
        }
        p += 1;
    }

    /*
    ** Now suck up the digits in the mantissa.  Use two integers to
    ** collect 9 digits each (this is faster than using floating-point).
    ** If the mantissa has more than 18 digits, ignore the extras, since
    ** they can't affect the value anyway.
    */

    pExp = p;
    p -= mantSize;
    if (decPt < 0)
    {
        decPt = mantSize;
    }
    else
    {
        mantSize -= 1; /* One of the digits was the point. */
    }
    if (mantSize > 18)
    {
        fracExp = decPt - 18;
        mantSize = 18;
    }
    else
    {
        fracExp = decPt - mantSize;
    }
    if (mantSize == 0)
    {
        fraction = 0.0;
        p = string;
        goto done;
    }
    else
    {
        int frac1, frac2;
        frac1 = 0;
        for (; mantSize > 9; mantSize -= 1)
        {
            c = *p;
            p += 1;
            if (c == '.')
            {
                c = *p;
                p += 1;
            }
            frac1 = 10 * frac1 + (c - '0');
        }
        frac2 = 0;
        for (; mantSize > 0; mantSize -= 1)
        {
            c = *p;
            p += 1;
            if (c == '.')
            {
                c = *p;
                p += 1;
            }
            frac2 = 10 * frac2 + (c - '0');
        }
        fraction = (1.0e9 * frac1) + frac2;
    }

    /*
     * Skim off the exponent.
     */

    p = pExp;
    if ((*p == 'E') || (*p == 'e'))
    {
        p += 1;
        if (*p == '-')
        {
            expSign = TRUE;
            p += 1;
        }
        else
        {
            if (*p == '+')
            {
                p += 1;
            }
            expSign = FALSE;
        }
        while (isdigit(*p))
        {
            exp = exp * 10 + (*p - '0');
            p += 1;
        }
    }
    if (expSign)
    {
        exp = fracExp - exp;
    }
    else
    {
        exp = fracExp + exp;
    }

    /*
     * Generate a floating-point number that represents the exponent.
     * Do this by processing the exponent one bit at a time to combine
     * many powers of 2 of 10. Then combine the exponent with the
     * fraction.
     */

    if (exp < 0)
    {
        expSign = TRUE;
        exp = -exp;
    }
    else
    {
        expSign = FALSE;
    }

    if (exp > maxExponent)
    {
        exp = maxExponent;
        errno = ERANGE;
    }

    dblExp = 1.0;
    for (d = powersOf10; exp != 0; exp >>= 1, d += 1)
    {
        if (exp & 01)
            dblExp *= *d;
    }
    if (expSign)
        fraction /= dblExp;
    else
        fraction *= dblExp;

done:
    if (endPtr != NULL)
        *endPtr = (char*)p;

    if (sign)
        return -fraction;

    return fraction;
}