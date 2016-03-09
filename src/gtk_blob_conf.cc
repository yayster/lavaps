
/*
 * gtk_blob_conf.cc
 * Copyright (C) 2003 by John Heidemann
 * $Id: gtk_blob_conf.cc,v 1.15 2004/06/07 23:40:56 johnh Exp $
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

#include <iostream>
#include <algorithm>

#include "main.hh"
#include "gtk_blob.hh"

GConfClient *gtk_blob_conf::client_ = NULL;
gtk_blob_conf::key_to_conf_map_t gtk_blob_conf::key_to_conf_map_;


gtk_blob_conf::gtk_blob_conf(const char *key, GConfValueType gconf_type) :
		key_(key), gconf_type_(gconf_type)
{
	/*
	 * Some conf entries can share keys, assuming they all point to
	 * the same place.  We can't check that here, but we don't 
	 * double-enter keys.
	 */
	if (key_to_conf_map_.find(key) == key_to_conf_map_.end()) {
		// new, enter it
		key_to_conf_map_[key_] = this;
	};
}

void
gtk_blob_conf::init()
{
	if (client_ != NULL)
		return;

	client_ = gconf_client_get_default();
	// xxx: should handle case of not being able to contact server
	assert(client_ != NULL);
	// watch our keys
	gconf_client_add_dir (client_, O_("/apps/lavaps"),
                        GCONF_CLIENT_PRELOAD_NONE, NULL);
	gconf_client_notify_add(client_, O_("/apps/lavaps"),
				conf_callback,
				NULL, NULL, NULL);
}

void
gtk_blob_conf::read_from_conf()
{
	/*
	 * questions:
	 * locale (3rd arg): NULL, or gconf_current_locale()
	 * default (4th arg): does it FORCE the default, or just use it?
	 */
	assert(client_ != NULL);
	assert(key_ != NULL);
	GConfEntry *entry = gconf_client_get_entry(client_, key_, NULL, FALSE, NULL);
	from_entry_validated(entry);
	if (entry)
		gconf_entry_free(entry);
}

void
gtk_blob_conf::conf_callback(GConfClient* client, guint cnxn_id,
			     GConfEntry *entry, gpointer user_data)
{
	assert(client == client_);

	// first, find the entry
	key_to_conf_map_t::iterator it = key_to_conf_map_.find(gconf_entry_get_key(entry));
	if (it == key_to_conf_map_.end()) {
		cerr << _("lavaps warning: gconf callback on unknown key ") << gconf_entry_get_key(entry) << endl;
		return;
	};
	gtk_blob_conf *gbc = (*it).second;

	gbc->from_entry_validated(entry);

	// assumes caller will free entry
}

void
gtk_blob_conf::from_entry_validated(GConfEntry *entry)
{
	// typechecking
	if (entry == NULL || gconf_entry_get_value(entry) == NULL) {
		set_default_value();
		// suppress lack of default silently
		return;
	};
	if (gconf_entry_get_value(entry)->type != gconf_type_) {
		cerr << _("lavaps warning: gconf key ") << gconf_entry_get_key(entry) 
		     << _(" is wrong type (")
		     << gconf_type_
		     << _(" expected, but got ")
		     << gconf_entry_get_value(entry)->type
		     << _("), so instead using default.\n");
		set_default_value();
		return;
	};

	// made it this far => set it
	from_conf(entry);
}


/***********************************************************************/

gtk_blob_conf_bool::gtk_blob_conf_bool(bool *value,
				       bool default_value,
				       const char *key,
				       GtkCheckMenuItem *ui_item) :
	gtk_blob_conf(key, GCONF_VALUE_BOOL),
	value_(value),
	default_value_(default_value),
	ui_item_(ui_item)
{
	check_init();
	set_default_value();
	read_from_conf();
	to_ui();
}

void
gtk_blob_conf_bool::to_ui()
{
	ui_item_->active = *value_;
}

void
gtk_blob_conf_bool::from_ui()
{
	// true value is just stored in gtk structure
	*value_ = ui_item_->active;
	to_conf();
}

void
gtk_blob_conf_bool::to_conf()
{
	gconf_client_set_bool(client_, key_, (*value_ ? TRUE : FALSE), NULL);
}

void
gtk_blob_conf_bool::from_conf(GConfEntry *entry)
{
	*value_ = gconf_value_get_bool(gconf_entry_get_value(entry));
	to_ui();
	// cerr << "gtk_blob_conf_bool::from_conf " << gconf_entry_get_key(entry) << " is " << (*value_ ? "true" : "false") << endl;
}

/***********************************************************************/

struct conf_enum_find_value_p : public unary_function<int,bool> {
	conf_enum_find_value_p(int value) : value_(value) {};
	int value_;
	bool operator()(gtk_blob_conf_enum_mapping*p) { return p->value_ == value_; }
};

