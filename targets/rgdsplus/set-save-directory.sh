#!/bin/sh
# Point DraStic's in-game saves at an existing directory.
set -eu

if [ "$#" -ne 1 ]; then
    printf 'Usage: %s SAVE_DIRECTORY\n' "$0" >&2
    exit 64
fi

root=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
if [ ! -x "$root/launch-rom.sh" ]; then
    printf 'Run this script from the installed DraStic UI Plus directory.\n' >&2
    exit 1
fi
if pgrep -x drastic >/dev/null; then
    printf 'Close DraStic before changing the save directory.\n' >&2
    exit 1
fi
if [ ! -d "$1" ] || [ ! -w "$1" ]; then
    printf 'Save directory must already exist and be writable: %s\n' "$1" >&2
    exit 1
fi

case "$1" in
    /*) target=$(realpath "$1") ;;
    *) target=$(realpath "$PWD/$1") ;;
esac
backup="$root/state/backup"
mkdir -p "$root/state"
if [ -L "$backup" ]; then
    current=$(readlink "$backup")
    case "$current" in
        "$target") ;;
        /storage/roms/nds) ln -sfnT "$target" "$backup" ;;
        *)
            printf 'A custom link already exists. Back it up before changing it: %s -> %s\n' "$backup" "$current" >&2
            exit 1 ;;
    esac
elif [ -e "$backup" ]; then
    printf 'Back up and move the existing path before retrying: %s\n' "$backup" >&2
    exit 1
else
    ln -sT "$target" "$backup"
fi
printf '%s -> %s\n' "$backup" "$target"
