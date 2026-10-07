# bidi_line245

`bidi_line245` is a C++17 library for laying out one mixed Arabic, Hebrew and
Latin line with FriBidi and HarfBuzz. It returns visual glyph records and
byte-accurate logical selection ranges.

## Build

```sh
make
make test
./bin/mixed_example
./bin/selection_example
```

The Makefile uses `third_party/include`, `third_party/lib` and embeds an
`$ORIGIN`-relative runtime library search path. No CMake is required.

## API

```cpp
#include <bidi_line245/bidi_line245.hpp>

bidi_line245::LineShaper shaper("fonts/DejaVuSans.ttf");
auto line = shaper.shape_line(text, 28.0, bidi_line245::BaseDirection::Auto);
auto ranges = shaper.selection_boxes(line, text, byte_begin, byte_end);
```

`Auto` uses the first strong direction and defaults to LTR. Explicit `Ltr`
and `Rtl` are also supported. The origin is the left baseline; X increases
right, Y increases up, and all measurements are pixels. Spaces retain their
actual advance. `x_advance` is a positive span and `rtl` identifies RTL runs.

Glyph cluster fields are half-open UTF-8 byte offsets in the original input.
Ligatures and combining sequences cover every merged source code point.
Directional formatting and isolate controls have no visible glyphs.

Selection endpoints are half-open byte offsets and must be on code-point
boundaries. Touching any part of a cluster selects that whole cluster.
Adjacent visual rectangles are merged, while disconnected mixed-script
selections can return several ranges. Empty selections return an empty vector.

The library rejects invalid UTF-8, line breaks, non-positive or non-finite
font sizes, unsupported visible characters, out-of-range or inverted byte
ranges, endpoints inside code points, and input longer than 4096 code points.
Empty text produces an empty layout.

## Modules

- `src/unicode_text.*`: UTF-8 decoding, boundaries and scripts.
- `src/bidi_analysis.*`: FriBidi levels, mirrors, isolates and visual order.
- `src/glyph_layout.*`: HarfBuzz shaping, RTL geometry and run placement.
- `src/selection.*`: cluster expansion and rectangle merging.
