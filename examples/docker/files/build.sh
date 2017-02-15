#!/bin/bash
#
# Build script for LEDE/OpenWRT and Socketman
#
# Copyright (C) 2017 Chris Blake <chrisrblake93@gmail.com>
#

# Used to get CPU count for build time
CPUCOUNT=$(grep -c ^processor /proc/cpuinfo)

# Do we have our env vars set correctly? Also set defaults if notn
if [ -z ${LEDE_REPO+x} ]; then
        LEDE_REPO="https://github.com/lede-project/source.git"
fi
if [ -z ${LEDE_BRANCH+x} ]; then
        LEDE_BRANCH="master"
fi
if [ -z ${LEDE_TARGET+x} ]; then
        echo "Error, LEDE_TARGET is not set!"
        exit 1
fi

# Clone down our repo, and cd into it
git clone $LEDE_REPO -b $LEDE_BRANCH ./source > /dev/null
cd ./source

# Add Socketman
echo "src-git cucumber http://github.com/cucumber-tony/cucumber-feeds.git" >> ./feeds.conf.default

# Update feeds
./scripts/feeds update -a
./scripts/feeds install -a

# Configure our target and enable building of socketman
echo "CONFIG_TARGET_${LEDE_TARGET}=y" > ./.config
echo "CONFIG_PACKAGE_socketman=m" >> ./.config
make defconfig

# Let's Go!
make -j$CPUCOUNT V=s 2>&1 | tee /builddir/save/build.log

# Did we fail? We check by seeing if we generated sha256sums.
if [ "$(find ./bin/targets/ | grep sha256sums | wc -l)" -ne "0" ]; then
  # If we are here, we are done so move our package
        find ./bin -iname 'socketman_*.ipk' | xargs -i cp {} /builddir/save/
        echo "Build Complete!"
  exit 0
else
  echo "Build Failed!"
  exit 1
fi
