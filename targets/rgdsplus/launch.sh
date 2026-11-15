#!/bin/sh
# Run DraStic UI Plus with private emulator data.
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
rom=$(realpath "${1:?Pass a Nintendo DS ROM}")
case "$rom" in
    "$root"/roms/*.[nN][dD][sS]|/storage/roms/nds/*.[nN][dD][sS]) test -f "$rom" ;;
    *) printf 'DraStic UI Plus: unsupported ROM path: %s\n' "$rom" >&2; exit 64 ;;
esac

system_drastic=/usr/config/drastic
binary="$system_drastic/drastic"
expected=d54980a36c5e0b5cc46868bf25acd5d33b4c5337334fd0c417255a36942feb95
if [ ! -x "$binary" ] || ! printf '%s  %s\n' "$expected" "$binary" | sha256sum -c - >/dev/null; then
    printf '%s\n' 'DraStic UI Plus: the installed ROCKNIX DraStic build is unsupported.' >&2
    exit 69
fi

state="$root/state"
mkdir -p "$state"/Saves "$state"/cheats \
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

# Use the ROCKNIX defaults and honor local directories or symlinks.
prepare_save_directory() {
    path="$state/$1"
    if [ ! -e "$path" ] && [ ! -L "$path" ]; then
        mkdir -p "$2"
        ln -sT "$2" "$path" || test -d "$path"
    fi
    if [ ! -d "$path" ] || [ ! -w "$path" ]; then
        printf 'DraStic UI Plus: save directory is unavailable or not writable: %s\n' "$path" >&2
        exit 73
    fi
}
prepare_save_directory backup /storage/roms/nds
prepare_save_directory savestates /storage/roms/savestates/nds

cd "$state"
export DRASTIC_UI_PLUS_DIR="$state"
export SDCARD_PATH="$state"
export SDL_VIDEO_WAYLAND_WMCLASS=drastic
export SDL_TOUCH_MOUSE_EVENTS=0
exec env LD_PRELOAD="$root/lib/libdrastic-ui-plus.so" "$binary" "$rom" > "$root/session.log" 2>&1
