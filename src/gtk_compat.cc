
/*
 * gtk_compat.cc
 * Copyright (C) 2003 by John Heidemann
 * $Id: gtk_compat.cc,v 1.7 2003/07/05 16:06:01 johnh Exp $
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
 *
 */

/*
 * Gnome/gtk functions that IMHO should exist but don't.
 */

#include "config.h"

#ifdef USE_GTK_BLOB

#include <unistd.h>
#include <stdlib.h>

#include <assert.h>

#include "gtk_compat.hh"


/*
 * gtk_menu_item_set_label_markup:
 * an abstraction to let us set the menu label text.
 * Implementation given by John (J5) Palmieri
 * on the gnome-devel-list mailing list,
 * and direct access to the label suggested by Michael Meeks.
 * Thanks!
 */
void
gtk_menu_item_set_label_markup(GtkMenuItem *menu_item, const char *label)
{
	gtk_label_set_markup(GTK_LABEL(GTK_BIN(menu_item)->child), label);
}

/*
  redraw_if_visible and put_item_after ARE
 * COPIED VERBATIM libgnomecanvas-2.2.0.1's gnome-canvas.c.
 * just because they're not exported.
 * They should go away if canvas exports 
 * gnome_canvas_item_lower_below.
 */
static void
redraw_if_visible (GnomeCanvasItem *item)
{
	if (item->object.flags & GNOME_CANVAS_ITEM_VISIBLE)
		gnome_canvas_request_redraw (item->canvas, (int)item->x1, (int)item->y1, (int)item->x2 + 1, (int)item->y2 + 1);
}
static gboolean
put_item_after (GList *link, GList *before)
{
	GnomeCanvasGroup *parent;

	if (link == before)
		return FALSE;

	parent = GNOME_CANVAS_GROUP (GNOME_CANVAS_ITEM (link->data)->parent);

	if (before == NULL) {
		if (link == parent->item_list)
			return FALSE;

		link->prev->next = link->next;

		if (link->next)
			link->next->prev = link->prev;
		else
			parent->item_list_end = link->prev;

		link->prev = before;
		link->next = parent->item_list;
		link->next->prev = link;
		parent->item_list = link;
	} else {
		if ((link == parent->item_list_end) && (before == parent->item_list_end->prev))
			return FALSE;

		if (link->next)
			link->next->prev = link->prev;

		if (link->prev)
			link->prev->next = link->next;
		else {
			parent->item_list = link->next;
			parent->item_list->prev = NULL;
		}

		link->prev = before;
		link->next = before->next;

		link->prev->next = link;

		if (link->next)
			link->next->prev = link;
		else
			parent->item_list_end = link;
	}
	return TRUE;
}

/**
 * gnome_canvas_item_lower_below:
 * @item: A canvas item whose stacking order should change.
 * @superior: The item that will be just above ITEM upon exit.
 *
 * Raises the item in the stacking order so that it just just below
 * superior.
 **/
void
gnome_canvas_item_lower_below(GnomeCanvasItem *item, GnomeCanvasItem *superior)
{
	GList *item_link, *superior_link;
	GnomeCanvasGroup *parent;

	g_return_if_fail(GNOME_IS_CANVAS_ITEM(item));
	g_return_if_fail(GNOME_IS_CANVAS_ITEM(superior));

	if (!item->parent)
		return;  /* not part of a canvas? */
	if (!superior->parent)
		return;  /* not part of a canvas? */
//	g_assert(item->parent == superior->parent);  /* can't move between canvases */

	parent = GNOME_CANVAS_GROUP(item->parent);
	item_link = g_list_find(parent->item_list, item);
	g_assert(item_link != NULL);
	superior_link = g_list_find(parent->item_list, item);
	g_assert(superior_link != NULL);

	/* move it */
	if (put_item_after(item_link, superior_link)) {
		redraw_if_visible(item);
		item->canvas->need_repick = TRUE;
	};

}



/*
 * This functions
 *	gnome_canvas_TkBezierPoints
 * and
 *	gnome_canvas_TkMakeBezierCurve
 * are taken (with minor editing/simplification) from tk-8.3.5
 * to implement smooth polygons.  It has been modified as necessary
 * to run without tcltk.
 *
 * Source file: tkTrig.c
 *
 * Copyright (c) 1992-1994 The Regents of the University of California.
 * Copyright (c) 1994-1997 Sun Microsystems, Inc.
 *
 * See the material "license.terms" (reproduced below) for information
 * on usage and redistribution of this file, and for a DISCLAIMER OF
 * ALL WARRANTIES.
 *
 * Original Tk RCS: @(#) Id: tkTrig.c,v 1.4 1999/12/14 06:52:33 hobbs Exp 
 */

