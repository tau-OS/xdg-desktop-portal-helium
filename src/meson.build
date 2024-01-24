gnome = import('gnome')

xdg_desktop_portal_dep = dependency('xdg-desktop-portal', version: '>= 1.17.0')

top_srcdir = meson.project_source_root()

# Desktop Portal D-Bus interfaces
desktop_portal_dbus_interfaces = [
  'org.freedesktop.impl.portal.Settings',
]

desktop_portal_interfaces_dir = xdg_desktop_portal_dep.get_variable(
  pkgconfig: 'interfaces_dir')

desktop_portal_dbus_interfaces_files = []
foreach intf: desktop_portal_dbus_interfaces
  desktop_portal_dbus_interfaces_files += [desktop_portal_interfaces_dir / '@0@.xml'.format(intf)]
endforeach

built_sources = gnome.gdbus_codegen(
  'xdg-desktop-portal-dbus',
  sources: desktop_portal_dbus_interfaces_files,
  interface_prefix: 'org.freedesktop.impl.portal.',
  namespace: 'XdpImpl',
)

shell_dbus_interfaces = files(
  top_srcdir / 'data' / 'org.gnome.Mutter.ServiceChannel.xml',
  top_srcdir / 'data' / 'org.gnome.Shell.Introspect.xml',
)

built_sources += gnome.gdbus_codegen(
  'shell-dbus',
  sources: shell_dbus_interfaces,
  interface_prefix: 'org.freedesktop.impl.portal.',
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
  dependency('glib-2.0'),
  dependency('gio-unix-2.0'),
  dependency('gtk4'),
  dependency('gsettings-desktop-schemas'),
  libadwaita_dep,
  xdg_desktop_portal_dep,
]

sources = built_sources + files(
  'externalwindow.c',
  'fc-monitor.c',
  'settings.c',
  'shellintrospect.c',
  'utils.c',
  'xdg-desktop-portal-tau.c',
)

common_flags = []

gtk_x11_dep = dependency('gtk4-x11', required: false)
if gtk_x11_dep.found()
  sources += files('externalwindow-x11.c')
  deps += [dependency('x11')]
  common_flags += ['-DHAVE_GTK_X11']
endif

gtk_wayland_dep = dependency('gtk4-wayland', required: false)
if gtk_wayland_dep.found()
  wayland_scanner = find_program('wayland-scanner')

  wayland_protocols_dir = top_srcdir / 'data'

  wayland_protocols = [
    'mutter-x11-interop',
  ]

  foreach protocol : wayland_protocols
    client_header = custom_target('@0@ client header'.format(protocol),
      input: wayland_protocols_dir / '@0@.xml'.format(protocol),
      output: '@0@-client-protocol.h'.format(protocol),
      command: [
        wayland_scanner,
        'client-header',
        '@INPUT@', '@OUTPUT@',
      ]
    )
    protocol_code = custom_target('@0@ source'.format(protocol),
      input: wayland_protocols_dir / '@0@.xml'.format(protocol),
      output: '@0@-protocol.c'.format(protocol),
      command: [
        wayland_scanner,
        'private-code',
        '@INPUT@', '@OUTPUT@',
      ]
    )

    sources += [client_header, protocol_code]
  endforeach

  sources += files('externalwindow-wayland.c')
  common_flags += ['-DHAVE_GTK_WAYLAND']
  deps += [
    dependency('wayland-client'),
  ]
endif

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
