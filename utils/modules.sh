
KVER=`uname -r`
set -e

mkdir modules/$KVER
cp Can/driver/Can.o modules/$KVER

