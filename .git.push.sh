#!/usr/bin/env bash
#
make clean
git add .
git commit -m "$(date +"%F %T")"
git push -u origin main
