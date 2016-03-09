/* -*-	Mode:C++ -*- */

/*
 * gtk_blob.hh
 * Copyright (C) 2003 by John Heidemann
 * $Id: gtk_lava_help.hh,v 1.3 2004/01/28 05:38:45 johnh Exp $
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

#ifdef USE_GTK_BLOB

#include <string.h>
#include "main.hh"  // for N_

class gtk_lava_help_text {
protected:
	static gtk_lava_help_text *first_;  // singly linked list
	const char *key_;
	const char *body_;
	const char *title_;
	gtk_lava_help_text *next_;  // singly linked list
public:
	gtk_lava_help_text(const char *key, const char *body, const char *title) :
		key_(key), body_(body), title_(title), next_(NULL) {
		// add us to the linked list
		next_ = first_;
		first_ = this;
	}
	static gtk_lava_help_text *find_by_key(const char *key) {
		gtk_lava_help_text *i = first_;
		while (i) {
			if (strcmp(key, i->key_) == 0)
				return i;
			i = i->next_;
		};
		return NULL;
	}
	// xxx: should we _(translate), or expect the caller to?  what's the right convention?
	const char *rawbody() { return body_; }
	const char *body() { return _(body_); }
	const char *title() { return _(title_); }
};

#endif /* USE_GTK_BLOB */

