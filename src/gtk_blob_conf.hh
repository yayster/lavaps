/* -*-	Mode:C++ -*- */

/*
 * gtk_blob_conf.hh
 * Copyright (C) 2003 by John Heidemann
 * $Id: gtk_blob_conf.hh,v 1.7 2003/07/15 03:31:07 johnh Exp $
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

#ifndef lavaps_gtk_blob_conf_hh
#define lavaps_gtk_blob_conf_hh

#ifdef USE_GTK_BLOB

#include <map>

#include <gconf/gconf-client.h>

#include "gtk_blob.hh"

struct ltstr {
	bool operator()(const char* s1, const char* s2) const {
		return strcmp(s1, s2) < 0;
	}
};


/*
 * gtk_blob_conf:
 * wraps around the UI and gconf stuff,
 * allowing one to change the other,
 * and keeping all sync'ed with our variables.
 */
class gtk_blob_conf {
protected: 
	static GConfClient *client_;
	typedef map<const char *, gtk_blob_conf*, ltstr> key_to_conf_map_t;
	static key_to_conf_map_t key_to_conf_map_;

	const char *key_;
	GConfValueType gconf_type_;

	static void init();
	static void check_init() { if (client_ == NULL) init(); };

	static void conf_callback(GConfClient* client, guint cnxn_id,
				  GConfEntry *entry, gpointer user_data);
	void read_from_conf();
	void from_entry_validated(GConfEntry *entry);

public:
	gtk_blob_conf(const char *key, GConfValueType gconf_type);
	virtual void set_default_value() = 0;
	virtual void to_ui() = 0;
	virtual void to_conf() = 0;
	virtual void from_ui() = 0;
	virtual void from_conf(GConfEntry *entry) = 0;
};

class gtk_blob_conf_bool : public gtk_blob_conf {
	bool *value_;
	bool default_value_;
	GtkCheckMenuItem *ui_item_;
	
public:
	gtk_blob_conf_bool(bool *value, bool default_value, const char *gconf_key, GtkCheckMenuItem *ui_item);

	virtual void set_default_value() { *value_ = default_value_; };
	virtual void to_ui();
	virtual void from_ui();
	virtual void to_conf();
	virtual void from_conf(GConfEntry *entry);
};

class gtk_blob_conf_enum_mapping {
public:
	int value_;
	const char *name_;
	GtkRadioMenuItem *item_;

	gtk_blob_conf_enum_mapping(int value, const char *name, GtkRadioMenuItem *item) :
		value_(value), name_(name), item_(item) {};
};

class gtk_blob_conf_enum : public gtk_blob_conf {
	int *value_;
	int my_value_;
	list<gtk_blob_conf_enum_mapping*> *mapping_;

public:
	gtk_blob_conf_enum(int *value, int my_value, const char *gconf_key, list<gtk_blob_conf_enum_mapping*> *mapping); // default is first in mapping

	virtual void set_default_value() { *value_ = mapping_->front()->value_; };
	virtual void to_ui();
	virtual void from_ui();
	virtual void to_conf();
	virtual void from_conf(GConfEntry *entry);
};

class gtk_blob_conf_double : public gtk_blob_conf {
	double *value_;
	double default_value_;
	double min_, max_;
	// only indirect menu interaction
	
public:
	gtk_blob_conf_double(double *value, double default_value, 
			     double min, double max, const char *key);

	virtual void set_default_value() { *value_ = default_value_; };
	virtual void to_ui();
	virtual void from_ui();
	virtual void to_conf();
	virtual void from_conf(GConfEntry *entry);
};

class gtk_blob_conf_int : public gtk_blob_conf {
	int *value_;
	int default_value_;
	int min_, max_;
	// only indirect menu interaction
	
public:
	gtk_blob_conf_int(int *value, int default_value, 
			     int min, int max, const char *key);

	virtual void set_default_value() { *value_ = default_value_; };
	virtual void to_ui();
	virtual void from_ui();
	virtual void to_conf();
	virtual void from_conf(GConfEntry *entry);
};

#endif /* USE_GTK_BLOB */

#endif /* ! lavaps_gtk_blob_conf_hh */
