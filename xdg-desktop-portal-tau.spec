%global tarball_version %%(echo %{version} | tr '~' '.')

# Required for xdp_impl_background_emit_running_applications_changed
%global xdg_desktop_portal_version 1.5.4

Name:           xdg-desktop-portal-tau
Version:        1.1
Release:        15
Summary:        Backend implementation for xdg-desktop-portal for gnomeOS.

License:        LGPLv2+
URL:            https://gnomeos.co
Source0:        https://github.com/gnome-OS/xdg-desktop-portal-tau/archive/refs/heads/main.zip

BuildRequires:  desktop-file-utils
BuildRequires:  gcc
BuildRequires:  gettext
BuildRequires:  meson
BuildRequires:  pkgconfig(fontconfig)
BuildRequires:  pkgconfig(gnome-bg-4)
BuildRequires:  pkgconfig(gnome-desktop-4)
BuildRequires:  pkgconfig(gsettings-desktop-schemas)
BuildRequires:  pkgconfig(gtk4)
BuildRequires:  pkgconfig(libadwaita-1)
BuildRequires:  pkgconfig(xdg-desktop-portal) >= %{xdg_desktop_portal_version}
BuildRequires:  systemd-rpm-macros

Requires:       dbus
Requires:       dbus-common
Requires:       xdg-desktop-portal >= %{xdg_desktop_portal_version}

Supplements:    gnome-shell

Provides:       xdg-desktop-portal-tau = 43
Provides:       xdg-desktop-portal-tau%{?_isa} = 43
Conflicts:       xdg-desktop-portal-tau = 43
Conflicts:       xdg-desktop-portal-tau%{?_isa} = 43

%description
A backend implementation for xdg-desktop-portal for gnomeOS.


%prep
%setup -q -n xdg-desktop-portal-tau-main


%build
%meson -Dsystemduserunitdir=%{_userunitdir}
%meson_build

%install
%meson_install
desktop-file-validate %{buildroot}/%{_datadir}/applications/xdg-desktop-portal-tau.desktop
%find_lang xdg-desktop-portal-tau


%post
%systemd_user_post xdg-desktop-portal-tau.service

%preun
%systemd_user_preun xdg-desktop-portal-tau.service


%files -f xdg-desktop-portal-tau.lang
%license COPYING
%doc NEWS README.md
%{_libexecdir}/xdg-desktop-portal-tau
%{_datadir}/applications/xdg-desktop-portal-tau.desktop
%{_datadir}/glib-2.0/schemas/xdg-desktop-portal-tau.gschema.xml
%{_datadir}/dbus-1/services/org.freedesktop.impl.portal.desktop.gnome.service
%{_datadir}/xdg-desktop-portal/portals/gnome.portal
%{_userunitdir}/xdg-desktop-portal-tau.service

%changelog
%autochangelog
