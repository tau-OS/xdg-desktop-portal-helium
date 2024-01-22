gnome = import('gnome')
xdg_desktop_portal_dep = dependency('xdg-desktop-portal', version: '>= 1.15.0')

# Desktop Portal D-Bus interfaces
desktop_portal_interfaces_dir = xdg_desktop_portal_dep.get_pkgconfig_variable('interfaces_dir')
desktop_portal_dbus_interfaces = [
  desktop_portal_interfaces_dir / 'org.freedesktop.impl.portal.Settings.xml',
]

built_sources = gnome.gdbus_codegen(
  'xdg-desktop-portal-dbus',
  sources: desktop_portal_dbus_interfaces,
  interface_prefix: 'org.freedesktop.impl.portal.',
  namespace: 'XdpImpl',
)

# GResources
built_sources += gnome.compile_resources(
  'xdg-desktop-portal-tau-resources',
  'xdg-desktop-portal-tau.gresource.xml',
  c_name: '_xdg_desktop',
)

# config.h
config_h = configuration_data()
config_h.set_quoted('GETTEXT_PACKAGE', meson.project_name())
config_h.set_quoted('LOCALEDIR', prefix / get_option('localedir'))
config_h.set_quoted('PACKAGE_NAME', meson.project_name())
config_h.set_quoted('PACKAGE_STRING', '@0@ @1@'.format(meson.project_name(), meson.project_version()))
built_sources += configure_file(output: 'config.h', configuration: config_h)

# Sources
libadwaita_dep = dependency(
  'libadwaita-1',
  fallback: ['libadwaita', 'libadwaita_dep'],
  default_options: ['examples=false', 'introspection=disabled', 'tests=false', 'vapi=false'],
)
deps = [
  declare_dependency(sources: built_sources),
  meson.get_compiler('c').find_library('m'),
  dependency('fontconfig'),
  dependency('glib-2.0', version: '>= 2.62'),
  dependency('gio-unix-2.0'),
  dependency('gtk4', version: '>= 4.0'),
  dependency('gsettings-desktop-schemas'),
  libadwaita_dep,
  xdg_desktop_portal_dep,
]

sources = built_sources + files(
  'settings.c',
  'utils.c',
  'xdg-desktop-portal-tau.c',
)

common_flags = []

add_project_arguments(common_flags, language: 'c')

executable(
  'xdg-desktop-portal-tau',
  sources,
  dependencies: deps,
  install: true,
  install_dir: libexecdir,
)

install_data (
  'xdg-desktop-portal-tau.gschema.xml',
  install_dir : datadir / 'glib-2.0' / 'schemas',
)

gnome.post_install(glib_compile_schemas: true)
