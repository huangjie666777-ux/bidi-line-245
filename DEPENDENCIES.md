# Build dependencies

C++17, g++11.4.0 and GNU Make4.3 are directly available in WSL.
HarfBuzz2.7.4 and FriBidi1.0.8 Linux x86_64 headers and shared libraries
are vendored in third_party/include and third_party/lib. This directory
contains dependencies and a single unmodified DejaVuSans.ttf font only.
No business library implementation or build system is supplied.

Compiler flags:
`-Ithird_party/include/harfbuzz -Ithird_party/include/fribidi`
Linker flags:
`-Lthird_party/lib -lharfbuzz -lfribidi`
For executables under bin or build, use this relative runtime search path:
`-Wl,-rpath,'$ORIGIN/../third_party/lib'`
No PATH or LD_LIBRARY_PATH setup is needed with the correct runtime path.
Font bytes can be opened using hb_blob_create_from_file, with hb_face_create,
hb_font_create and hb_ot_font_set_funcs; FreeType headers are not needed.

third_party/dependencies.json records pinned Ubuntu package sources and
checksums. Corresponding FriBidi source and Ubuntu patches are included
under third_party/source. License notices are under third_party/licenses
and fonts/LICENSE. The OS supplies the same transitive system libraries
to every project copy; no system installation or upgrade was performed.
