# Requirements
- Cross-Compiled Qt version installed on jetson
- Mounted sysroot
- Linaro gcc tools

# Command to configure cross compiled Qt5.13.2 version
## Note that it needs to be adjusted to your own environment

*./configure -release -opensource -nomake examples -nomake tests -opengl es2 -sysroot /mnt/sysroot -device-option CROSS_COMPILE=/opt/qt5jxavier/gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu- -prefix /usr/local/qt5 -confirm-license -skip qtgamepad -skip qtgraphicaleffects -skip qtlocation -skip qtmacextras -skip qtspeech -skip qtsvg -skip qtwayland -skip qtwebchannel -skip qtwebsockets -skip qtandroidextras -skip qtvirtualkeyboard -skip qtpurchasing -skip qtlottie -skip qtnetworkauth -skip qtwebglplugin -skip qtscript -xplatform linux-aarch64-gnu-g++ -make libs -pkg-config -no-use-gold-linker*

# Useful Links:
- https://sysprogs.com/w/fixing-rpath-link-issues-with-cross-compilers/
- https://forum.qt.io/topic/48556/libqxcb-so-not-found

# Known problems:
During compilation gcc may not see all of the necessary libaries, that's why it's need to adjust /etc/ld.so.conf.d on the jetson. Link how to do this is under Useful Links paragraph.