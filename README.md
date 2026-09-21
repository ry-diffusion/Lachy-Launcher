# Lachy-Launcher

A Linux launcher for Minecraft Pocket Edition / Bedrock Edition, with a focus on improved support for legacy versions (0.15.10) and version 1.1.

## Features

- ✅ Win10 UI support (0.15.10)
- ✅ Bugfix for 1.1 Storage issues
- ✅ F3 Debug Menu
- ✅ Smooth Window Resize
- ✅ Native Wayland client support
- ✅ Asset caching for improved performance
- ✅ Automatic FMOD audio library patching

## Quick Start

```bash
# Build the launcher
cmake -B build -S . -DGAMEWINDOW_SYSTEM=GLFW -DGLFW_BUILD_WAYLAND=1 \
    -DCMAKE_TOOLCHAIN_FILE=cmake/linux32.cmake -DCMAKE_POLICY_VERSION_MINIMUM=3.5
cmake --build build -j$(nproc)

# Run the game
./build/mcpelauncher-client/mcpelauncher-client --game-dir /path/to/minecraft/
```

## Command Line Options

### Basic Options
| Option | Short | Description |
|--------|-------|-------------|
| `--help` | `-h` | Show help information |
| `--version` | `-v` | Print version info |

### Directory Options
| Option | Short | Description |
|--------|-------|-------------|
| `--game-dir <path>` | `-dg` | Directory containing game files and assets |
| `--data-dir <path>` | `-dd` | Directory for game data storage |
| `--cache-dir <path>` | `-dc` | Directory for cache files |

### Window Options
| Option | Short | Description | Default |
|--------|-------|-------------|---------|
| `--width <pixels>` | `-ww` | Window width | 720 |
| `--height <pixels>` | `-wh` | Window height | 480 |
| `--scale <factor>` | `-s` | Pixel scale factor | 2.0 |

### Audio Options
| Option | Short | Description |
|--------|-------|-------------|
| `--disable-fmod` | `-df` | Disable FMOD audio (no sound) |

### Performance Options
| Option | Short | Description |
|--------|-------|-------------|
| `--disable-cache` | `-nc` | Disable asset caching |
| `--malloc-zero` | `-mz` | Zero-initialize all malloc memory (debug) |

### Developer/Debug Options
| Option | Short | Description |
|--------|-------|-------------|
| `--dry-run` | `-n` | Validate setup and exit without launching |
| `--verbose` | `-V` | Enable verbose logging |
| `--quiet` | `-q` | Suppress most log output |
| `--save-config` | `-sc` | Save current settings to config file |

## Configuration File

Settings are stored in `~/.config/mcpelauncher/config.properties`. The launcher automatically:
- Loads settings on startup
- Remembers the last used game directory
- Saves the game directory after each launch

### Config File Format
```properties
# Lachy-Launcher Configuration
window.width=720
window.height=480
window.scale=2.0
audio.disable_fmod=false
performance.disable_cache=false
log.level=1
game.last_dir=/path/to/game/
```

### Saving Settings
Use `--save-config` to save current CLI options to the config file:
```bash
./mcpelauncher-client -dg /path/to/game -ww 1280 -wh 720 --disable-fmod --save-config
```

## Examples

```bash
# Basic launch (uses last game dir from config)
./mcpelauncher-client

# Launch with specific game directory
./mcpelauncher-client -dg ~/.local/share/mcpelauncher/versions/0.15.10.0/

# Launch with custom window size
./mcpelauncher-client -dg /path/to/game -ww 1280 -wh 720

# Validate setup without launching (dry-run)
./mcpelauncher-client -dg /path/to/game --dry-run

# Launch with verbose logging
./mcpelauncher-client -dg /path/to/game --verbose

# Launch without audio
./mcpelauncher-client -dg /path/to/game --disable-fmod
```

## Building from Source

### Dependencies (Fedora/RHEL)

```bash
sudo dnf install -y \
    glibc-devel.i686 glibc-devel libpng-devel.i686 libstdc++-devel.i686 \
    libevdev-devel.i686 libcurl-devel.i686 mesa-libEGL-devel.i686 \
    systemd-devel.i686 gcc-c++ clang cmake make git ca-certificates \
    libstdc++ libpng-devel zlib-devel libX11-devel libXi-devel \
    libcurl-devel systemd-devel libevdev-devel mesa-libEGL-devel \
    alsa-lib pulseaudio-libs mesa-dri-drivers libXtst-devel \
    openssl-devel wayland-devel.i686 libxkbcommon-devel.i686 \
    wayland-protocols-devel extra-cmake-modules freeglut-devel \
    libXrandr-devel.i686 libXinerama-devel.i686 libXcursor-devel.i686
```

### Dependencies (Arch Linux)

```bash
sudo pacman -S --needed \
    lib32-glibc lib32-libpng lib32-libstdc++5 lib32-libevdev \
    lib32-curl lib32-mesa lib32-systemd-libs gcc cmake make git \
    libpng zlib libx11 libxi curl systemd libevdev mesa \
    alsa-lib pulseaudio libxtst openssl wayland libxkbcommon \
    wayland-protocols extra-cmake-modules freeglut libxrandr \
    libxinerama libxcursor
```

### Build Commands

```bash
# Standard build
cmake -B build -S . \
    -DGAMEWINDOW_SYSTEM=GLFW \
    -DGLFW_BUILD_WAYLAND=1 \
    -DCMAKE_TOOLCHAIN_FILE=cmake/linux32.cmake \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON

cmake --build build -j$(nproc)

# Release build with optimizations
cmake -B build -S . \
    -DGAMEWINDOW_SYSTEM=GLFW \
    -DGLFW_BUILD_WAYLAND=1 \
    -DCMAKE_TOOLCHAIN_FILE=cmake/linux32.cmake \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON

cmake --build build -j$(nproc)
```

## Troubleshooting

### FMOD Audio Issues

If you see an error like `cannot enable executable stack`, the launcher will automatically fall back to stub audio (no sound). To enable sound, ensure the `libfmod.so.9.16` library has been patched (this happens automatically during build).

### Missing Game Directory

Always specify the game directory with `--game-dir` or `-dg`:
```bash
./mcpelauncher-client -dg /path/to/minecraft/version/
```

### Validating Your Setup

Use dry-run mode to check if everything is configured correctly:
```bash
./mcpelauncher-client -dg /path/to/game --dry-run
```

## Architecture

```
Lachy-Launcher/
├── mcpelauncher-client/   # Main launcher client
├── mcpelauncher-core/     # Core launcher functionality
├── mcpelauncher-common/   # Shared utilities
├── libhybris/             # Android compatibility layer
├── libjnivm/              # JNI virtual machine
├── game-window/           # Window management (GLFW)
├── minecraft-symbols/     # Minecraft symbol definitions
└── cmake/                 # Build configuration
```

## License

See [LICENSE](LICENSE) for details.
