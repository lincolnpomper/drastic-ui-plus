# DraStic UI Plus

Unofficial UI add-on for DraStic based on [Fun Drastic](https://github.com/Utility-Muffin-Research-Kitchen/Fun-Drastic-src) by tenlevels and contributors. This branch targets the Anbernic RG DS Plus running ROCKNIX.

Includes dual-screen display and touch input, shared DraStic saves and save states, native fast-forward, LCD Grid and RetroArch LCD1x/LCD3x with adjustable intensity, and vertical save-state previews captured before LCD filtering.

The base import is Fun Drastic commit [7514876](https://github.com/Utility-Muffin-Research-Kitchen/Fun-Drastic-src/commit/75148764fd171f0f0e013fa3a13bedd9b9fb0991). The RG DS Plus code comes from Lincoln Pompermaier's local Fun Drastic branch at commit `2ebabbf474910686b5f4a6e358a6a8d3a3b90501`. See [LICENSE](LICENSE) and [CREDITS.md](CREDITS.md).

## Build and package

```sh
make image
make rgdsplus
make pack-rgdsplus
```

The archive in `dist/` contains the UI library, scripts, EmulationStation configuration, and documentation. The launcher executes the compatible DraStic build at `/usr/config/drastic/drastic` and references the BIOS and emulator data already on the console. It checks the executable hash before loading the library because the integration depends on that build.

## Install on ROCKNIX

Close the running game before installation. Copy the archive to the console and extract it under `/storage` so the launcher is at `/storage/drastic-ui-plus/launch-rom.sh`. Install the archive's two `es_*.cfg` files in `/storage/.config/emulationstation/`, then restart EmulationStation. If the earlier Fun Drastic test integration is installed, move its `es_*_fun_drastic.cfg` files outside that directory first to avoid competing Nintendo DS overrides.

Choose **DraStic UI Plus** as the Nintendo DS emulator. The **LCD Filter** menu offers LCD Grid, LCD1x, LCD3x and Off. **Intensity** applies to all three filters, and **Brighten Scanlines** and **Brighten LCD** tune the horizontal and vertical masks in LCD1x and LCD3x. Reopen the game after changing settings.

The add-on uses its own `state/` folder for UI settings and preview images. In-game saves and save states use the shared ROCKNIX locations configured by the launcher.
