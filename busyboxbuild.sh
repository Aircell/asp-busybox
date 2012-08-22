#!/bin/bash -eux

# This cross-build uses the Codesourcery tools.
# If you prefer other tools or install them elsewhere
# point at them here.

tools=/usr/share/codesourcery/Sourcery_G++_Lite/bin
if ![ $(which arm-none-linux-gnueabi-gcc) ]; then
  cat <<< "
    This cross-build uses the Codesourcery tools,
    which I install in $tools.
    If you install them in another place, or use other tools,
    please put them in your PATH 
  "

export PATH+=:$tools

# cleanliness is next to godliness
git clean -dfx

# The next commands are pulled right from the INSTALL instructions,
# with arguments added to the make command line to force a cross-build
makeargs='ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi-'
make $makeargs defconfig
sed -e 's/.*FEATURE_PREFER_APPLETS.*/CONFIG_FEATURE_PREFER_APPLETS=y/' -i .config
sed -e 's/.*FEATURE_SH_STANDALONE.*/CONFIG_FEATURE_SH_STANDALONE=y/' -i .config
make $makeargs
/home/jsh/workspace/orr/codesourcery/Sourcery_G++_Lite/bin/arm-none-linux-gnueabi-gcc-4.3.3
/home/jsh/workspace/orr/codesourcery/Sourcery_G++_Lite/share/doc/arm-arm-none-linux-gnueabi/man/man1/arm-none-linux-gnueabi-gcc.1
