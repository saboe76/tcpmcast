#!/usr/bin/env bash
#
git add .
git commit -m "$(date +"%F %T")"
git push -u origin main
