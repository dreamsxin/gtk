/* GDK - The GIMP Drawing Kit
 * Copyright (C) 2009 Carlos Garnacho <carlosg@gnome.org>
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

#include "config.h"
#include <stdlib.h>

#include "gdkdevice-broadway.h"

#include "gdkwindow.h"
#include "gdkprivate-broadway.h"

static gboolean gdk_broadway_device_get_history (GdkDevice      *device,
						 GdkWindow      *window,
						 guint32         start,
						 guint32         stop,
						 GdkTimeCoord ***events,
						 gint           *n_events);
static void gdk_broadway_device_get_state (GdkDevice       *device,
					   GdkWindow       *window,
					   gdouble         *axes,
					   GdkModifierType *mask);
static void gdk_broadway_device_set_window_cursor (GdkDevice *device,
						   GdkWindow *window,
						   GdkCursor *cursor);
static void gdk_broadway_device_warp (GdkDevice *device,
				      gdouble    x,
				      gdouble    y);
static void gdk_broadway_device_query_state (GdkDevice        *device,
                                             GdkWindow        *window,
                                             GdkWindow       **child_window,
                                             gdouble          *root_x,
                                             gdouble          *root_y,
                                             gdouble          *win_x,
                                             gdouble          *win_y,
                                             GdkModifierType  *mask);
static GdkGrabStatus gdk_broadway_device_grab   (GdkDevice     *device,
						 GdkWindow     *window,
						 gboolean       owner_events,
						 GdkEventMask   event_mask,
						 GdkWindow     *confine_to,
						 GdkCursor     *cursor,
						 guint32        time_);
static void          gdk_broadway_device_ungrab (GdkDevice     *device,
						 guint32        time_);
static GdkWindow * gdk_broadway_device_window_at_position (GdkDevice       *device,
							   gdouble         *win_x,
							   gdouble         *win_y,
							   GdkModifierType *mask,
							   gboolean         get_toplevel);
static void      gdk_broadway_device_select_window_events (GdkDevice       *device,
							   GdkWindow       *window,
							   GdkEventMask     event_mask);


G_DEFINE_TYPE (GdkBroadwayDevice, gdk_broadway_device, GDK_TYPE_DEVICE)

static void
gdk_broadway_device_class_init (GdkBroadwayDeviceClass *klass)
{
  GdkDeviceClass *device_class = GDK_DEVICE_CLASS (klass);

  device_class->get_history = gdk_broadway_device_get_history;
  device_class->get_state = gdk_broadway_device_get_state;
  device_class->set_window_cursor = gdk_broadway_device_set_window_cursor;
  device_class->warp = gdk_broadway_device_warp;
  device_class->query_state = gdk_broadway_device_query_state;
  device_class->grab = gdk_broadway_device_grab;
  device_class->ungrab = gdk_broadway_device_ungrab;
  device_class->window_at_position = gdk_broadway_device_window_at_position;
  device_class->select_window_events = gdk_broadway_device_select_window_events;
}

static void
gdk_broadway_device_init (GdkBroadwayDevice *device_core)
{
  GdkDevice *device;

  device = GDK_DEVICE (device_core);

  _gdk_device_add_axis (device, NULL, GDK_AXIS_X, 0, 0, 1);
  _gdk_device_add_axis (device, NULL, GDK_AXIS_Y, 0, 0, 1);
}

static gboolean
gdk_broadway_device_get_history (GdkDevice      *device,
				 GdkWindow      *window,
				 guint32         start,
				 guint32         stop,
				 GdkTimeCoord ***events,
				 gint           *n_events)
{
  return FALSE;
}

static void
gdk_broadway_device_get_state (GdkDevice       *device,
			       GdkWindow       *window,
			       gdouble         *axes,
			       GdkModifierType *mask)
{
  gdouble x, y;

  gdk_window_get_device_position_double (window, device, &x, &y, mask);

  if (axes)
    {
      axes[0] = x;
      axes[1] = y;
    }
}

static void
gdk_broadway_device_set_window_cursor (GdkDevice *device,
				       GdkWindow *window,
				       GdkCursor *cursor)
{
}

static void
gdk_broadway_device_warp (GdkDevice *device,
			  gdouble    x,
			  gdouble    y)
{
}

static void
gdk_broadway_device_query_state (GdkDevice        *device,
				 GdkWindow        *window,
				 GdkWindow       **child_window,
				 gdouble          *root_x,
				 gdouble          *root_y,
				 gdouble          *win_x,
				 gdouble          *win_y,
				 GdkModifierType  *mask)
{
  GdkDisplay *display;
  GdkBroadwayDisplay *broadway_display;
  gint32 device_root_x, device_root_y;
  guint32 mouse_toplevel_id;
  guint32 mask32;

  if (gdk_device_get_source (device) != GDK_SOURCE_MOUSE)
    return;

  display = gdk_device_get_display (device);
  broadway_display = GDK_BROADWAY_DISPLAY (display);

  _gdk_broadway_server_query_mouse (broadway_display->server,
				    &mouse_toplevel_id,
				    &device_root_x,
				    &device_root_y,
				    &mask32);

  if (root_x)
    *root_x = device_root_x;
  if (root_y)
    *root_y = device_root_y;
  if (win_x)
    *win_x = device_root_x;
  if (win_y)
    *win_y = device_root_y;
  if (mask)
    *mask = mask32;
  if (child_window)
    {
      GdkWindow *mouse_toplevel;

      mouse_toplevel = g_hash_table_lookup (broadway_display->id_ht, GUINT_TO_POINTER (mouse_toplevel_id));
      if (window == NULL)
	*child_window = mouse_toplevel;
      else
	*child_window = NULL;
    }

  return;
}

void
_gdk_broadway_window_grab_check_unmap (GdkWindow *window,
				       gulong     serial)
{
  GdkDisplay *display = gdk_window_get_display (window);
  GdkSeat *seat;
  GList *devices, *d;

  seat = gdk_display_get_default_seat (display);

  devices = gdk_seat_get_slaves (seat, GDK_SEAT_CAPABILITY_ALL);
  devices = g_list_prepend (devices, gdk_seat_get_keyboard (seat));
  devices = g_list_prepend (devices, gdk_seat_get_pointer (seat));

  /* End all grabs on the newly hidden window */
  for (d = devices; d; d = d->next)
    _gdk_display_end_device_grab (display, d->data, serial, window, TRUE);

  g_list_free (devices);
}


