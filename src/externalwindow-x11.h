/*
 * Copyright © 2016 Red Hat, Inc
 *
 * This program is free software; you can redistribute it and/or
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
 *
 * Authors:
 *       Jonas Ådahl <jadahl@redhat.com>
 */

#pragma once

#include <glib-object.h>

#include "externalwindow.h"


#define EXTERNAL_TYPE_WINDOW_X11 (external_window_x11_get_type ())
G_DECLARE_FINAL_TYPE (ExternalWindowX11, external_window_x11,
                      EXTERNAL, WINDOW_X11, ExternalWindow)

ExternalWindowX11 *external_window_x11_new (const char *handle_str);

GdkDisplay * init_external_window_x11_display (GError **error);