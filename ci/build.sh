#!/bin/sh -ex
#
# Copyright (c) 2018-2024 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: GPL-2.0-or-later

opts='-Doptimization=2 -Dwerror=true'

case "${ENABLE_VALGRIND-}" in
	yes)
		opts="$opts -Dpam-debug=true"
		;;
esac

echo 'BEGIN OF BUILD ENVIRONMENT INFORMATION'
uname -a |head -1
libc="$(ldd /bin/sh |sed -n 's|^[^/]*\(/[^ ]*/libc\.so[^ ]*\).*|\1|p' |head -1)"
$libc |head -1
$CC --version |head -1
meson --version |head -1
ninja --version |head -1
kver="$(printf '%s\n%s\n' '#include <linux/version.h>' 'LINUX_VERSION_CODE' | $CC -E -P -)"
printf 'kernel-headers %s.%s.%s\n' $((kver/65536)) $((kver/256%256)) $((kver%256))
echo 'END OF BUILD ENVIRONMENT INFORMATION'

mkdir build
meson setup $opts build

# If "meson dist" supported -v option, it could be used here
# instead of all subsequent individual meson commands.

meson compile -v -C build
mkdir build/destdir
DESTDIR=$(pwd)/build/destdir meson install -C build
meson test -v -C build

if [ -n "${ENABLE_VALGRIND}" ]; then
	meson test -v -C build --wrap='valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1'
fi

if git status --porcelain |grep '^?'; then
	echo >&2 'git status reported untracked files'
	exit 1
fi
