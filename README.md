# ISSTracker live

ISSTracker live is an AmigaOS 1.3 Workbench application that displays the current ISS ground position on an equirectangular world map.

Features:
- Plain HTTP/1.0 fallback fetch from `api.open-notify.org/iss-now.json` through a bsdsocket-compatible library.
- Optional HTTPS enhanced fetch through `amitls13.library` from `https://api.wheretheiss.at/v1/satellites/25544` when the library is installed.
- Enhanced HTTPS data can show altitude, velocity, footprint, visibility and solar coordinates.
- HTTP crew fetch from `api.open-notify.org/astros.json` with people-in-space count, spacecraft names and crew names.
- Minimal JSON/string scanners; no heavyweight JSON dependency.
- Coordinates stored as signed 16-bit centi-degrees.
- Blinking ISS marker and a 16-position trail.
- Local city database and integer nearest-city estimate.
- Local day/twilight/night estimate from UTC timestamp and coordinates.
- Local land/water/coast classification.
- Rotating localized status lines for position, update time, ISS details, crew summary and crew names.
- Classic Intuition-style Workbench UI for Kickstart/Workbench 1.3.
- No proxy, no GadTools, no palette changes.

Build:

```sh
make clean && make
```

Target compiler: bebbo/amiga-gcc with `-mcrt=nix13`. Recommended stack: `131072` when optional HTTPS via `amitls13.library` is used. HTTP-only fallback remains lighter.
