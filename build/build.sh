#!/bin/bash
#
# Copyright (C) 2013-2014, Nanjing WFNEX Technology Co., Ltd
#
source ./env.sh

help()
{
echo ===============================================================================
echo build usage:
echo ===============================================================================
echo "./build.sh [type]"
echo "[type]:"
echo "          help    		: help info" 
echo "          debug     		: build a debug"
echo "          release		    : build a release"
echo "          all 		    : build a debug and release"
echo ===============================================================================
}

if [ "$1" == "help" ] || [ "$1" == "" ]
then
	help
	exit 1
fi

if [ "$1" == "debug" ]
then
	make -f Makefile debug=1 optimize=0
elif [ "$1" == "release" ]
then
	make -f Makefile debug=0 optimize=1
elif [ "$1" == "all" ]
then
	make -f Makefile cleanall
	make -f Makefile debug=0 optimize=1
	make -f Makefile cleanall
	make -f Makefile debug=1 optimize=0
	make -f Makefile cleanall
else
	help
	exit 1
fi

