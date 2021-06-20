project(
  'xdg-desktop-portal-gnome',
  'c',
  version: '41.alpha',
  meson_version: '>= 0.53.0',
  license: 'LGPLv2.1+'
)

prefix = get_option('prefix')
datadir = get_option('datadir')
libdir = get_option('libdir')
libexecdir = get_option('libexecdir')

systemduserunitdir = get_option('systemduserunitdir')
if systemduserunitdir == ''
  systemduserunitdir = libdir / 'systemd' / 'user'
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