/*
 * licence terms (from the file license.terms in tk-8.3.5):
 *
 * This software is copyrighted by the Regents of the University of
 * California, Sun Microsystems, Inc., Scriptics Corporation,
 * and other parties.  The following terms apply to all files associated
 * with the software unless explicitly disclaimed in individual files.
 * 
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 * 
 * IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY
 * FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
 * ARISING OUT OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY
 * DERIVATIVES THEREOF, EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 * 
 * THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE
 * IS PROVIDED ON AN "AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE
 * NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR
 * MODIFICATIONS.
 * 
 * GOVERNMENT USE: If you are acquiring this software on behalf of the
 * U.S. government, the Government shall have only "Restricted Rights"
 * in the software and related documentation as defined in the Federal 
 * Acquisition Regulations (FARs) in Clause 52.227.19 (c) (2).  If you
 * are acquiring the software on behalf of the Department of Defense, the
 * software shall be classified as "Commercial Computer Software" and the
 * Government shall have only "Restricted Rights" as defined in Clause
 * 252.227-7013 (c) (1) of DFARs.  Notwithstanding the foregoing, the
 * authors grant the U.S. Government and others acting in its behalf
 * permission to use and distribute the software in accordance with the
 * terms specified in this license. 
 */


/*
 *--------------------------------------------------------------
 *
 * gnome_canvas_TkBezierPoints --
 *
 *	Given four control points, create a larger set of points
 *	for a Bezier spline based on the points.
 *
 * Results:
 *	The array at *coordPtr gets filled in with 2*numSteps
 *	coordinates, which correspond to the Bezier spline defined
 *	by the four control points.  Note:  no output point is
 *	generated for the first input point, but an output point
 *	*is* generated for the last input point.
 *
 * Side effects:
 *	None.
 *
 *--------------------------------------------------------------
 */

static void
gnome_canvas_TkBezierPoints(double *control, int numSteps, double *coordPtr)
#if 0
    double control[];			/* Array of coordinates for four
					 * control points:  x0, y0, x1, y1,
					 * ... x3 y3. */
    int numSteps;			/* Number of curve points to
					 * generate.  */
    register double *coordPtr;		/* Where to put new points. */
#endif /* 0 */
{
    int i;
    double u, u2, u3, t, t2, t3;

    for (i = 1; i <= numSteps; i++, coordPtr += 2) {
	t = ((double) i)/((double) numSteps);
	t2 = t*t;
	t3 = t2*t;
	u = 1.0 - t;
	u2 = u*u;
	u3 = u2*u;
	coordPtr[0] = control[0]*u3
		+ 3.0 * (control[2]*t*u2 + control[4]*t2*u) + control[6]*t3;
	coordPtr[1] = control[1]*u3
		+ 3.0 * (control[3]*t*u2 + control[5]*t2*u) + control[7]*t3;
    }
}



/*
 *--------------------------------------------------------------
 *
 * gnome_canvas_TkMakeBezierCurve --
 *
 *	Given a set of points, create a new set of points that fit
 *	parabolic splines to the line segments connecting the original
 *	points.  Produces output points in either of two forms.
 *
 *	Note: in spite of this procedure's name, it does *not* generate
 *	Bezier curves.  Since only three control points are used for
 *	each curve segment, not four, the curves are actually just
 *	parabolic.
 *
 * Results:
 *	Either or both of the xPoints or dblPoints arrays are filled
 *	in.  The return value is the number of points placed in the
 *	arrays.  Note:  if the first and last points are the same, then
 *	a closed curve is generated.
 *
 * Side effects:
 *	None.
 *
 *
 * original api: TkMakeBezierCurve(canvas, pointPtr, numPoints, numSteps, xPoints, dblPoints) 
 *  Tk_Canvas canvas;    Canvas in which curve is to be drawn.
 *    XPoint xPoints[];   Array of XPoints to fill in (e.g. for display.
 *			NULL means don't fill in any XPoints.
 *
 *--------------------------------------------------------------
 */

static int
gnome_canvas_TkMakeBezierCurve(double *pointPtr, int numPoints, int numSteps, double *dblPoints)
#if 0
    double *pointPtr;			/* Array of input coordinates:  x0,
					 * y0, x1, y1, etc.. */
    int numPoints;			/* Number of points at pointPtr. */
    int numSteps;			/* Number of steps to use for each
					 * spline segments (determines
					 * smoothness of curve). */
    double dblPoints[];			/* Array of points to fill in as
					 * doubles, in the form x0, y0,
					 * x1, y1, ....  NULL means don't
					 * fill in anything in this form. 
					 * Caller must make sure that this
					 * array has enough space. */
