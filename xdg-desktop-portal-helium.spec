%global tarball_version %%(echo %{version} | tr '~' '.')

# Required for xdp_impl_background_emit_running_applications_changed
%global xdg_desktop_portal_version 1.5.4

Name:           xdg-desktop-portal-helium
Version:        1.1
Release:        15
Summary:        Backend implementation for xdg-desktop-portal for Helium apps.

License:        LGPLv2+
URL:            https://gnomeos.co
Source0:        https://github.com/gnome-OS/xdg-desktop-portal-helium/archive/refs/heads/main.zip

BuildRequires:  desktop-file-utils
BuildRequires:  gcc
BuildRequires:  gettext
BuildRequires:  meson
BuildRequires:  pkgconfig(fontconfig)
BuildRequires:  pkgconfig(gnome-bg-4)
BuildRequires:  pkgconfig(gnome-desktop-4)
BuildRequires:  pkgconfig(gsettings-desktop-schemas)
BuildRequires:  pkgconfig(gtk4)
BuildRequires:  pkgconfig(libhelium-1)
BuildRequires:  pkgconfig(xdg-desktop-portal) >= %{xdg_desktop_portal_version}
BuildRequires:  systemd-rpm-macros

Requires:       dbus
Requires:       dbus-common
Requires:       xdg-desktop-portal >= %{xdg_desktop_portal_version}

%description
A backend implementation for xdg-desktop-portal for Helium apps.


%prep
%setup -q -n xdg-desktop-portal-helium-main


%build
%meson -Dsystemduserunitdir=%{_userunitdir}
%meson_build

%install
%meson_install
desktop-file-validate %{buildroot}/%{_datadir}/applications/xdg-desktop-portal-helium.desktop
%find_lang xdg-desktop-portal-helium


%post
%systemd_user_post xdg-desktop-portal-helium.service

%preun
%systemd_user_preun xdg-desktop-portal-helium.service


%files -f xdg-desktop-portal-helium.lang
%license COPYING
%doc README.md
%{_libexecdir}/xdg-desktop-portal-helium
%{_datadir}/applications/xdg-desktop-portal-helium.desktop
%{_datadir}/glib-2.0/schemas/xdg-desktop-portal-helium.gschema.xml
%{_datadir}/dbus-1/services/org.freedesktop.impl.portal.desktop.gnome.service
%{_datadir}/xdg-desktop-portal/portals/gnome.portal
%{_userunitdir}/xdg-desktop-portal-helium.service

%changelog
%autochangelog
