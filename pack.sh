#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
cd "$root"
hook=build/rgdsplus/libfundrastic.so
test -f "$hook" || { printf '%s\n' 'Build with make rgdsplus first.' >&2; exit 1; }
mkdir -p dist
stage=$(mktemp -d "$root/dist/.package.XXXXXX")
trap 'rm -rf "$stage"' EXIT
out="$stage/drastic-ui-plus"
mkdir -p "$out/lib" "$out/language"
cp "$hook" "$out/lib/"
cp LICENSE CREDITS.md README.md "$out/"
cp shared/language/*.txt "$out/language/"
cp targets/rgdsplus/launch.sh targets/rgdsplus/launch-rom.sh "$out/"
cp targets/rgdsplus/es_features_fun_drastic.cfg "$out/es_features_drastic_ui_plus.cfg"
cp targets/rgdsplus/es_systems_fun_drastic.cfg "$out/es_systems_drastic_ui_plus.cfg"
chmod 755 "$out/launch.sh" "$out/launch-rom.sh"
COPYFILE_DISABLE=1 tar -czf "$stage/drastic-ui-plus-rgdsplus.tar.gz" -C "$stage" drastic-ui-plus
mv "$stage/drastic-ui-plus-rgdsplus.tar.gz" dist/
printf '%s\n' 'Built dist/drastic-ui-plus-rgdsplus.tar.gz'
