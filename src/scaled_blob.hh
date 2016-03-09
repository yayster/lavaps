/* -*-	Mode:C++ -*- */

/*
 * scaled_blob.hh
 * Copyright (C) 1999-2003 by John Heidemann
 * $Id: scaled_blob.hh,v 1.6 2003/07/05 18:04:54 johnh Exp $
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
 * scaled_blob extends a blob to provide scaling & lozenge functiosn
 * (that are common to tcl & gtk backends)
 */

#include "blob.hh"

class scaled_blob : public blob {
private:
	scaled_blob(const scaled_blob& tb);
	scaled_blob& operator=(const scaled_blob& tb);

protected:
	static bool vertical_;
	static int raw_width_, raw_height_;  // raw, not rotated (in x/y)
	static double x_scale_, y_scale_;
	static void resize_window(int w, int h);

	static bool lozenge_shaped_;
	static int lozenge_x_shrink_pct_,
		lozenge_y_buldge_peak_pct_;
	static bool lozenge_need_recompute_;

	// mapping through scaling and lozenging
	static void lozenge_point(double x, double y, double *nx, double *ny);

public:
	static bool has_base_;

	scaled_blob(process_view *pv);
	virtual ~scaled_blob();

	static bool is_vertical() { return vertical_; }

	static void scale_point(double x, double y, double *nx, double *ny);
};
