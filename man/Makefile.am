
SUBDIRS = de fr

man_MANS = amsynth.1

amsynth.1: amsynth.1.md
	pandoc -s -t man -V footer='$(PACKAGE_STRING)' -o $@ $<

EXTRA_DIST = amsynth.1.md

DISTCLEANFILES = $(man_MANS)
