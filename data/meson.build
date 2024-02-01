i18n = import('i18n')

libexecdir_conf = configuration_data()
libexecdir_conf.set('libexecdir', prefix / libexecdir)

# Portal
portal_dir = datadir / 'xdg-desktop-portal' / 'portals'
install_data('helium.portal', install_dir: portal_dir)

# Service
configure_file(
  input: 'org.freedesktop.impl.portal.desktop.helium.service.in',
  output: 'org.freedesktop.impl.portal.desktop.helium.service',
  configuration: libexecdir_conf,
  install_dir: dbus_service_dir,
)

# systemd unit
configure_file(
  input: 'xdg-desktop-portal-helium.service.in',
  output: 'xdg-desktop-portal-helium.service',
  configuration: libexecdir_conf,
  install_dir: systemduserunitdir,
)

# Desktop file
desktop_in = configure_file(
  input: 'xdg-desktop-portal-helium.desktop.in.in',
  output: 'xdg-desktop-portal-helium.desktop.in',
  configuration: libexecdir_conf,
)

i18n.merge_file(
  type: 'desktop',
  input: desktop_in,
  output: 'xdg-desktop-portal-helium.desktop',
  po_dir: meson.source_root() / 'po',
  install: true,
  install_dir: datadir / 'applications',
)
