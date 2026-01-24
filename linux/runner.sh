#!/usr/bin/env bash
# This is not to be run by itself.

set -e
this_dir="$(readlink -f "$(dirname "$0")")"
DE="${XDG_CURRENT_DESKTOP:-}"

export XDG_DATA_DIRS="$this_dir/usr/share:$XDG_DATA_DIRS"
export XDG_CONFIG_DIRS="$this_dir/usr/etc/xdg:$XDG_CONFIG_DIRS"

export QT_AUTO_SCREEN_SCALE_FACTOR=1
export QT_ENABLE_HIGHDPI_SCALING=1
export QT_STYLE_OVERRIDE=
export QT_QPA_PLATFORMTHEME=xdgdesktopportal

exec "$this_dir"/usr/bin/AboutThisPC "$@"