#! /bin/sh
#
# provide $(MAKE) with values for linux KVERSION and INCLUDE.
# 
# a file ./+linux-info may be specified containing assignments to
# variables `kver' and `linux' about the kernel version and source tree
# directory.

set -e

prog=linux-info

if test -f +linux; then
	linux=`cat +linux | sed '/^#/d'`
else
	linux=,,linux-headers
	if ! test -d $linux; then
		mkdir $linux
		cd $linux
		echo '* extracing linux headers' >&2
		if ! tar xf ../../runsys/linux/linux-headers.tar; then
			cd ..
			rm -rf $linux
			exit 1
		else
			cd ..
		fi
	fi
fi

wd=`pwd`
if test -d "$linux"; then
    cd $linux
    if test -d include/linux; then :
    else
	echo "$prog: Seems not to be a linux source directory: $linux" >&2
	exit 1
    fi
else
    echo "$prog: Not a directory: $linux" >&2
    exit 1
fi

# wd is `$linux'



# print some information for the developer
utsh=include/generated/utsrelease.h
vh=include/linux/version.h
if test -f $utsh; then
	vh=$utsh
fi
uts_release=`cat $vh | sed -n '/#.*fine *UTS_RELEASE/s,.*UTS_REL.* ",,p' | sed 's, *" *$,,'`


case $1 in
    --kversion)
	echo $uts_release
	;;
    --srcdir)
	echo $linux
	;;
    -I)
	echo $linux/include
	;;
    *)
cat <<EOF >&2

  ,------------------------
  | Kernel source tree:
  |	$linux
  |
  | Version:
  |   $uts_release
  |
  \`------------------------


EOF
;;
esac




#
# arch-tag: M.T. Mo Jan 31 18:48:41 CET 2005
#
