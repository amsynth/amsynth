#!/bin/sh
# Run this to generate all the initial makefiles, etc.

set -e

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

cd "$srcdir"
mkdir -p m4 >/dev/null 2>&1 || true
autoconf --version | head -n1
automake --version | head -n1
autoreconf --verbose --force --install
intltoolize --force
cd -

echo
echo "----------------------------------------------------------------"
echo "Initialized build system. For a common configuration please run:"
echo "----------------------------------------------------------------"
echo
echo "./configure"
echo
