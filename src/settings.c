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

#include "xdg-desktop-portal-dbus.h"

static GHashTable *settings;

typedef struct {
  GSettingsSchema *schema;
  GSettings *settings;
} SettingsBundle;

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

static GVariant * get_color_scheme (void)
{
  SettingsBundle *bundle = g_hash_table_lookup (settings, "org.gnome.desktop.interface");
  int color_scheme;

  if (!g_settings_schema_has_key (bundle->schema, "color-scheme"))
    return g_variant_new_uint32 (0); /* No preference */

  color_scheme = g_settings_get_enum (bundle->settings, "color-scheme");

  return g_variant_new_uint32 (color_scheme);
}
static GVariant * get_contrast (void)
{
  SettingsBundle *bundle = g_hash_table_lookup (settings, "com.fyralabs.desktop.appearance");
  double hc_option;

  if (!g_settings_schema_has_key (bundle->schema, "contrast"))
    return g_variant_new_double (0.0); /* No preference */

  hc_option = g_settings_get_double (bundle->settings, "contrast");

  return g_variant_new_double  (hc_option);
}

static GVariant * get_ensor_scheme (void)
{
  SettingsBundle *bundle = g_hash_table_lookup (settings, "com.fyralabs.desktop.appearance");
  int ensor_scheme;

  if (!g_settings_schema_has_key (bundle->schema, "ensor-scheme"))
    return g_variant_new_uint32 (0); /* No preference */

  ensor_scheme = g_settings_get_enum (bundle->settings, "ensor-scheme");

  return g_variant_new_uint32 (ensor_scheme);
}

