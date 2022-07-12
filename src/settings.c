/*
 * Copyright © 2018 Igalia S.L.
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
 *       Patrick Griffis <pgriffis@igalia.com>
 */

#include "config.h"

#include <time.h>
#include <string.h>
#include <glib/gi18n.h>
#include <gio/gio.h>

#include "settings.h"
#include "utils.h"
#include "shellintrospect.h"

#include "xdg-desktop-portal-dbus.h"
#include "fc-monitor.h"

static GHashTable *settings;
static FcMonitor *fontconfig_monitor;
static int fontconfig_serial;
static gboolean enable_animations;

static void sync_animations_enabled (XdpImplSettings *impl, ShellIntrospect *shell_introspect);

typedef struct {
  GSettingsSchema *schema;
  GSettings *settings;
} SettingsBundle;

typedef enum {
   MULTI = 0,
   PURPLE = 1,
   PINK = 2,
   RED = 3,
   ORANGE = 4,
   YELLOW = 5,
   GREEN = 6,
   MINT = 7,
   BLUE = 8,
   MONO = 9
 } AccentColor;

static SettingsBundle *
settings_bundle_new (GSettingsSchema *schema,
                     GSettings       *settings)
{
  SettingsBundle *bundle = g_new (SettingsBundle, 1);
  bundle->schema = schema;
  bundle->settings = settings;
  return bundle;
}

static void
settings_bundle_free (SettingsBundle *bundle)
{
  g_object_unref (bundle->schema);
  g_object_unref (bundle->settings);
  g_free (bundle);
}

static gboolean
namespace_matches (const char         *namespace,
                   const char * const *patterns)
{
  size_t i;

  for (i = 0; patterns[i]; ++i)
    {
      size_t pattern_len;
      const char *pattern = patterns[i];

      if (pattern[0] == '\0')
        return TRUE;
      if (strcmp (namespace, pattern) == 0)
        return TRUE;

      pattern_len = strlen (pattern);
      if (pattern[pattern_len - 1] == '*' && strncmp (namespace, pattern, pattern_len - 1) == 0)
        return TRUE;
    }

  if (i == 0) /* Empty array */
    return TRUE;

  return FALSE;
}

static GVariant *
get_color_scheme (void)
{
  SettingsBundle *bundle = g_hash_table_lookup (settings, "org.gnome.desktop.interface");
  int color_scheme;

  if (!g_settings_schema_has_key (bundle->schema, "color-scheme"))
    return g_variant_new_uint32 (0); /* No preference */

  color_scheme = g_settings_get_enum (bundle->settings, "color-scheme");

  return g_variant_new_uint32 (color_scheme);
}

static GVariant *
 get_accent_color (void)
 {
   SettingsBundle *bundle = g_hash_table_lookup (settings, "co.tauos.desktop.appearance");

   if (!g_settings_schema_has_key (bundle->schema, "accent-color"))
     return g_variant_new_uint32 (0); /* No preference */

   AccentColor color = g_settings_get_enum (bundle->settings, "accent-color");

   switch (color) {
     case PURPLE:
       GVariant * purple[] = {
         g_variant_new_double (0.5490),
         g_variant_new_double (0.3372),
         g_variant_new_double (0.7490)
       };
       return g_variant_new_tuple(purple, 3);
     case PINK:
       GVariant * pink[] = {
         g_variant_new_double (0.7490),
         g_variant_new_double (0.3372),
         g_variant_new_double (0.6588)
       };
       return g_variant_new_tuple(pink, 3);
     case RED:
       GVariant * red[] = {
         g_variant_new_double (0.8588),
         g_variant_new_double (0.1568),
         g_variant_new_double (0.3764)
       };
       return g_variant_new_tuple(red, 3);
     case ORANGE:
       GVariant * orange[] = {
         g_variant_new_double (0.9686),
         g_variant_new_double (0.5058),
         g_variant_new_double (0.168)
       };
       return g_variant_new_tuple(orange, 3);
     case YELLOW:
       GVariant * yellow[] = {
         g_variant_new_double (0.8784),
         g_variant_new_double (0.6313),
         g_variant_new_double (0.0039)
       };
       return g_variant_new_tuple(yellow, 3);
     case GREEN:
       GVariant * green[] = {
         g_variant_new_double (0.2862),
         g_variant_new_double (0.8156),
         g_variant_new_double (0.3686)
       };
       return g_variant_new_tuple(green, 3);
     case MINT:
       GVariant * mint[] = {
         g_variant_new_double (0.3372),
         g_variant_new_double (0.7490),
         g_variant_new_double (0.6509)
       };
       return g_variant_new_tuple(mint, 3);
     case BLUE:
       GVariant * blue[] = {
         g_variant_new_double (0.1490),
         g_variant_new_double (0.5568),
         g_variant_new_double (0.9764)
       };
       return g_variant_new_tuple(blue, 3);
     case MONO:
       GVariant * mono[] = {
         g_variant_new_double (0.3334),
         g_variant_new_double (0.3334),
         g_variant_new_double (0.3334)
       };
       return g_variant_new_tuple(mono, 3);
     case MULTI:
     default:
       return g_variant_new_uint32 (0); /* Unknown color or multicolor mode */
   }
 }

