# NAME
sparsecat - tool for extracting the data from an Android sparse image

# SYNOPSIS

Simple utility for extracting the data from an Android sparse image format file
 and sending it to stdout.

# DESCRIPTION

This project uses the `meson` build system and depends on the Android
`libsparse` library. Use the following commands to build:

    meson builddir
    ninja -C builddir

The binary will then be available in `builddir/sparsecat`.

# EXAMPLES

This program takes one argument: a sparse image file. This must be provided and
the output must be piped to another program.

    sparsecat inputfile.img | dd of=/dev/mmcblk0

# COPYRIGHT

Copyright (C) 2023 Isaac True <isaac@is.having.coffee>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
