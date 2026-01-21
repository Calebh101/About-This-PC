#!/bin/sh
# This is not to be run by itself.

HERE="$(dirname "$(readlink -f "${0}")")"
export LD_LIBRARY_PATH="$HERE/usr/lib:$LD_LIBRARY_PATH"
export QT_PLUGIN_PATH="$HERE/usr/plugins"
export QT_QPA_PLATFORM_PLUGIN_PATH="$HERE/usr/plugins/platforms"
export PATH="$HERE/usr/bin:$PATH"
exec "$HERE/usr/bin/AboutThisPC" "$@"