static GVariant * get_font_weight (void)
{
  SettingsBundle *bundle = g_hash_table_lookup (settings, "com.fyralabs.desktop.appearance");
  double font_weight;

  if (!g_settings_schema_has_key (bundle->schema, "font-weight"))
    return g_variant_new_double (1.0); /* No preference */

  font_weight = g_settings_get_double (bundle->settings, "font-weight");

  return g_variant_new_double (font_weight);
}
static GVariant * get_ui_roundness (void)
{
  SettingsBundle *bundle = g_hash_table_lookup (settings, "com.fyralabs.desktop.appearance");
  double ui_roundness;

  if (!g_settings_schema_has_key (bundle->schema, "roundness"))
    return g_variant_new_double (1.0); /* No preference */

  ui_roundness = g_settings_get_double (bundle->settings, "roundness");

  return g_variant_new_double (ui_roundness);
}
static GVariant * get_density (void)
{
  SettingsBundle *bundle = g_hash_table_lookup (settings, "com.fyralabs.desktop.appearance");
  guint density;

  if (!g_settings_schema_has_key (bundle->schema, "density"))
    return g_variant_new_uint32 (0); /* Default */

  density = g_settings_get_uint (bundle->settings, "density");

  if (density > 2)
    density = 0;

  return g_variant_new_uint32 (density);
}
static GVariant * get_accent_color (void)
{
  SettingsBundle * bundle = g_hash_table_lookup (settings, "com.fyralabs.desktop.appearance");

  GVariant * no_preference[] = {
    g_variant_new_double (69.0),
    g_variant_new_double (420.0),
    g_variant_new_double (1337.0)
  };

  if (!g_settings_schema_has_key (bundle->schema, "accent-color"))
    return g_variant_new_tuple (no_preference, 3); /* No preference */

  char* color = g_settings_get_string (bundle->settings, "accent-color");

  if (strcmp (color, "purple") == 0) {
    GVariant * purple[] = { /* #8b55be */
      g_variant_new_double (0.5490),
      g_variant_new_double (0.3372),
      g_variant_new_double (0.7490)
    };
    return g_variant_new_tuple (purple, 3);
  } else if (strcmp (color, "pink") == 0) {
    GVariant * pink[] = { /* #be55a7 */
      g_variant_new_double (0.7490),
      g_variant_new_double (0.3372),
      g_variant_new_double (0.6588)
    };
    return g_variant_new_tuple (pink, 3);
  } else if (strcmp (color, "red") == 0) {
    GVariant * red[] = { /* #da275f */
      g_variant_new_double (0.8588),
      g_variant_new_double (0.1568),
      g_variant_new_double (0.3764)
    };
    return g_variant_new_tuple (red, 3);
  } else if (strcmp (color, "orange") == 0) {
    GVariant * orange[] = { /* #f6802a */
      g_variant_new_double (0.9686),
      g_variant_new_double (0.5058),
      g_variant_new_double (0.1680)
    };
    return g_variant_new_tuple (orange, 3);
  } else if (strcmp (color, "yellow") == 0) {
    GVariant * yellow[] = { /* #dfa000 */
      g_variant_new_double (0.8784),
      g_variant_new_double (0.6313),
      g_variant_new_double (0.0039)
    };
    return g_variant_new_tuple (yellow, 3);
  } else if (strcmp (color, "green") == 0) {
    GVariant * green[] = { /* #48cf5d */
      g_variant_new_double (0.2862),
      g_variant_new_double (0.8156),
      g_variant_new_double (0.3686)
    };
    return g_variant_new_tuple (green, 3);
  } else if (strcmp (color, "mint") == 0) {
    GVariant * mint[] = { /* #55bea5 */
      g_variant_new_double (0.3372),
      g_variant_new_double (0.7490),
      g_variant_new_double (0.6509)
    };
    return g_variant_new_tuple (mint, 3);
  } else if (strcmp (color, "blue") == 0) {
    GVariant * blue[] = { /* #44b9fb */
      g_variant_new_double (0.2705),
      g_variant_new_double (0.7294),
      g_variant_new_double (0.9882)
    };
    return g_variant_new_tuple (blue, 3);
  } else if (strcmp (color, "brown") == 0) {
    GVariant * brown[] = { /* #be8755 */
      g_variant_new_double (0.7490),
      g_variant_new_double (0.5330),
      g_variant_new_double (0.3370)
    };
    return g_variant_new_tuple (brown, 3);
  } else if (strcmp (color, "mono") == 0) {
    GVariant * mono[] = { /* #000000 */
      g_variant_new_double (0.0),
      g_variant_new_double (0.0),
      g_variant_new_double (0.0)
    };
    return g_variant_new_tuple (mono, 3);
  } else if (strcmp (color, "multi") == 0) {
    return g_variant_new_tuple (no_preference, 3);
  }

  GdkRGBA rgba;
  if (gdk_rgba_parse (&rgba, color)) {
    GVariant * custom[] = {
      g_variant_new_double (rgba.red),
      g_variant_new_double (rgba.green),
      g_variant_new_double (rgba.blue)
    };
    return g_variant_new_tuple (custom, 3);
  }

  return g_variant_new_tuple (no_preference, 3);
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
          g_variant_dict_insert_value (&dict, keys[i], g_settings_get_value (value->settings, keys[i]));
        }

      g_variant_builder_add (builder, "{s@a{sv}}", key, g_variant_dict_end (&dict));
    }

  if (namespace_matches ("org.freedesktop.appearance", arg_namespaces))
    {
      GVariantDict dict;

      g_variant_dict_init (&dict, NULL);
      g_variant_dict_insert_value (&dict, "color-scheme", get_color_scheme ());
      g_variant_dict_insert_value (&dict, "accent-color", get_accent_color ());
      g_variant_dict_insert_value (&dict, "ensor-scheme", get_ensor_scheme ());
      g_variant_dict_insert_value (&dict, "font-weight", get_font_weight ());
      g_variant_dict_insert_value (&dict, "roundness", get_ui_roundness ());
      g_variant_dict_insert_value (&dict, "contrast", get_contrast ());
      g_variant_dict_insert_value (&dict, "density", get_density ());

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

  if (strcmp (arg_namespace, "org.freedesktop.appearance") == 0 &&
           strcmp (arg_key, "color-scheme") == 0)
    {
      g_dbus_method_invocation_return_value (invocation,
                                             g_variant_new ("(v)", get_color_scheme ()));
      return TRUE;
    }
  else if (strcmp (arg_namespace, "org.freedesktop.appearance") == 0 &&
           strcmp (arg_key, "ensor-scheme") == 0)
    {
      g_dbus_method_invocation_return_value (invocation,
                                             g_variant_new ("(v)", get_ensor_scheme ()));
      return TRUE;
    }
  else if (strcmp (arg_namespace, "org.freedesktop.appearance") == 0 &&
           strcmp (arg_key, "font-weight") == 0)
    {
      g_dbus_method_invocation_return_value (invocation,
                                             g_variant_new ("(v)", get_font_weight ()));
      return TRUE;
    }
  else if (strcmp (arg_namespace, "org.freedesktop.appearance") == 0 &&
           strcmp (arg_key, "roundness") == 0)
    {
      g_dbus_method_invocation_return_value (invocation,
                                             g_variant_new ("(v)", get_ui_roundness ()));
      return TRUE;
    }
  else if (strcmp (arg_namespace, "org.freedesktop.appearance") == 0 &&
           strcmp (arg_key, "accent-color") == 0)
    {
      g_dbus_method_invocation_return_value (invocation,
                                             g_variant_new ("(v)", get_accent_color ()));
      return TRUE;
    }
  else if (strcmp (arg_namespace, "org.freedesktop.appearance") == 0 &&
           strcmp (arg_key, "contrast") == 0)
    {
      g_dbus_method_invocation_return_value (invocation,
                                             g_variant_new ("(v)", get_contrast ()));
      return TRUE;
    }
  else if (strcmp (arg_namespace, "org.freedesktop.appearance") == 0 &&
           strcmp (arg_key, "density") == 0)
    {
      g_dbus_method_invocation_return_value (invocation,
                                             g_variant_new ("(v)", get_density ()));
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
      strcmp (key, "color-scheme") == 0)
    xdp_impl_settings_emit_setting_changed (user_data->self,
                                            "org.freedesktop.appearance", key,
                                            g_variant_new ("v", get_color_scheme ()));

  if (strcmp (user_data->namespace, "com.fyralabs.desktop.appearance") == 0 &&
      strcmp (key, "ensor-scheme") == 0)
    xdp_impl_settings_emit_setting_changed (user_data->self,
                                            "org.freedesktop.appearance", key,
                                            g_variant_new ("v", get_ensor_scheme ()));

  if (strcmp (user_data->namespace, "com.fyralabs.desktop.appearance") == 0 &&
      strcmp (key, "font-weight") == 0)
    xdp_impl_settings_emit_setting_changed (user_data->self,
                                            "org.freedesktop.appearance", key,
                                            g_variant_new ("v", get_font_weight ()));

  if (strcmp (user_data->namespace, "com.fyralabs.desktop.appearance") == 0 &&
      strcmp (key, "roundness") == 0)
    xdp_impl_settings_emit_setting_changed (user_data->self,
                                            "org.freedesktop.appearance", key,
                                            g_variant_new ("v", get_ui_roundness ()));

  if (strcmp (user_data->namespace, "com.fyralabs.desktop.appearance") == 0 &&
      strcmp (key, "accent-color") == 0)
    xdp_impl_settings_emit_setting_changed (user_data->self,
                                            "org.freedesktop.appearance", key,
                                            g_variant_new ("v", get_accent_color ()));

  if (strcmp (user_data->namespace, "com.fyralabs.desktop.appearance") == 0 &&
      strcmp (key, "contrast") == 0)
    xdp_impl_settings_emit_setting_changed (user_data->self,
                                            "org.freedesktop.appearance", key,
                                            g_variant_new ("v", get_contrast ()));

  if (strcmp (user_data->namespace, "com.fyralabs.desktop.appearance") == 0 &&
      strcmp (key, "density") == 0)
    xdp_impl_settings_emit_setting_changed (user_data->self,
                                            "org.freedesktop.appearance", key,
                                            g_variant_new ("v", get_density ()));
}

static void
init_settings_table (XdpImplSettings *settings,
                     GHashTable      *table)
{
  static const char * const schemas[] = {
    "org.gnome.desktop.interface",
    "com.fyralabs.desktop.appearance"
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

gboolean
settings_init (GDBusConnection  *bus,
               GError          **error)
{
  GDBusInterfaceSkeleton *helper;

  helper = G_DBUS_INTERFACE_SKELETON (xdp_impl_settings_skeleton_new ());

  g_signal_connect (helper, "handle-read", G_CALLBACK (settings_handle_read), NULL);
  g_signal_connect (helper, "handle-read-all", G_CALLBACK (settings_handle_read_all), NULL);

  settings = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, (GDestroyNotify)settings_bundle_free);

  init_settings_table (XDP_IMPL_SETTINGS (helper), settings);

  if (!g_dbus_interface_skeleton_export (helper,
                                         bus,
                                         DESKTOP_PORTAL_OBJECT_PATH,
                                         error))
    return FALSE;

  g_debug ("providing %s", g_dbus_interface_skeleton_get_info (helper)->name);

  return TRUE;
}
