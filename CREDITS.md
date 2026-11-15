# Credits

Fun Drastic — name and concept: **tenlevels**.

Fun Drastic is a frontend and compatibility layer; the work below is other
people's, and it is what makes Fun Drastic possible.

## The emulator

- **DraStic** — proprietary Nintendo DS emulator by **Exophase** (with
  Lordus and slaanesh). DraStic UI Plus runs the compatible executable
  already installed by ROCKNIX. The executable is excluded from this
  repository and its release packages. DraStic UI Plus is an unofficial
  add-on with no affiliation with or endorsement from Exophase.
- **DraStic free BIOS** — ARM7/ARM9 BIOS replacements supplied with
  DraStic. The add-on references the copies provided by ROCKNIX. This
  repository and its release packages exclude both the DraStic replacement
  BIOS files and Nintendo BIOS files.

## Libraries

- **SDL2** — Simple DirectMedia Layer (zlib license). Fun Drastic targets the
  SDL2 API; DraStic's own SDL2 provides it at run time.
- **stb_image** and **stb_truetype** — image loading and TrueType rasterising
  by Sean Barrett (public domain / MIT).

## Fonts

- **Noto Sans CJK SC** (shipped as `fonts/Translate.otf`) — Google, under the
  SIL Open Font License 1.1. Renders translated menus, including Chinese. The
  default menu uses Fun Drastic's own built-in bitmap font; this font is used
  only for languages the bitmap font can't draw.

## Data

- **usrcheat.dat** — community-maintained DS cheat database, assembled over
  many years by the DS cheat scene.
- **game_database.xml** — DraStic's save-type database.

## Per-device packages

A CFW/device package may bundle its own extras — a theme font, an SDL stack,
libraries specific to that firmware. Those carry their own credits in their
target folder, not here.

## Trademarks

Nintendo, Nintendo DS, and all game titles are trademarks of their respective
owners. DraStic UI Plus does not bundle Nintendo code or game assets.
The README screenshots illustrate the add-on in use. Theme names refer
only to color palettes.

## DraStic UI Plus

- **[lincolnpomper](https://github.com/lincolnpomper)** — RG DS Plus adaptation and maintenance.
- **Gigaherz** — public-domain [LCD3x shader distributed with RetroArch](https://github.com/libretro/slang-shaders/blob/master/handheld/shaders/lcd3x.slang), the basis for the RGB LCD3x and monochrome LCD1x masks implemented in SDL.
