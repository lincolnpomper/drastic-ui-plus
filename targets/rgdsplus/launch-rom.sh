#!/bin/bash
# The menu reads the ROM header directly. Stage compressed games as raw .nds files.
. /etc/profile.d/001-functions
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
rom=$(realpath "${1:?Pass a Nintendo DS ROM}")
game_name=${rom##*/}
shader=$(get_setting drastic_ui_plus_shader nds "$game_name")
if [ -z "$shader" ] || [ "$shader" = auto ]; then
    shader=$(get_setting shader nds "$game_name")
fi
export DRASTIC_UI_PLUS_SHADER="$shader"
intensity=$(get_setting drastic_ui_plus_lcd_intensity nds "$game_name")
case "$intensity" in
    soft|medium|original|strong) ;;
    *) intensity=original ;;
esac
export DRASTIC_UI_PLUS_LCD_INTENSITY="$intensity"
DRASTIC_UI_PLUS_BRIGHTEN_SCANLINES=$(get_setting drastic_ui_plus_brighten_scanlines nds "$game_name")
DRASTIC_UI_PLUS_BRIGHTEN_LCD=$(get_setting drastic_ui_plus_brighten_lcd nds "$game_name")
export DRASTIC_UI_PLUS_BRIGHTEN_SCANLINES DRASTIC_UI_PLUS_BRIGHTEN_LCD
export DRASTIC_UI_PLUS_SAVE_DIR=/storage/roms/saves/retroarch/nds
export DRASTIC_UI_PLUS_STATE_DIR=/storage/roms/saves/states/retroarch/nds
case "$rom" in
    /storage/roms/nds/*.[nN][dD][sS]) exec "$root/launch.sh" "$rom" ;;
    /storage/roms/nds/*.[zZ][iI][pP]|/storage/roms/nds/*.[7][zZ]) ;;
    *) printf 'DraStic UI Plus: unsupported ROM path: %s\n' "$rom" >&2; exit 64 ;;
esac

mkdir -p "$root/roms"
work=$(mktemp -d "$root/roms/.archive.XXXXXX")
trap 'rm -rf -- "$work"' EXIT
trap 'exit 130' INT
trap 'exit 143' TERM
7za l -slt -- "$rom" > "$work/members"
member=$(awk '
    /^----------$/ { members = 1; next }
    members && /^Path = / {
        candidate = substr($0, 8)
        # macOS adds AppleDouble resource metadata beside archived ROMs.
        if (tolower(candidate) !~ /\.nds$/ ||
            candidate ~ /(^|[\/\\])__MACOSX([\/\\]|$)/ ||
            candidate ~ /(^|[\/\\])\._[^\/\\]*$/)
            next
        path = candidate
        count++
    }
    END { if (count == 1) print path; else exit 1 }
' "$work/members") || {
    printf 'DraStic UI Plus: expected exactly one .nds file in %s\n' "$rom" >&2
    exit 65
}
name=${rom##*/}
game="$work/${name%.*}.nds"
7za x -so -spd -- "$rom" "$member" > "$game"
test -s "$game"
"$root/launch.sh" "$game"
