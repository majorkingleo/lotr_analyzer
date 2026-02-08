#!/usr/bin/env bash

for i in ../data/* ; do
	zstdcat "${i}" > `basename ${i}`
done
