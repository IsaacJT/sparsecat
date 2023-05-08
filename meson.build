# SPDX-License-Identifier: GPL-2.0

project('sparsecat', 'c',
  version : '0.1',
  default_options : ['c_std=gnu17', 'warning_level=3'])
deps = [
  meson.get_compiler('c').find_library(':android/libsparse.so')
]
executable('sparsecat',
           'src/main.c',
           install : true,
           dependencies: deps)
