#!/bin/sh
# Run this to generate all the initial makefiles, etc.

set -e

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

cd "$srcdir"
mkdir -p m4 >/dev/null 2>&1 || true
autoreconf --verbose --force --install
intltoolize --force
cd -

if ! grep -q AX_CXX_COMPILE_STDCXX_11 aclocal.m4; then
	echo
	echo "ERROR: The AX_CXX_COMPILE_STDCXX_11 macro from the GNU Autoconf Archive was not found."
	echo
	echo "Try installing the autoconf-archive package and re-running autogen.sh"
	echo
	exit 1
fi

echo
echo "----------------------------------------------------------------"
echo "Initialized build system. For a common configuration please run:"
echo "----------------------------------------------------------------"
echo
echo "./configure CFLAGS='-g -O0' $args"
echo