static GVariant *
get_theme_value (const char *key)
{
  SettingsBundle *bundle = g_hash_table_lookup (settings, "org.gnome.desktop.a11y.interface");
  const char *theme;
  gboolean hc = FALSE;

  if (bundle && g_settings_schema_has_key (bundle->schema, "high-contrast"))
    hc = g_settings_get_boolean (bundle->settings, "high-contrast");

  if (hc)
    return g_variant_new_string ("HighContrast");

  bundle = g_hash_table_lookup (settings, "org.gnome.desktop.interface");
  theme = g_settings_get_string (bundle->settings, key);

  return g_variant_new_string (theme);
}

static gboolean
settings_handle_read_all (XdpImplSettings       *object,
                          GDBusMethodInvocation *invocation,
                          const char * const    *arg_namespaces,
                          gpointer               data)
{
  g_autoptr(GVariantBuilder) builder = g_variant_builder_new (G_VARIANT_TYPE ("(a{sa{sv}})"));
  GHashTableIter iter;
  char *key;
  SettingsBundle *value;

  g_variant_builder_open (builder, G_VARIANT_TYPE ("a{sa{sv}}"));

  g_hash_table_iter_init (&iter, settings);
  while (g_hash_table_iter_next (&iter, (gpointer *)&key, (gpointer *)&value))
    {
      g_auto (GStrv) keys = NULL;
      GVariantDict dict;
      gsize i;

      if (!namespace_matches (key, arg_namespaces))
        continue;

      keys = g_settings_schema_list_keys (value->schema);
      g_variant_dict_init (&dict, NULL);
      for (i = 0; keys[i]; ++i)
        {
          if (strcmp (key, "org.gnome.desktop.interface") == 0 &&
              strcmp (keys[i], "enable-animations") == 0)
            g_variant_dict_insert_value (&dict, keys[i], g_variant_new_boolean (enable_animations));
          else if (strcmp (key, "org.gnome.desktop.interface") == 0 &&
                   (strcmp (keys[i], "gtk-theme") == 0 || strcmp (keys[i], "icon-theme") == 0))
            g_variant_dict_insert_value (&dict, keys[i], get_theme_value (keys[i]));
          else
            g_variant_dict_insert_value (&dict, keys[i], g_settings_get_value (value->settings, keys[i]));
        }

      g_variant_builder_add (builder, "{s@a{sv}}", key, g_variant_dict_end (&dict));
    }

  if (namespace_matches ("org.gnome.fontconfig", arg_namespaces))
    {
      GVariantDict dict;

      g_variant_dict_init (&dict, NULL);
      g_variant_dict_insert_value (&dict, "serial", g_variant_new_int32 (fontconfig_serial));

      g_variant_builder_add (builder, "{s@a{sv}}", "org.gnome.fontconfig", g_variant_dict_end (&dict));
    }

  if (namespace_matches ("org.freedesktop.appearance", arg_namespaces))
    {
      GVariantDict dict;

      g_variant_dict_init (&dict, NULL);
      g_variant_dict_insert_value (&dict, "color-scheme", get_color_scheme ());
      g_variant_dict_insert_value (&dict, "accent-color", get_accent_color ());

      g_variant_builder_add (builder, "{s@a{sv}}", "org.freedesktop.appearance", g_variant_dict_end (&dict));
    }

  g_variant_builder_close (builder);

  g_dbus_method_invocation_return_value (invocation, g_variant_builder_end (builder));

  return TRUE;
}

