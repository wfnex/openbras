#!/bin/sh
#
# Copyright (C) 2013-2014, Nanjing StarOS Technology Co., Ltd
#
CURRENT=`pwd`
export ROOTDIR=$CURRENT/../

export USER_ROOT=$CURRENT/../

export BUILD_NUMBER=1

export OPENBRAS_VERSION="1.0.0"

export OPENBRAS_STAGE_ROOT=/tmp
export OPENBRAS_RPM_ROOT=$OPENBRAS_STAGE_ROOT/rpm
export OPENBRAS_RPM_VERSION=${OPENBRAS_VERSION//-/.}
export OPENBRAS_STAGE=$CURRENT/openbras


