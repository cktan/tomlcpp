# Copyright (c) 2022, Sonos, Inc.  All rights reserved.

project(
    'tomlcpp',
    ['c', 'cpp'],
    meson_version: '>=0.56.0',
    default_options: [
        'b_ndebug=true',
        'c_std=c11',
        'cpp_std=c++14',
        'debug=true',
        'optimization=2',
        'strip=false',
        'warning_level=3',
        'werror=true',
    ]
)

pkg_mod = import('pkgconfig')

headers = [
    'toml.h',
    'tomlcpp.hpp',
]

src_files = [
    'toml.c',
    'tomlcpp.cpp',
]

cflags = ['-fpic']

if meson.is_cross_build()
    add_global_arguments(
        cflags,
        language: ['c', 'cpp'],
    )
else
    add_global_arguments(
        cflags,
        language: ['c', 'cpp'],
    )
endif

_lib = library(
    'tomlcpp',
    src_files,
)

_dep = declare_dependency(
    link_with: _lib
)

_exe = []

_exe += executable(
    'toml_json',
    'toml_json.cpp',
    dependencies: _dep,
)

_exe += executable(
    'toml_sample',
    'toml_sample.cpp',
    dependencies: _dep,
)

_pkg = pkg_mod.generate(
    _lib,
    version: '1.0',
    name: 'tomlcpp',
    description: 'toml c++ library',
)
