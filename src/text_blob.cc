
/*
 * text_blob.cc
 * Copyright (C) 1999-2001 by John Heidemann
 * $Id: text_blob.cc,v 1.7 2003/04/19 21:58:46 johnh Exp $
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

#ifdef USE_TEXT_BLOB

#include <stdlib.h>  // atoi
#include <tcl.h>

#include <iostream>

#include "main.hh"
#include "text_blob.hh"
#include "process_list.hh"

// #include <algorithm>

// instantiate me
blob *
blob::create_real_blob(process_view *pv)
{
	ENTRY_TRACE(__FILE__,__LINE__);
	return (blob*) new text_blob(pv);
}



// static
void
text_blob::mainloop()
{
	ENTRY_TRACE(__FILE__,__LINE__);
	static process_list pl;
	for (;;) {
		sleep(1);
		pl.scan();
		cout << endl;
	};
}


void
text_blob::update_canvas()
{
	ENTRY_TRACE(__FILE__,__LINE__);
	cout << "x";
}

void
text_blob::op(blob_op op, int delta)
{
	ENTRY_TRACE(__FILE__,__LINE__);
	blob::op(op, delta);
	update_canvas();
}

#endif /* USE_TEXT_BLOB */
