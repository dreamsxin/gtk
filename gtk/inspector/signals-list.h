/*
 * Copyright (c) 2014 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _GTK_INSPECTOR_SIGNALS_LIST_H_
#define _GTK_INSPECTOR_SIGNALS_LIST_H_

#include <gtk/gtkpaned.h>

#define GTK_TYPE_INSPECTOR_SIGNALS_LIST            (gtk_inspector_signals_list_get_type())
#define GTK_INSPECTOR_SIGNALS_LIST(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), GTK_TYPE_INSPECTOR_SIGNALS_LIST, GtkInspectorSignalsList))
#define GTK_INSPECTOR_SIGNALS_LIST_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), GTK_TYPE_INSPECTOR_SIGNALS_LIST, GtkInspectorSignalsListClass))
#define GTK_INSPECTOR_IS_SIGNALS_LIST(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), GTK_TYPE_INSPECTOR_SIGNALS_LIST))
#define GTK_INSPECTOR_IS_SIGNALS_LIST_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), GTK_TYPE_INSPECTOR_SIGNALS_LIST))
#define GTK_INSPECTOR_SIGNALS_LIST_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), GTK_TYPE_INSPECTOR_SIGNALS_LIST, GtkInspectorSignalsListClass))


typedef struct _GtkInspectorSignalsListPrivate GtkInspectorSignalsListPrivate;

typedef struct _GtkInspectorSignalsList
{
  GtkPaned parent;
  GtkInspectorSignalsListPrivate *priv;
} GtkInspectorSignalsList;

typedef struct _GtkInspectorSignalsListClass
{
  GtkPanedClass parent;
} GtkInspectorSignalsListClass;

G_BEGIN_DECLS

GType      gtk_inspector_signals_list_get_type   (void);
void       gtk_inspector_signals_list_set_object (GtkInspectorSignalsList *sl,
                                                  GObject                 *object);

G_END_DECLS

#endif // _GTK_INSPECTOR_SIGNALS_LIST_H_

// vim: set et sw=2 ts=2: