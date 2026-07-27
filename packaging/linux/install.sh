#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later

set -eu

daymark_script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
daymark_package_root="${daymark_script_dir}"
daymark_user_home="${DAYMARK_INSTALL_HOME:-${HOME:?HOME must be set}}"
if [ -n "${DAYMARK_DATA_HOME:-}" ]; then
    daymark_data_home="${DAYMARK_DATA_HOME}"
elif [ -n "${DAYMARK_INSTALL_HOME:-}" ]; then
    daymark_data_home="${daymark_user_home}/.local/share"
else
    daymark_data_home="${XDG_DATA_HOME:-${daymark_user_home}/.local/share}"
fi
daymark_install_root="${daymark_user_home}/.local/opt/daymark"
daymark_bin_dir="${daymark_user_home}/.local/bin"
daymark_launcher="${daymark_bin_dir}/daymark"
daymark_applications_dir="${daymark_data_home}/applications"
daymark_icon_dir="${daymark_data_home}/icons/hicolor/scalable/apps"
daymark_desktop_file="${daymark_applications_dir}/org.daymark.dashboard.desktop"

if [ ! -x "${daymark_package_root}/bin/daymark" ]; then
    echo "Daymark's executable was not found next to this installer." >&2
    echo "Extract the complete release archive before running install.sh." >&2
    exit 1
fi

mkdir -p \
    "${daymark_install_root}" \
    "${daymark_bin_dir}" \
    "${daymark_applications_dir}" \
    "${daymark_icon_dir}"

if [ "${daymark_package_root}" != "${daymark_install_root}" ]; then
    # Replace individual files instead of truncating them in place. Linux
    # refuses to truncate a currently executing binary (ETXTBSY), while an
    # atomic unlink-and-copy safely upgrades it and lets the old process finish.
    cp -a --remove-destination "${daymark_package_root}/." "${daymark_install_root}/"
fi

# Daymark 0.1 initially shipped a partial Qt plugin bundle. Remove its redirect
# when upgrading so Fedora's complete Wayland/GLX/EGL plugin set is discoverable.
if [ ! -f "${daymark_package_root}/bin/qt.conf" ]; then
    rm -f "${daymark_install_root}/bin/qt.conf"
fi

if [ -e "${daymark_launcher}" ] && [ ! -L "${daymark_launcher}" ]; then
    echo "Refusing to replace the existing file at ${daymark_launcher}." >&2
    exit 1
fi
ln -sfn "${daymark_install_root}/bin/daymark" "${daymark_launcher}"

install -m 0644 \
    "${daymark_install_root}/share/icons/hicolor/scalable/apps/org.daymark.dashboard.svg" \
    "${daymark_icon_dir}/org.daymark.dashboard.svg"

sed "s|^Exec=daymark$|Exec=${daymark_launcher}|" \
    "${daymark_install_root}/share/applications/org.daymark.dashboard.desktop" \
    > "${daymark_desktop_file}"
chmod 0644 "${daymark_desktop_file}"

if [ -n "${DAYMARK_INSTALL_HOME:-}" ]; then
    daymark_desktop_dir="${daymark_user_home}/Desktop"
elif command -v xdg-user-dir >/dev/null 2>&1; then
    daymark_desktop_dir=$(xdg-user-dir DESKTOP)
else
    daymark_desktop_dir="${daymark_user_home}/Desktop"
fi

if [ -z "${daymark_desktop_dir}" ]; then
    daymark_desktop_dir="${daymark_user_home}/Desktop"
fi

mkdir -p "${daymark_desktop_dir}"
install -m 0755 \
    "${daymark_desktop_file}" \
    "${daymark_desktop_dir}/Daymark.desktop"

if command -v update-desktop-database >/dev/null 2>&1; then
    update-desktop-database "${daymark_applications_dir}" >/dev/null 2>&1 || true
fi

if [ -z "${DAYMARK_INSTALL_HOME:-}" ] && command -v gio >/dev/null 2>&1; then
    gio set "${daymark_desktop_dir}/Daymark.desktop" metadata::trusted true \
        >/dev/null 2>&1 || true
fi

echo "Daymark installed successfully."
echo "Application: ${daymark_install_root}"
echo "Command:     ${daymark_launcher}"
echo "Menu entry:  ${daymark_desktop_file}"
echo "Shortcut:    ${daymark_desktop_dir}/Daymark.desktop"
