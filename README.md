Esse launcher tem o foco em especificamente melhorar o suporte da versão 0.15.10 e da 1.1 no Linux.
=========================================================================================

** Features
 - [x] Win10 UI (0.15.10)
 - [x] Bugfix 1.1 Storage
 - [x] A F3 Menu
 - [x] Smooth Window Resize
 - [x] Native wayland client

** How to Build
```shell
cmake -B build -S . -D GAMEWINDOW_SYSTEM=GLFW -D GLFW_USE_WAYLAND=1  -DCMAKE_TOOLCHAIN_FILE=cmake/linux32.cmake && cmake --build build -j$(nproc)
```

** Dependencies 
```shell
sudo dnf install -y glibc-devel.i686 glibc-devel libpng-devel.i686  libstdc++-devel.i686 libevdev-devel.i686 libcurl-devel.i686 mesa-libEGL-devel.i686 systemd-devel.i686  gcc-c++ clang cmake make git ca-certificates libstdc++ glibc-devel libpng-devel zlib-devel libX11-devel libXi-devel libcurl-devel systemd-devel libevdev-devel mesa-libEGL-devel alsa-lib pulseaudio-libs mesa-dri-drivers systemd-devel libXtst-devel openssl-devel qt5-qtbase-devel qt5-qtwebengine-devel qt5-qtdeclarative-devel qt5-qtsvg-devel qt5-qtquickcontrols qt5-qtquickcontrols2 wayland-devel.i686 libxkbcommon-devel.i686 wayland-protocols-devel extra-cmake-modules freeglut-devel 
```
