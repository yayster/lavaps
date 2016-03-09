
/*
 * gtk_compat.hh
 * Copyright (C) 2003 by John Heidemann
 * $Id: gtk_compat.hh,v 1.4 2003/06/27 13:47:19 johnh Exp $
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

#include <gtk/gtk.h>
#include <libgnomecanvas/libgnomecanvas.h>

extern void gtk_menu_item_set_label_markup(GtkMenuItem *menu_item, const char *label);
extern void gnome_canvas_item_lower_below(GnomeCanvasItem *inferior, GnomeCanvasItem *superior);
extern GnomeCanvasPoints *gnome_canvas_points_smooth(GnomeCanvasPoints *src, int splineSteps);

#endif /* USE_GTK_BLOB */
