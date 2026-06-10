#!/bin/sh
cp wlcf.m wlcf.1
man -M . ./wlcf.1 | man2html -topm 0 -botm 0 -cgiurl \$title.html > ./wlcf.html
