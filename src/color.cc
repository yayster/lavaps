
/*
 * color.cc
 * Copyright (C) 1999-2003 by John Heidemann
 * $Id: color.cc,v 1.12 2003/06/22 04:26:07 johnh Exp $
 * 
 * From xlockmore-4.11's xlock/color.c,
 * in turn from swirl.c
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

/*
 * Jamie Zawinski has released this code
 * for use in lavaps under the GPL (as of 11 November 1999).
 *
 * The original copyright and license was:
 *
 * swirl.c Copyright (c) 1994 M.Dobie <mrd@ecs.soton.ac.uk>
 * xlock Copyright (c) 1988-91 by Patrick J. Naughton.
 * xscreensaver, Copyright (c) 1997 Jamie Zawinski <jwz@netscape.com>
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation.
 *
 * This file is provided AS IS with no warranties of any kind.  The author
 * shall have no liability with respect to the infringement of copyrights,
 * trade secrets or any patents by this file or any part thereof.  In no
 * event will the author be liable for any lost revenue or profits or
 * other special, indirect and consequential damages.
 *
 */


#include "config.h"

#include <math.h>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>

/*
 * hsb_to_rgb basically from xlockmore (This is basically the same as
 * the code in Foley, Van Damme, Feiner, Hughes figure 13.34.),
 * but modified to allow scaling (from 0..maximum).
 */
void
hsb_to_rgb(double H, double S, double B,
	int maximum,
	int *r, int *g, int *b)
{
	int         i;
	double      f, bb;
	int p, q, t;

	H -= floor(H);		/* remove anything over 1 */
	H *= 6.0;
	i = (int) floor(H);	/* 0..5 */
	f = H - (float) i;	/* f = fractional part of H */
	bb = maximum * B;
	p = (unsigned) (bb * (1.0 - S));
	q = (unsigned) (bb * (1.0 - (S * f)));
	t = (unsigned) (bb * (1.0 - (S * (1.0 - f))));
	switch (i) {
	case 0:
		*r = (int) bb;
		*g = t;
		*b = p;
		break;
	case 1:
		*r = q;
		*g = (int) bb;
		*b = p;
		break;
	case 2:
		*r = p;
		*g = (int) bb;
		*b = t;
		break;
	case 3:
		*r = p;
		*g = q;
		*b = (int) bb;
		break;
	case 4:
		*r = t;
		*g = p;
		*b = (int) bb;
		break;
	case 5:
		*r = (int) bb;
		*g = p;
		*b = q;
		break;
	}
}

string 
rgb8_to_string(int r, int g, int b)
{
	ostringstream oss;
	oss << "#" 
	    << setbase(16) << setfill('0')
	    << setw(2) << (r & 0xff)
	    << setw(2) << (g & 0xff)
	    << setw(2) << (b & 0xff);
	return oss.str();
}

string 
rgb16_to_string(int r, int g, int b)
{
	return rgb8_to_string((r>>8), (g>>8), (b>>8));
}


/*
 * hsb_to_string is my front-end
 * returns an x-compatible color string
 */
string
hsb_to_rgb_string(double H, double S, double B)
{
	int r, g, b;

	hsb_to_rgb(H, S, B, 255, &r, &g, &b);
	string s;
	return rgb8_to_string(r, g, b);
}
