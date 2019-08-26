#!/bin/sh -x

rm -fv ltmain.sh config.sub config.guess config.h.in aclocal.m4
mkdir -p m4
aclocal -I m4
#autoheader
libtoolize --automake --copy
# Fix ltmain.sh for -fsanitize
sed -i -e 's/-stdlib=\*)/-stdlib=\*|-fsanitize=\*)/g' ltmain.sh
automake --add-missing --copy --force
autoreconf
chmod 755 configure