static gboolean
settings_handle_read (XdpImplSettings       *object,
                      GDBusMethodInvocation *invocation,
                      const char            *arg_namespace,
                      const char            *arg_key,
                      gpointer               data)
{
  g_debug ("Read %s %s", arg_namespace, arg_key);

  if (strcmp (arg_namespace, "org.gnome.fontconfig") == 0)
    {
      if (strcmp (arg_key, "serial") == 0)
        {
          g_dbus_method_invocation_return_value (invocation,
                                                 g_variant_new ("(v)", g_variant_new_int32 (fontconfig_serial)));
          return TRUE;
        }
    }
  else if (strcmp (arg_namespace, "org.freedesktop.appearance") == 0 &&
           strcmp (arg_key, "color-scheme") == 0)
    {
      g_dbus_method_invocation_return_value (invocation,
                                             g_variant_new ("(v)", get_color_scheme ()));
      return TRUE;
    }
  else if (strcmp (arg_namespace, "org.freedesktop.appearance") == 0 &&
           strcmp (arg_key, "accent-color") == 0)
    {
      g_dbus_method_invocation_return_value (invocation,
                                             g_variant_new ("(v)", get_accent_color ()));
      return TRUE;
    }
  else if (strcmp (arg_namespace, "org.gnome.desktop.interface") == 0 &&
           strcmp (arg_key, "enable-animations") == 0)
    {
      g_dbus_method_invocation_return_value (invocation,
                                             g_variant_new ("(v)", g_variant_new_boolean (enable_animations)));
      return TRUE;
    }
  else if (strcmp (arg_namespace, "org.gnome.desktop.interface") == 0 &&
           (strcmp (arg_key, "gtk-theme") == 0 || strcmp (arg_key, "icon-theme") == 0))
    {
      g_dbus_method_invocation_return_value (invocation,
                                             g_variant_new ("(v)", get_theme_value (arg_key)));
      return TRUE;
    }
  else if (g_hash_table_contains (settings, arg_namespace))
    {
      SettingsBundle *bundle = g_hash_table_lookup (settings, arg_namespace);
      if (g_settings_schema_has_key (bundle->schema, arg_key))
        {
          g_autoptr (GVariant) variant = NULL;

          variant = g_settings_get_value (bundle->settings, arg_key);
          g_dbus_method_invocation_return_value (invocation, g_variant_new ("(v)", variant));
          return TRUE;
        }
    }

  g_debug ("Attempted to read unknown namespace/key pair: %s %s", arg_namespace, arg_key);
  g_dbus_method_invocation_return_error_literal (invocation, XDG_DESKTOP_PORTAL_ERROR,
                                                 XDG_DESKTOP_PORTAL_ERROR_NOT_FOUND,
                                                 _("Requested setting not found"));

  return TRUE;
}

typedef struct {
  XdpImplSettings *self;
  const char *namespace;
} ChangedSignalUserData;

static ChangedSignalUserData *
changed_signal_user_data_new (XdpImplSettings *settings,
                              const char      *namespace)
{
  ChangedSignalUserData *data = g_new (ChangedSignalUserData, 1);
  data->self = settings;
  data->namespace = namespace;
  return data;
}

static void
changed_signal_user_data_destroy (gpointer  data,
                                  GClosure *closure)
{
  g_free (data);
}

static void
on_settings_changed (GSettings             *settings,
                     const char            *key,
                     ChangedSignalUserData *user_data)
{
  g_autoptr (GVariant) new_value = g_settings_get_value (settings, key);

  g_debug ("Emitting changed for %s %s", user_data->namespace, key);
  if (strcmp (user_data->namespace, "org.gnome.desktop.interface") == 0 &&
      strcmp (key, "enable-animations") == 0)
    sync_animations_enabled (user_data->self, shell_introspect_get ());
  else
    xdp_impl_settings_emit_setting_changed (user_data->self,
                                            user_data->namespace, key,
                                            g_variant_new ("v", new_value));

  if (strcmp (user_data->namespace, "org.gnome.desktop.interface") == 0 &&
      strcmp (key, "color-scheme") == 0)
    xdp_impl_settings_emit_setting_changed (user_data->self,
                                            "org.freedesktop.appearance", key,
                                            g_variant_new ("v", get_color_scheme ()));

  if (strcmp (user_data->namespace, "co.tauos.desktop.appearance") == 0 &&
      strcmp (key, "accent-color") == 0)
    xdp_impl_settings_emit_setting_changed (user_data->self,
                                            "org.freedesktop.appearance", key,
                                            g_variant_new ("v", get_accent_color ()));

  if (strcmp (user_data->namespace, "org.gnome.desktop.a11y.interface") == 0 &&
      strcmp (key, "high-contrast") == 0)
    {
      xdp_impl_settings_emit_setting_changed (user_data->self,
                                              "org.gnome.desktop.interface", "gtk-theme",
                                              g_variant_new ("v", get_theme_value ("gtk-theme")));
      xdp_impl_settings_emit_setting_changed (user_data->self,
                                              "org.gnome.desktop.interface", "icon-theme",
                                              g_variant_new ("v", get_theme_value ("icon-theme")));
    }
}

