* THIS IS A FORK.

Esse launcher tem o foco em especificamente melhorar o suporte da versão 0.15.10 no mesmo.
=========================================================================================

** Features
 - [x] Win10 UI
 - [ ] Resize Window
 - [x] Native wayland client

** tmp how to build wayland
-->  cmake -B build -S . -D GAMEWINDOW_SYSTEM=GLFW -D GLFW_USE_WAYLAND=1  -DCMAKE_TOOLCHAIN_FILE=cmake/linux32.cmake && cmake -j$(nproc) --build build

** Install 32 bit libraries (Fedora) Unverified.
sudo dnf install -y glibc-devel.i686 glibc-devel libpng-devel.i686  libstdc++-devel.i686 libevdev-devel.i686 libcurl-devel.i686 mesa-libEGL-devel.i686 systemd-devel.i686  gcc-c++ clang cmake make git ca-certificates libstdc++ glibc-devel libpng-devel zlib-devel libX11-devel libXi-devel libcurl-devel systemd-devel libevdev-devel mesa-libEGL-devel alsa-lib pulseaudio-libs mesa-dri-drivers systemd-devel libXtst-devel openssl-devel qt5-qtbase-devel qt5-qtwebengine-devel qt5-qtdeclarative-devel qt5-qtsvg-devel qt5-qtquickcontrols qt5-qtquickcontrols2
