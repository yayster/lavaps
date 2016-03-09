
/*
 * scaled_blob.cc
 * Copyright (C) 1999-2003 by John Heidemann
 * $Id: scaled_blob.cc,v 1.5 2003/12/02 01:11:36 johnh Exp $
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

#include "config.h"

#include "scaled_blob.hh"

double scaled_blob::x_scale_ = 1.0;
double scaled_blob::y_scale_ = 1.0;
bool scaled_blob::vertical_ = true;

int scaled_blob::raw_height_ = 0;
int scaled_blob::raw_width_ = 0;

bool scaled_blob::has_base_ = true;
bool scaled_blob::lozenge_shaped_ = true;
int scaled_blob::lozenge_x_shrink_pct_ = 25;  // 0;  // 25;
int scaled_blob::lozenge_y_buldge_peak_pct_ = 75;
bool scaled_blob::lozenge_need_recompute_ = true;


scaled_blob::scaled_blob(process_view *pv) :
	blob(pv)
{
}

scaled_blob::~scaled_blob()
{
}

void
scaled_blob::resize_window(int w, int h)
{
	raw_width_ = w;
	raw_height_ = h;
	vertical_ = (w <= h);
	if (!vertical_)
		swap(w, h);
	x_scale_ = (double)w / X_MAX;
	int total_height = Y_MAX + (has_base_ ? BASE_EXTENSION : 0);
	y_scale_ = (double)h / total_height;
#if 0
	cout << O_("brs: vert ") << vertical_ <<
		" w=" << w <<
		" h=" << h <<
		" x=" << x_scale_ << 
		" y=" << y_scale_ << 
		endl;
#endif /* 0 */
	redraw_all();
	lozenge_need_recompute_ = true;
}


void
scaled_blob::lozenge_point(double x, double y, double *nx, double *ny)
	/*
	 * Map from x,y into nx,ny
	 * adjusting for distortion due to lava lamp lozenge shape.
	 */
{
	// 0,0 is top left
	static double Y_BULDGE_START,
		Y_BASE_START,
		Y_TOP_HEIGHT,  // from zero to buldge (majority)
		Y_BOT_HEIGHT,  // minority
		X_MID_HALF_WIDTH,
		X_END_HALF_WIDTH;
	static double X_REDUCTION_FRAC;

	if (lozenge_need_recompute_) {
		lozenge_need_recompute_ = false;
		Y_BULDGE_START = Y_MAX * lozenge_y_buldge_peak_pct_ / 100;
		Y_BASE_START = Y_MAX;
		Y_TOP_HEIGHT = Y_MAX - Y_BULDGE_START;
		Y_BOT_HEIGHT = Y_BULDGE_START;
		X_MID_HALF_WIDTH = X_MAX / 2;
		X_END_HALF_WIDTH = X_MAX * lozenge_x_shrink_pct_ / 100;
		X_REDUCTION_FRAC = ((X_MID_HALF_WIDTH - X_END_HALF_WIDTH) / (1.0 * X_MID_HALF_WIDTH));
	};

	double frac_x = (x - X_MID_HALF_WIDTH) / (1.0 * X_MID_HALF_WIDTH);
	bool y_on_top = (y > Y_BULDGE_START);
	bool y_on_base = (y > Y_BASE_START);
	double abs_frac_y = 
		y_on_base ? 1.0 :
		y_on_top ? (y - Y_BULDGE_START) / (1.0 * Y_TOP_HEIGHT)
		: (y - Y_BULDGE_START) / (-1.0 * Y_BOT_HEIGHT);
	double adj_frac_x = frac_x - frac_x * abs_frac_y * X_REDUCTION_FRAC;

	*nx = adj_frac_x * X_MID_HALF_WIDTH + X_MID_HALF_WIDTH;
//	*ny = int((y_on_top ? (abs_frac_y * Y_TOP) : (- abs_frac_y * Y_BOT)) + Y_BULDGE);
	*ny = y;
}

void
scaled_blob::scale_point(double x, double y, double *nx, double *ny)
	/*
	 * Map from abstract x,y to scaled coordinates.
	 * adjusting for distortion due to lava lamp lozenge shape
	 * and scaling to screen.
	 */
{
	// distort
	if (lozenge_x_shrink_pct_ != 0)
		lozenge_point(x, y, &x, &y);

	// scale to screen
	*nx = x * x_scale_;
	*ny = y * y_scale_;
	if (!vertical_)
		swap(*nx, *ny);
}
	
