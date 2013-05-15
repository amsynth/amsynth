#!/bin/bash

if [ -e /etc/fedora-release ]; then
	echo Detected fedora
	yum install gcc-c++ libtool gtkmm24-devel alsa-lib-devel jack-audio-connection-kit-devel lash-devel
	exit $?
fi

if [ -e /etc/apt/sources.list ]; then
	echo Detected Debian-based system
	apt-get install pkg-config libgtk2.0-dev libgtkmm-2.4-dev libjack-jackd2-dev
fi
