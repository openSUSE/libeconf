# Building and installing libeconf

libeconf supports two options to build it: Meson (preferred) or CMake (alternate)

## Building with Meson

libeconf requires a relatively recent version of Meson, version 0.49 or newer.

Building with Meson is quite simple:

```shell
$ meson build
$ ninja -C build
$ ninja -C build test
```

If you want to build with the address sanitizer enabled, add
`-Db_sanitize=address` as an argument to `meson build`.

## Building with CMake

libeconf requires CMake 3.12 or newer.

Building with CMake is straightforward:

```shell
$ cmake -B build
$ make -C build
$ make -C build check
$ make -C build doc
```

If you want to build with the address sanitizer enabled, add
`-DCMAKE_BUILD_TYPE=SanitizeAddress` as an argument to `cmake -B build`.
