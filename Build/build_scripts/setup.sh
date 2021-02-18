#!/bin/bash
#==============================================================================
# Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.
#
# Redistribution of this file, in original or modified form, without
# prior written consent of Allied Vision Technologies is prohibited.
#
#------------------------------------------------------------------------------
#
# File:         -setup.sh
#
# Description:  -setup script for gcc cross compiler toolchain
#               -downloads specific gcc compilers and setups path
#               -linaro toolchain is used
#               -must be called with 'source setup.sh'
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
# include
#==============================================================================
source $(pwd)/file.sh
source $(pwd)/directory.sh
source $(pwd)/error.sh
#==============================================================================
# globals
#==============================================================================
GCC_NAME="gcc-linaro"

# path variables
PATH_EXT_GCC_ROOT="https://releases.linaro.org/components/toolchain/binaries"
PATH_GCC_ROOT="${PATH_DEV}/gcc"

# gcc cross compiler
GCC_ARMV7="arm-linux-gnueabihf"
GCC_ARMV8="aarch64-linux-gnu"
GCC_SELECTED=""

# gcc versions
FILE_VERSION_4_LATEST="4.9.4-2017.01"
FILE_VERSION_5_LATEST="5.5.0-2017.10"
FILE_VERSION_6_LATEST="6.4.1-2018.05"
FILE_VERSION_7_LATEST="7.5.0-2019.12"
FILE_VERSION_SELECTED=""
FOLDER_VERSION_SELECTED=""

# host platform
PLATFORM_HOST_32="i686"
PLATFORM_HOST_64="x86_64"
PLATFORM_HOST_SELECTED=""

# target platform
PLATFORM_TARGET_ARMV7="armv7"
PLATFORM_TARGET_ARMV8="armv8"
PLATFORM_TARGET_SELECTED=""

# selected gcc file and folder name
FILE_GCC_SELECTED=""
#==============================================================================
# print usage
#==============================================================================
usage()
{
    log info "Script usage: setup.sh <host> <target> <version major>"
    log_raw "   host:           'x32'......32bit host system"
    log_raw "                   'x64'......64bit host system"
    log_raw "   target:         'armv7'....ARM v7 architecture"
    log_raw "                   'armv8'....ARM v8 architecture"
    log_raw "   version major:  '4'........GCC of version 4.x.x"
    log_raw "                   '5'........GCC of version 5.x.x"
    log_raw "                   '6'........GCC of version 6.x.x"
    log_raw "                   '7'........GCC of version 7.x.x"
    log_raw "   *note that the script is always selecting the latest version of selected major version GCC."
}
#==============================================================================
# check if selected GCC exists
#==============================================================================
gcc_exists()
{
    if proceed
    then
        log info "Selected GCC host:${PLATFORM_HOST_SELECTED}, target:${PLATFORM_TARGET_SELECTED}, version:${FILE_VERSION_SELECTED}"
        if directory_exist "${PATH_GCC_ROOT}/${FILE_GCC_SELECTED}"
        then
            log debug "Selected GCC already exists in ${PATH_GCC_ROOT}/${FILE_GCC_SELECTED}"

            # set environment variables
            log info "Set environment variables for selected GCC"
            set_gcc_environment

            SUCCESS_FLAG=$FALSE
        else
            log debug "Selected GCC will be created in ${PATH_GCC_ROOT}/${FILE_GCC_SELECTED}"
            SUCCESS_FLAG=$TRUE
        fi
    fi
}
#==============================================================================
# create folder for selected GCC
#==============================================================================
create_gcc_folder()
{
    # check if development folder exist
    # and create it if not present
    if proceed
    then
        if ! directory_exist $PATH_DEV
        then
            log debug "Create development folder ${PATH_DEV}"
            call "create_directory ${PATH_DEV}"
        else
            log debug "${PATH_DEV} already exists"
            SUCCESS_FLAG=$TRUE
        fi
    fi

    # check if gcc root folder exist
    # and create it if not present
    if proceed
    then
        if ! directory_exist ${PATH_GCC_ROOT}
        then
            log debug "Create GCC folder ${PATH_GCC_ROOT}"
            call "create_directory ${PATH_GCC_ROOT}"
        else
            log debug "${PATH_GCC_ROOT} already exists"
            SUCCESS_FLAG=$TRUE
        fi
    fi
}
#==============================================================================
# download selected GCC
#==============================================================================
download_gcc()
{
    # download selected GCC
    if proceed
    then
        # check if file was already downloaded
        if ! file_exist "${PATH_DOWNLOADS}/${FILE_GCC_SELECTED}.tar.xz"
        then
            log info "Download GCC tarball ${PATH_EXT_GCC_ROOT}/${FOLDER_VERSION_SELECTED}/${GCC_SELECTED}/${FILE_GCC_SELECTED}.tar.xz to ${PATH_DOWNLOADS}"
            call "download_file ${PATH_EXT_GCC_ROOT}/${FOLDER_VERSION_SELECTED}/${GCC_SELECTED}/${FILE_GCC_SELECTED}.tar.xz ${PATH_DOWNLOADS}"
        else
            log debug "GCC tarball ${PATH_EXT_GCC_ROOT}/${FOLDER_VERSION_SELECTED}/${GCC_SELECTED}/${FILE_GCC_SELECTED}.tar.xz was already downloaded in ${PATH_DOWNLOADS}"
            SUCCESS_FLAG=$TRUE
        fi
    fi
}
#==============================================================================
# extract selected GCC tarball
#==============================================================================
extract_gcc()
{
    # extract GCC tarball
    if proceed
    then
        log info "Extract GCC tarball ${PATH_DOWNLOADS}/${FILE_GCC_SELECTED}.tar.xz to ${PATH_GCC_ROOT}}"
        call "extract_file_tar ${PATH_DOWNLOADS}/${FILE_GCC_SELECTED}.tar.xz ${PATH_GCC_ROOT}"
    fi

    # remove tarball after successful extraction
    if proceed
    then
        log info "Remove extracted tarball ${PATH_DOWNLOADS}/${FILE_GCC_SELECTED}.tar.xz"
        call "remove_file ${PATH_DOWNLOADS}/${FILE_GCC_SELECTED}.tar.xz"
    fi
}
#==============================================================================
# set environment variables for gcc cross compiling
#==============================================================================
set_gcc_environment()
{
    if proceed
    then
        # set env var for selected cross compiler
        export CROSS_COMPILE="${PATH_GCC_ROOT}/${FILE_GCC_SELECTED}/bin/${GCC_SELECTED}-"

        # set env var for architecture and cross compiler
        if check_parameter $PLATFORM_TARGET_SELECTED "armv7"
        then
            export ARCH=arm
        elif check_parameter $PLATFORM_TARGET_SELECTED "armv8"
        then
            export ARCH=arm
        fi

        log_raw "   ARCH:          ${ARCH}"
        log_raw "   CROSS_COMPILE: ${CROSS_COMPILE}"
    fi
}
#==============================================================================
# script execution: parameter parsing
#==============================================================================