struct conf_enum_find_name_p : public unary_function<const char*,bool> {
	conf_enum_find_name_p(const char *name) : name_(name) {};
	const char *name_;
	bool operator()(gtk_blob_conf_enum_mapping*p) { return strcasecmp(p->name_,name_) == 0; }
};



gtk_blob_conf_enum::gtk_blob_conf_enum(int *value,
				       int my_value,
				       const char *key,
				       list<gtk_blob_conf_enum_mapping*> *mapping) :
	gtk_blob_conf(key, GCONF_VALUE_STRING),
	value_(value),
	my_value_(my_value),
	mapping_(mapping)
{
	check_init();
	set_default_value();
	read_from_conf();
	to_ui();
}

void
gtk_blob_conf_enum::to_ui()
{
	for (list<gtk_blob_conf_enum_mapping*>::iterator it = mapping_->begin();
	     it != mapping_->end();
	     it++) {
		GtkCheckMenuItem *check_item = GTK_CHECK_MENU_ITEM((*it)->item_);
		check_item->active = ((*it)->value_ == *value_) ? TRUE : FALSE;
	};
}

void
gtk_blob_conf_enum::from_ui()
{
	/*
	 * While in bool, we pull the value from the UI,
	 * here we know the proper state from our state.
	 */
	// cerr << "gtk_blob_conf_enum::from_ui: found on " << key_ << endl;
	*value_ = my_value_;
	to_conf();
}

void
gtk_blob_conf_enum::to_conf()
{
	list<gtk_blob_conf_enum_mapping*>::iterator it = 
		find_if(mapping_->begin(), 
		     mapping_->end(), 
		     conf_enum_find_value_p(*value_));
	assert(it != mapping_->end());
	gconf_client_set_string(client_, key_, (*it)->name_, NULL);
}

void
gtk_blob_conf_enum::from_conf(GConfEntry *entry)
{
	const char *new_value = gconf_value_get_string(gconf_entry_get_value(entry));
	list<gtk_blob_conf_enum_mapping*>::iterator it =
		find_if(mapping_->begin(), 
		     mapping_->end(), 
		     conf_enum_find_name_p(new_value));
	if (it == mapping_->end()) {
		cerr << _("lavaps warning: gconf key ") << key_
		     << _(" has unknown value ")
		     << new_value
		     << _(", so instead using default.\n");
		set_default_value();
		to_ui();
		return;
	};
	*value_ = (*it)->value_;
	to_ui();
	// cerr << "gtk_blob_conf_enum::from_conf " << gconf_entry_get_key(entry) << " is " << (*it)->name_ << endl;
}

/***********************************************************************/

gtk_blob_conf_double::gtk_blob_conf_double(double *value, double default_value, 
					   double min, double max, 
					   const char *key) :
	gtk_blob_conf(key, GCONF_VALUE_FLOAT),
	value_(value),
	default_value_(default_value),
	min_(min),
	max_(max)
{
	check_init();
	set_default_value();
	read_from_conf();
	to_ui();
}

void
gtk_blob_conf_double::to_ui()
{
	// no UI
}

void
gtk_blob_conf_double::from_ui()
{
	// no UI
	to_conf();
}

void
gtk_blob_conf_double::to_conf()
{
	// cerr << "gtk_blob_conf_double::to_conf " << key_ << " is " << *value_ << endl;
	gconf_client_set_float(client_, key_, *value_, NULL);
}

void
gtk_blob_conf_double::from_conf(GConfEntry *entry)
{
	*value_ = gconf_value_get_float(gconf_entry_get_value(entry));
	to_ui();
	// cerr << "gtk_blob_conf_double::from_conf " << key_ << " is " << *value_ << endl;
}

/***********************************************************************/

gtk_blob_conf_int::gtk_blob_conf_int(int *value, int default_value, 
					   int min, int max, 
					   const char *key) :
	gtk_blob_conf(key, GCONF_VALUE_INT),
	value_(value),
	default_value_(default_value),
	min_(min),
	max_(max)
{
	check_init();
	set_default_value();
	read_from_conf();
	to_ui();
}

void
gtk_blob_conf_int::to_ui()
{
	// no UI
}

void
gtk_blob_conf_int::from_ui()
{
	// no UI
	to_conf();
}

void
gtk_blob_conf_int::to_conf()
{
	// cerr << "gtk_blob_conf_int::to_conf(): " << key_ << " is " << *value_ << endl;
	gconf_client_set_int(client_, key_, *value_, NULL);
}

void
gtk_blob_conf_int::from_conf(GConfEntry *entry)
{
	*value_ = gconf_value_get_int(gconf_entry_get_value(entry));
	to_ui();
	// cerr << "gtk_blob_conf_int::from_conf " << key_ << " is " << gconf_entry_get_key(entry) << " is " << *value_ << endl;
}

#endif /* USE_GTK_BLOB */

