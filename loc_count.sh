#!/bin/bash

wc -l `find src | grep '\.cc'` `find src | grep '\.h'` | grep 'total' | sed -e 's/total//' \
	| tr '\n' '\0' | tr ' ' '\0'
