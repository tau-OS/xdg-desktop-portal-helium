i18n = import('i18n')

libexecdir_conf = configuration_data()
libexecdir_conf.set('libexecdir', prefix / libexecdir)

# Portal
tau_portal_interfaces = [
  'org.freedesktop.impl.portal.Settings',
]
tau_portal_conf = configuration_data()
tau_portal_conf.set('PORTALS', ';'.join(tau_portal_interfaces))

portal_dir = datadir / 'xdg-desktop-portal' / 'portals'
configure_file(
  input: 'tau.portal.in',
  output: 'tau.portal',
  configuration: tau_portal_conf,
  install_dir: portal_dir,
)

# Service
configure_file(
  input: 'org.freedesktop.impl.portal.desktop.tau.service.in',
  output: 'org.freedesktop.impl.portal.desktop.tau.service',
  configuration: libexecdir_conf,
  install_dir: dbus_service_dir,
)

# systemd unit
configure_file(
  input: 'xdg-desktop-portal-tau.service.in',
  output: 'xdg-desktop-portal-tau.service',
  configuration: libexecdir_conf,
  install_dir: systemduserunitdir,
)

# Desktop file
desktop_in = configure_file(
  input: 'xdg-desktop-portal-tau.desktop.in.in',
  output: 'xdg-desktop-portal-tau.desktop.in',
  configuration: libexecdir_conf,
)

i18n.merge_file(
  type: 'desktop',
  input: desktop_in,
  output: 'xdg-desktop-portal-tau.desktop',
  po_dir: meson.source_root() / 'po',
  install: true,
  install_dir: datadir / 'applications',
)
