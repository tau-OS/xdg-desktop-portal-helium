%global tarball_version %%(echo %{version} | tr '~' '.')

# Required for xdp_impl_background_emit_running_applications_changed
%global xdg_desktop_portal_version 1.5.4

Name:           xdg-desktop-portal-tau
Version:        1.1
Release:        10
Summary:        Backend implementation for xdg-desktop-portal using GNOME (tauOS additions)

License:        LGPLv2+
URL:            https://tauos.co
Source0:        https://github.com/tau-OS/xdg-desktop-portal-tau/archive/refs/heads/main.zip

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

Provides:       xdg-desktop-portal-gnome = 43
Provides:       xdg-desktop-portal-gnome%{?_isa} = 43
Conflicts:       xdg-desktop-portal-gnome = 43
Conflicts:       xdg-desktop-portal-gnome%{?_isa} = 43

%description
A backend implementation for xdg-desktop-portal that is using various pieces of
GNOME infrastructure, such as the org.gnome.Shell.Screenshot or
org.gnome.SessionManager D-Bus interfaces. (with tauOS additions.)


%prep
%setup -q -n xdg-desktop-portal-tau-main


%build
%meson -Dsystemduserunitdir=%{_userunitdir}
%meson_build

%install
%meson_install
desktop-file-validate %{buildroot}/%{_datadir}/applications/xdg-desktop-portal-gnome.desktop
%find_lang xdg-desktop-portal-gnome


%post
%systemd_user_post xdg-desktop-portal-gnome.service

%preun
%systemd_user_preun xdg-desktop-portal-gnome.service


%files -f xdg-desktop-portal-gnome.lang
%license COPYING
%doc NEWS README.md
%{_libexecdir}/xdg-desktop-portal-gnome
%{_datadir}/applications/xdg-desktop-portal-gnome.desktop
%{_datadir}/glib-2.0/schemas/xdg-desktop-portal-gnome.gschema.xml
%{_datadir}/dbus-1/services/org.freedesktop.impl.portal.desktop.gnome.service
%{_datadir}/xdg-desktop-portal/portals/gnome.portal
%{_userunitdir}/xdg-desktop-portal-gnome.service

%changelog
%autochangelog
