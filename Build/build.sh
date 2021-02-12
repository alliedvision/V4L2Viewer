#!/bin/bash
#==============================================================================
# Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.
#
# Redistribution of this file, in original or modified form, without
# prior written consent of Allied Vision Technologies is prohibited.
#
#------------------------------------------------------------------------------
#
# File:         -build.sh
#
# Description:  -bash script for building the V4L2Viewer
#               -provide parameter for cross compiling: armv7, armv8
#
#------------------------------------------------------------------------------
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF TITLE,
# NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR  PURPOSE ARE
# DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
# AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
# TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#==============================================================================
#
#==============================================================================
# save current path
#==============================================================================
PATH_CURRENT=$(pwd)
#==============================================================================
# include function calling with error handling
#==============================================================================
source build_scripts/error.sh
#==============================================================================
# compile function
#==============================================================================
v4l2_compile()
{
    log info "Build v4l2 test tool"
    cd Make
    call "make -j4"
    cd $PATH_CURRENT
}
#==============================================================================
# cross compile function
#==============================================================================
v4l2_cross_compile()
{
    log info "Build v4l2 test tool"
    cd Make
    call "make -j4 WORDSIZE=${WORDSIZE} ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} CONFIG=${CONFIG}"
    cd $PATH_CURRENT
}
#==============================================================================
# set paths used by the build scripts
#==============================================================================
PATH_DEV=$PATH_CURRENT
PATH_DOWNLOADS=~/Downloads
#==============================================================================
# initialize helper functions
#==============================================================================
source build_scripts/common.sh
source build_scripts/logging.sh
#==============================================================================
# script execution
#==============================================================================
# check if parameter 2 was given
# and set build config (debug or release)
if parameter_exist $2
then
    if check_parameter $2 "Release" || check_parameter $2 "release"
    then
        CONFIG="Release"
    else
        CONFIG="Debug"
    fi
else
    CONFIG="Debug"
fi

# check if parameter 1 was given
# and set environment for cross compiling respectively
if parameter_exist $1
then
    if check_parameter $1 "armv7"
    then
        # setup armv7 gcc for cross compiling
        log info "Setup GCC for armv7 and x64 host"
        cd build_scripts
        source setup.sh x64 armv7 4
        cd $PATH_CURRENT
        WORDSIZE=32

    elif check_parameter $1 "armv8"
    then
        # setup armv7 gcc for cross compiling
        log info "Setup GCC for armv8 and x64 host"
        cd build_scripts
        source setup.sh x64 armv8 7
        cd $PATH_CURRENT
        WORDSIZE=64

    fi

    log info "Compiling with WORDSIZE=${WORDSIZE} ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} CONFIG=${CONFIG}"

    v4l2_cross_compile

else
    v4l2_compile
fi

# print footer on success
if proceed
then
    log info "Build successful"
fi
