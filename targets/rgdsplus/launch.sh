#!/bin/sh
# Run Fun Drastic with private emulator data.
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
rom=$(realpath "${1:?Pass a Nintendo DS ROM}")
case "$rom" in
    "$root"/roms/*.[nN][dD][sS]|/storage/roms/nds/*.[nN][dD][sS]) test -f "$rom" ;;
    *) printf 'Fun Drastic: unsupported ROM path: %s\n' "$rom" >&2; exit 64 ;;
esac

system_drastic=/usr/config/drastic
binary="$system_drastic/drastic"
expected=d54980a36c5e0b5cc46868bf25acd5d33b4c5337334fd0c417255a36942feb95
if [ ! -x "$binary" ] || ! printf '%s  %s\n' "$expected" "$binary" | sha256sum -c - >/dev/null; then
    printf '%s\n' 'DraStic UI Plus: the installed ROCKNIX DraStic build is unsupported.' >&2
    exit 69
fi

state="$root/state"
mkdir -p "$state"/backup "$state"/savestates "$state"/Saves "$state"/cheats \
    "$state"/profiles "$state"/unzip_cache "$state"/input_record "$state"/slot2 "$state"/scripts
# Keep a writable config and reference the data already supplied by ROCKNIX.
mkdir -p "$state/config" "$state/system"
if [ ! -f "$state/config/drastic.cfg" ]; then
    cp "$system_drastic/config/drastic.cfg" "$state/config/drastic.cfg"
fi
for entry in drastic_logo_0.raw drastic_logo_1.raw game_database.xml microphone \
    system/drastic_bios_arm7.bin system/drastic_bios_arm9.bin; do
    if [ -e "$system_drastic/$entry" ] && [ ! -e "$state/$entry" ] && [ ! -L "$state/$entry" ]; then
        ln -s "$system_drastic/$entry" "$state/$entry"
    fi
done
if [ -d "$root/language" ] && [ ! -e "$state/language" ] && [ ! -L "$state/language" ]; then
    ln -s "$root/language" "$state/language"
fi
if [ -f "$system_drastic/usrcheat.dat" ] && [ ! -e "$state/config/usrcheat.dat" ] && \
    [ ! -L "$state/config/usrcheat.dat" ]; then
    ln -s "$system_drastic/usrcheat.dat" "$state/config/usrcheat.dat"
fi

if [ -n "${FUN_DRASTIC_SAVE_DIR:-}" ]; then
    test -d "$FUN_DRASTIC_SAVE_DIR"
    name=${rom##*/}
    legacy="$FUN_DRASTIC_SAVE_DIR/${name%.*}.sav"
    native="$FUN_DRASTIC_SAVE_DIR/${name%.*}.dsv"
    # DraStic reads raw saves at its .dsv path. Seed that file once from
    # an earlier Fun Drastic save, leaving the original and any .dsv intact.
    if [ -f "$legacy" ] && [ -s "$legacy" ] && [ ! -e "$native" ] && [ ! -L "$native" ]; then
        (
            tmp=$(mktemp "$FUN_DRASTIC_SAVE_DIR/.fundrastic-save.XXXXXX")
            trap 'rm -f -- "$tmp"' EXIT
            cp "$legacy" "$tmp"
            # Publish the complete copy without overwriting a concurrent save.
            ln -T -- "$tmp" "$native" || test -f "$native"
        )
    fi
fi
if [ -n "${FUN_DRASTIC_STATE_DIR:-}" ]; then
    test -d "$FUN_DRASTIC_STATE_DIR"
fi
cd "$state"
export FUN_DRASTIC_DIR="$state"
export SDCARD_PATH="$state"
export SDL_VIDEO_WAYLAND_WMCLASS=drastic
export SDL_TOUCH_MOUSE_EVENTS=0
exec env LD_PRELOAD="$root/lib/libfundrastic.so" "$binary" "$rom" > "$root/session.log" 2>&1
