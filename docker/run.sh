#!/bin/bash

#GIT_URL="https://git.hiddenbox.org/lotherk/pioengine.git"
GIT_URL="http://10.10.0.4/lotherk/pioengine.git"

git clone $GIT_URL /tmp/pioe || exit -1
cd /tmp/pioe
mkdir build
cd build
cmake .. || exit -2
make || exit -3
make test || exit -4
make package || exit -5
cp -v *deb *zip *msi /tmp/pkg/ || true
exit 0