void
_gdk_broadway_window_grab_check_destroy (GdkWindow *window)
{
  GdkDisplay *display = gdk_window_get_display (window);
  GdkSeat *seat;
  GdkDeviceGrabInfo *grab;
  GList *devices, *d;

  seat = gdk_display_get_default_seat (display);

  devices = NULL;
  devices = g_list_prepend (devices, gdk_seat_get_keyboard (seat));
  devices = g_list_prepend (devices, gdk_seat_get_pointer (seat));

  for (d = devices; d; d = d->next)
    {
      /* Make sure there is no lasting grab in this native window */
      grab = _gdk_display_get_last_device_grab (display, d->data);

      if (grab && grab->native_window == window)
	{
	  grab->serial_end = grab->serial_start;
	  grab->implicit_ungrab = TRUE;
	}

    }

  g_list_free (devices);
}


static GdkGrabStatus
gdk_broadway_device_grab (GdkDevice    *device,
			  GdkWindow    *window,
			  gboolean      owner_events,
			  GdkEventMask  event_mask,
			  GdkWindow    *confine_to,
			  GdkCursor    *cursor,
			  guint32       time_)
{
  GdkDisplay *display;
  GdkBroadwayDisplay *broadway_display;

  display = gdk_device_get_display (device);
  broadway_display = GDK_BROADWAY_DISPLAY (display);

  if (gdk_device_get_source (device) == GDK_SOURCE_KEYBOARD)
    {
      /* Device is a keyboard */
      return GDK_GRAB_SUCCESS;
    }
  else
    {
      /* Device is a pointer */
      return _gdk_broadway_server_grab_pointer (broadway_display->server,
						GDK_WINDOW_IMPL_BROADWAY (window->impl)->id,
						owner_events,
						event_mask,
						time_);
    }
}

#define TIME_IS_LATER(time1, time2)                        \
  ( (( time1 > time2 ) && ( time1 - time2 < ((guint32)-1)/2 )) ||  \
    (( time1 < time2 ) && ( time2 - time1 > ((guint32)-1)/2 ))     \
  )

static void
gdk_broadway_device_ungrab (GdkDevice *device,
			    guint32    time_)
{
  GdkDisplay *display;
  GdkBroadwayDisplay *broadway_display;
  GdkDeviceGrabInfo *grab;
  guint32 serial;

  display = gdk_device_get_display (device);
  broadway_display = GDK_BROADWAY_DISPLAY (display);

  if (gdk_device_get_source (device) == GDK_SOURCE_KEYBOARD)
    {
      /* Device is a keyboard */
    }
  else
    {
      /* Device is a pointer */
      serial = _gdk_broadway_server_ungrab_pointer (broadway_display->server, time_);

      if (serial != 0)
	{
	  grab = _gdk_display_get_last_device_grab (display, device);
	  if (grab &&
	      (time_ == GDK_CURRENT_TIME ||
	       grab->time == GDK_CURRENT_TIME ||
	       !TIME_IS_LATER (grab->time, time_)))
	    grab->serial_end = serial;
	}
    }
}

static GdkWindow *
gdk_broadway_device_window_at_position (GdkDevice       *device,
					gdouble         *win_x,
					gdouble         *win_y,
					GdkModifierType *mask,
					gboolean         get_toplevel)
{
  GdkWindow *window;

  gdk_broadway_device_query_state (device, NULL, &window, NULL, NULL, win_x, win_y, mask);

  return window;
}

static void
gdk_broadway_device_select_window_events (GdkDevice    *device,
					  GdkWindow    *window,
					  GdkEventMask  event_mask)
{
}