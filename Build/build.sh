#!/bin/bash
#==============================================================================
# Copyright (C) 2018 Allied Vision Technologies.  All Rights Reserved.
#
# Redistribution of this file, in original or modified form, without
# prior written consent of Allied Vision Technologies is prohibited.
#
#------------------------------------------------------------------------------
#
# File:         -build.sh
#
# Description:  -bash script for building v4l2test tool
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
# run environment script 
# and include error handling
#==============================================================================
PATH_CURRENT=$(pwd)
cd ../Externals/build_scripts/common
source ./env.sh
cd $PATH_CURRENT
#==============================================================================
# include
#==============================================================================
source $PATH_SCRIPTS_COMMON/error.sh
#==============================================================================
# compile
#==============================================================================
v4l2_compile()
{
    log info "Build v4l2 test tool"
    cd Make
    call "make -j4"
    cd $PATH_CURRENT
}
#==============================================================================
# cross compile
#==============================================================================
v4l2_cross_compile()
{
    log info "Build v4l2 test tool"
    cd Make
    call "make -j4 WORDSIZE=${WORDSIZE} ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} CONFIG=${CONFIG}"
    cd $PATH_CURRENT
}
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
        cd $PATH_SCRIPTS_COMMON/gcc
        source setup.sh x64 armv7 4
        cd $PATH_CURRENT
        WORDSIZE=32

    elif check_parameter $1 "armv8"
    then
        # setup armv7 gcc for cross compiling
        log info "Setup GCC for armv8 and x64 host"
        cd $PATH_SCRIPTS_COMMON/gcc
        source setup.sh x64 armv8 7
        cd $PATH_CURRENT
        WORDSIZE=64

    fi

    v4l2_cross_compile

else
    v4l2_compile
fi

# print footer on success
if proceed
then
    show_logo timmay
fi