static void
init_settings_table (XdpImplSettings *settings,
                     GHashTable      *table)
{
  static const char * const schemas[] = {
    "org.gnome.desktop.interface",
    "org.gnome.settings-daemon.peripherals.mouse",
    "org.gnome.desktop.sound",
    "org.gnome.desktop.privacy",
    "org.gnome.desktop.wm.preferences",
    "org.gnome.settings-daemon.plugins.xsettings",
    "org.gnome.desktop.a11y",
    "org.gnome.desktop.a11y.interface",
    "org.gnome.desktop.input-sources",
    "co.tauos.desktop.appearance"
  };
  size_t i;
  GSettingsSchemaSource *source = g_settings_schema_source_get_default ();

  for (i = 0; i < G_N_ELEMENTS(schemas); ++i)
    {
      GSettings *setting;
      GSettingsSchema *schema;
      SettingsBundle *bundle;
      const char *schema_name = schemas[i];

      schema = g_settings_schema_source_lookup (source, schema_name, TRUE);
      if (!schema)
        {
          g_debug ("%s schema not found", schema_name);
          continue;
        }

      setting = g_settings_new (schema_name);
      bundle = settings_bundle_new (schema, setting);
      g_signal_connect_data (setting, "changed", G_CALLBACK(on_settings_changed),
                             changed_signal_user_data_new (settings, schema_name),
                             changed_signal_user_data_destroy, 0);
      g_hash_table_insert (table, (char*)schema_name, bundle);
    }
}

static void
fontconfig_changed (FcMonitor       *monitor,
                    XdpImplSettings *impl)
{
  const char *namespace = "org.gnome.fontconfig";
  const char *key = "serial";

  g_debug ("Emitting changed for %s %s", namespace, key);

  fontconfig_serial++;

  xdp_impl_settings_emit_setting_changed (impl,
                                          namespace, key,
                                          g_variant_new ("v", g_variant_new_int32 (fontconfig_serial)));
}

static void
set_enable_animations (XdpImplSettings *impl,
                       gboolean         new_enable_animations)
{
  const char *namespace = "org.gnome.desktop.interface";
  const char *key = "enable-animations";
  GVariant *enable_animations_variant;

  if (enable_animations == new_enable_animations)
    return;

  enable_animations = new_enable_animations;
  enable_animations_variant =
    g_variant_new ("v", g_variant_new_boolean (enable_animations));
  xdp_impl_settings_emit_setting_changed (impl,
                                          namespace,
                                          key,
                                          enable_animations_variant);
}

static void
sync_animations_enabled (XdpImplSettings *impl,
                         ShellIntrospect *shell_introspect)
{
  gboolean new_enable_animations;

  if (!shell_introspect_are_animations_enabled (shell_introspect,
                                                &new_enable_animations))
    {
      SettingsBundle *bundle = g_hash_table_lookup (settings, "org.gnome.desktop.interface");
      new_enable_animations = g_settings_get_boolean (bundle->settings, "enable-animations");
    }

  set_enable_animations (impl, new_enable_animations);
}

static void
animations_enabled_changed (ShellIntrospect *shell_introspect,
                            XdpImplSettings *impl)
{
  sync_animations_enabled (impl, shell_introspect);
}

gboolean
settings_init (GDBusConnection  *bus,
               GError          **error)
{
  GDBusInterfaceSkeleton *helper;
  ShellIntrospect *shell_introspect;

  helper = G_DBUS_INTERFACE_SKELETON (xdp_impl_settings_skeleton_new ());

  g_signal_connect (helper, "handle-read", G_CALLBACK (settings_handle_read), NULL);
  g_signal_connect (helper, "handle-read-all", G_CALLBACK (settings_handle_read_all), NULL);

  settings = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, (GDestroyNotify)settings_bundle_free);

  init_settings_table (XDP_IMPL_SETTINGS (helper), settings);

  fontconfig_monitor = fc_monitor_new ();
  g_signal_connect (fontconfig_monitor, "updated", G_CALLBACK (fontconfig_changed), helper);
  fc_monitor_start (fontconfig_monitor);

  shell_introspect = shell_introspect_get ();
  g_signal_connect (shell_introspect, "animations-enabled-changed",
                    G_CALLBACK (animations_enabled_changed),
                    helper);
  sync_animations_enabled (XDP_IMPL_SETTINGS (helper),
                           shell_introspect);

  if (!g_dbus_interface_skeleton_export (helper,
                                         bus,
                                         DESKTOP_PORTAL_OBJECT_PATH,
                                         error))
    return FALSE;

  g_debug ("providing %s", g_dbus_interface_skeleton_get_info (helper)->name);

  return TRUE;

}
