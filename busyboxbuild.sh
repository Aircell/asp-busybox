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
fi

export PATH+=:$tools

# cleanliness is next to godliness
git clean -dfx

# The next commands are pulled right from the INSTALL instructions.
make defconfig
perl -i -pe '
  s/.*FEATURE_PREFER_APPLETS.*/CONFIG_FEATURE_PREFER_APPLETS=y/;
  s/.*FEATURE_SH_STANDALONE.*/CONFIG_FEATURE_SH_STANDALONE=y/;
  s/.*STATIC.*/CONFIG_STATIC=y/;
  s/.*INSTALL_NO_USR.*/CONFIG_INSTALL_NO_USR=y/;
  s/.*CROSS_COMPILER_PREFIX.*/CONFIG_CROSS_COMPILER_PREFIX="arm-none-linux-gnueabi-"/;
' .config
make
