#!/bin/bash

# scm_version script
# Retrieves either svn revision number or git revision number.
# In case of local changes the retruned revision number is 0.
# Parameters:
# $1: Path to get the SVN(!) revision number for

# Check if we use git
if 	isgit=`git rev-parse --is-inside-work-tree 2>/dev/null` &&
	test -n $isgit &&
	rev=`git rev-parse --verify --short HEAD 2>/dev/null`; then

	printf -- 'git%s' "$rev"
	git update-index -q --refresh
	if  ! git diff-index --quiet --name-only HEAD 2>/dev/null; then
		printf '%s' -dirty		
	fi
	exit
fi
# Check if we use svn
if  rev=`LANG= LC_ALL= LC_MESSAGES=C svn info $1 2>/dev/null | grep '^Last Changed Rev'`; then
	rev=`echo $rev | awk '{print $NF}'`
	printf -- 'svn%s' "$rev"
	if 	mod=`LANG= LC_ALL= LC_MESSAGES=C svnversion -n $1 2>/dev/null | grep -o '[^0-9]*' | tr -d '\n'` &&
       	test -n $mod; then
		printf '%s' -dirty
	fi
	exit
fi

printf "0"

