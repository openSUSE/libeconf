# Building and installing libeconf

libeconf supports two options to build it: Meson (preferred) or CMake (alternate)

## Building with Meson

libeconf requires a relatively recent version of Meson, version 0.49 or newer.

Building with Meson is quite simple:

```shell
$ meson build
$ ninja -C build
$ ninja -C build test
$ sudo ninja -C build install
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
$ sudo make -C build install
```

If you want to build with the address sanitizer enabled, add
`-DCMAKE_BUILD_TYPE=SanitizeAddress` as an argument to `cmake -B build`.

# Tagging new Release

1. Edit NEWS declaring the new version number and making all the changes to it.
2. Update the version number in CMakeLists.txt and meson.build.
2. Commit to git.
3. On https://github.com/openSUSE/libeconf click on releases on the right column (or go to https://github.com/openSUSE/libeconf/releases)
4. 'Draft a new release'.
5. In 'tag version' write vX.Y.Z matching the new version you declared in your NEWS commit.
6. Write a release title (e.g. "Release Version X.Y.Z")
7. In the description just copy/paste the NEWS entry.
8. Publish the release.

This creates a git tag for all the latest commits (hence the NEWS commit being important) under the new version number.
