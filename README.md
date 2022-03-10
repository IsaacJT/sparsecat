# sparsecat

# Overview

Simple utility for extracting the data from an Android sparse image format file and sending it to stdout.

# Building

This project uses the `meson` build system and depends on the Android `libsparse` library. Use the following commands to build:

    meson builddir
    ninja -C builddir

The binary will then be available in `builddir/sparsecat`.

# Usage

This program takes one argument: a sparse image file. This must be provided and the output must be piped to another program.

Example:

    sparsecat inputfile.img | dd of=/dev/mmcblk0