# parsing parameter 1 (help or host architecture)
if parameter_exist $1
then
    if check_parameter $1 "help"
    then
        usage
        SUCCESS_FLAG=$FALSE
    elif check_parameter $1 "x32"
    then
        PLATFORM_HOST_SELECTED=$PLATFORM_HOST_32
        SUCCESS_FLAG=$TRUE
    elif check_parameter $1 "x64"
    then
        PLATFORM_HOST_SELECTED=$PLATFORM_HOST_64
        SUCCESS_FLAG=$TRUE
    else
        log error "${LOG_ERROR_PARAMETER_INVALID}"
        usage
        SUCCESS_FLAG=$FALSE
    fi
else
    log error "${LOG_ERROR_PARAMETER_MISSING}"
    usage
    SUCCESS_FLAG=$FALSE
fi

# parsing parameter 2 (target)
if proceed
then
    if parameter_exist $2
    then
        if check_parameter $2 "armv7"
        then
            PLATFORM_TARGET_SELECTED=$PLATFORM_TARGET_ARMV7
            GCC_SELECTED=$GCC_ARMV7
            SUCCESS_FLAG=$TRUE
        elif check_parameter $2 "armv8"
        then
            PLATFORM_TARGET_SELECTED=$PLATFORM_TARGET_ARMV8
            GCC_SELECTED=$GCC_ARMV8
            SUCCESS_FLAG=$TRUE
        else
            log error "${LOG_ERROR_PARAMETER_INVALID}"
            usage
            SUCCESS_FLAG=$FALSE
        fi
    else
        log error "${LOG_ERROR_PARAMETER_MISSING}"
        usage
        SUCCESS_FLAG=$FALSE
    fi
fi

# parsing parameter 3 (version major)
if proceed
then
    if parameter_exist $3
    then
        if check_parameter $3 "4"
        then
            FOLDER_VERSION_SELECTED="latest-4"
            FILE_VERSION_SELECTED=$FILE_VERSION_4_LATEST
            SUCCESS_FLAG=$TRUE
        elif check_parameter $3 "5"
        then
            FOLDER_VERSION_SELECTED="latest-5"
            FILE_VERSION_SELECTED=$FILE_VERSION_5_LATEST
            SUCCESS_FLAG=$TRUE
        elif check_parameter $3 "6"
        then
            FOLDER_VERSION_SELECTED="latest-6"
            FILE_VERSION_SELECTED=$FILE_VERSION_6_LATEST
            SUCCESS_FLAG=$TRUE
        elif check_parameter $3 "7"
        then
            FOLDER_VERSION_SELECTED="latest-7"
            FILE_VERSION_SELECTED=$FILE_VERSION_7_LATEST
            SUCCESS_FLAG=$TRUE
        else
            log error "${LOG_ERROR_PARAMETER_INVALID}"
            usage
            SUCCESS_FLAG=$FALSE
        fi
    else
        log error "${LOG_ERROR_PARAMETER_MISSING}"
        usage
        SUCCESS_FLAG=$FALSE
    fi
fi

# create selected gcc filename
if proceed
then
    FILE_GCC_SELECTED="${GCC_NAME}-${FILE_VERSION_SELECTED}-${PLATFORM_HOST_SELECTED}_${GCC_SELECTED}"
    log info "Using selected GCC filename ${FILE_GCC_SELECTED}"}
fi
#==============================================================================
# run script with respective parameter
#==============================================================================
gcc_exists
create_gcc_folder
download_gcc
extract_gcc
set_gcc_environment

# if everything goes smoothly, print success
if proceed
then
    log info "Setup successful"
fi
