/* -*-	Mode:C++ -*- */

/*
 * text_blob.h
 * Copyright (C) 1999 by John Heidemann
 * $Id: text_blob.hh,v 1.5 1999/09/06 18:16:16 johnh Exp $
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

#ifndef lavaps_text_blob_h
#define lavaps_text_blob_h

#ifdef USE_TEXT_BLOB

#include "blob.hh"
#include "const_str.hh"


class text_blob : public blob {
private:
	text_blob(const text_blob& tb);
	text_blob& operator=(const text_blob& tb);

protected:
	virtual void op(blob_op op, int delta);

	void update_canvas();

public:
	text_blob(process_view *pv) : blob(pv) {};
	virtual ~text_blob() {};

	static void mainloop();
};

#endif /* USE_TEXT_BLOB */

#endif /* ! lavaps_text_blob_h */

