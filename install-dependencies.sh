#!/bin/bash

if [ -e /etc/fedora-release ]; then
	echo Detected fedora
	yum install gcc-c++ libtool gtkmm24-devel alsa-lib-devel jack-audio-connection-kit-devel lash-devel
	exit $?
fi

