project(
  'xdg-desktop-portal-helium',
  'c',
  version: '45.0',
  meson_version: '>= 0.53.0',
  license: 'LGPLv2.1+'
)

prefix = get_option('prefix')
datadir = get_option('datadir')
libdir = get_option('libdir')
libexecdir = get_option('libexecdir')

systemduserunitdir = get_option('systemduserunitdir')
if systemduserunitdir == ''
  systemd = dependency('systemd', version: '>= 242')
  systemduserunitdir = systemd.get_pkgconfig_variable(
    'systemduserunitdir',
    define_variable: ['prefix', get_option('prefix')]
  )
endif

dbus_service_dir = get_option('dbus_service_dir')
if dbus_service_dir == ''
  dbus_service_dir = datadir / 'dbus-1' / 'services'
endif

subdir('data')
subdir('po')
subdir('src')

summary({
  'prefix': prefix,
  'datadir': datadir,
  'libdir': libdir,
  'libexecdir': libexecdir,
  'dbus_service_dir': dbus_service_dir,
  'systemduserunitdir': systemduserunitdir,
})