#endif /* 0 */
{
    int closed, outputPoints, i;
    int numCoords = numPoints*2;
    double control[8];

    /*
     * If the curve is a closed one then generate a special spline
     * that spans the last points and the first ones.  Otherwise
     * just put the first point into the output.
     */

    if (!pointPtr) {
	/* Of pointPtr == NULL, this function returns an upper limit.
	 * of the array size to store the coordinates. This can be
	 * used to allocate storage, before the actual coordinates
	 * are calculated. */
	return 1 + numPoints * numSteps;
    }

    outputPoints = 0;
    if ((pointPtr[0] == pointPtr[numCoords-2])
	    && (pointPtr[1] == pointPtr[numCoords-1])) {
	closed = 1;
	control[0] = 0.5*pointPtr[numCoords-4] + 0.5*pointPtr[0];
	control[1] = 0.5*pointPtr[numCoords-3] + 0.5*pointPtr[1];
	control[2] = 0.167*pointPtr[numCoords-4] + 0.833*pointPtr[0];
	control[3] = 0.167*pointPtr[numCoords-3] + 0.833*pointPtr[1];
	control[4] = 0.833*pointPtr[0] + 0.167*pointPtr[2];
	control[5] = 0.833*pointPtr[1] + 0.167*pointPtr[3];
	control[6] = 0.5*pointPtr[0] + 0.5*pointPtr[2];
	control[7] = 0.5*pointPtr[1] + 0.5*pointPtr[3];
	if (dblPoints != NULL) {
	    dblPoints[0] = control[0];
	    dblPoints[1] = control[1];
	    gnome_canvas_TkBezierPoints(control, numSteps, dblPoints+2);
	    dblPoints += 2*(numSteps+1);
	}
	outputPoints += numSteps+1;
    } else {
	closed = 0;
	if (dblPoints != NULL) {
	    dblPoints[0] = pointPtr[0];
	    dblPoints[1] = pointPtr[1];
	    dblPoints += 2;
	}
	outputPoints += 1;
    }

    for (i = 2; i < numPoints; i++, pointPtr += 2) {
	/*
	 * Set up the first two control points.  This is done
	 * differently for the first spline of an open curve
	 * than for other cases.
	 */

	if ((i == 2) && !closed) {
	    control[0] = pointPtr[0];
	    control[1] = pointPtr[1];
	    control[2] = 0.333*pointPtr[0] + 0.667*pointPtr[2];
	    control[3] = 0.333*pointPtr[1] + 0.667*pointPtr[3];
	} else {
	    control[0] = 0.5*pointPtr[0] + 0.5*pointPtr[2];
	    control[1] = 0.5*pointPtr[1] + 0.5*pointPtr[3];
	    control[2] = 0.167*pointPtr[0] + 0.833*pointPtr[2];
	    control[3] = 0.167*pointPtr[1] + 0.833*pointPtr[3];
	}

	/*
	 * Set up the last two control points.  This is done
	 * differently for the last spline of an open curve
	 * than for other cases.
	 */

	if ((i == (numPoints-1)) && !closed) {
	    control[4] = .667*pointPtr[2] + .333*pointPtr[4];
	    control[5] = .667*pointPtr[3] + .333*pointPtr[5];
	    control[6] = pointPtr[4];
	    control[7] = pointPtr[5];
	} else {
	    control[4] = .833*pointPtr[2] + .167*pointPtr[4];
	    control[5] = .833*pointPtr[3] + .167*pointPtr[5];
	    control[6] = 0.5*pointPtr[2] + 0.5*pointPtr[4];
	    control[7] = 0.5*pointPtr[3] + 0.5*pointPtr[5];
	}

	/*
	 * If the first two points coincide, or if the last
	 * two points coincide, then generate a single
	 * straight-line segment by outputting the last control
	 * point.
	 */

	if (((pointPtr[0] == pointPtr[2]) && (pointPtr[1] == pointPtr[3]))
		|| ((pointPtr[2] == pointPtr[4])
		&& (pointPtr[3] == pointPtr[5]))) {
	    if (dblPoints != NULL) {
		dblPoints[0] = control[6];
		dblPoints[1] = control[7];
		dblPoints += 2;
	    }
	    outputPoints += 1;
	    continue;
	}

	/*
	 * Generate a Bezier spline using the control points.
	 */
	if (dblPoints != NULL) {
	    gnome_canvas_TkBezierPoints(control, numSteps, dblPoints);
	    dblPoints += 2*numSteps;
	}
	outputPoints += numSteps;
    }
    return outputPoints;
}


/**
 * gnome_canvas_points_smooth:
 * @src: Source list of points inbetween which to interopolate.
 * @splineSteps: How many points to put between each original one.
 * 
 * Creates a structure that should be used to pass an array of points to
 * items.
 *
 * Internal implementation is based on Tk-8.3.5.
 *
 * This function itself is a bit of a hack:
 * really there should just be a "smooth" option to polygon.
 * 
 * Return value: A newly-created array of interpolated points.
 * It should be freed by the caller via gnome_canvas_points_free().
 *
 **/
GnomeCanvasPoints *
gnome_canvas_points_smooth(GnomeCanvasPoints *src, int splineSteps)
{
	int numPointsEstimate, numPoints;
	GnomeCanvasPoints *dest;

	/* find how many points we need */
	numPointsEstimate = gnome_canvas_TkMakeBezierCurve((double *) NULL,
							   src->num_points,
							   splineSteps,
							   NULL);
	dest = gnome_canvas_points_new(numPointsEstimate);
	if (dest == NULL)  // out of memory!
		return NULL;

	numPoints = gnome_canvas_TkMakeBezierCurve(src->coords,
						   src->num_points,
						   splineSteps,
						   dest->coords);
	assert(numPoints <= numPointsEstimate);
	// assume that gtk_points_free can deal with changes to num_points
	dest->num_points = numPoints;

	return dest;
}

#endif /* USE_GTK_BLOB */
