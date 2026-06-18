# ISS Tracker

ISS Tracker is an AmigaOS 1.3 Workbench application that displays the current ISS ground position on an equirectangular world map.

Features:
- Plain HTTP/1.0 fetch from `api.open-notify.org/iss-now.json` through a bsdsocket-compatible library.
- Minimal JSON scanner for latitude, longitude, and timestamp.
- Coordinates stored as signed 16-bit centi-degrees.
- Blinking ISS marker and a 16-position trail.
- Local city database and integer nearest-city estimate.
- Local day/twilight/night estimate from UTC timestamp and coordinates.
- Local land/water/coast classification.
- Classic Intuition-style Workbench UI for Kickstart/Workbench 1.3.
- No HTTPS, no proxy, no GadTools, no palette changes.

Build:

```sh
make clean && make
```

Target compiler: bebbo/amiga-gcc with `-mcrt=nix13`. Recommended stack: `8000`.
