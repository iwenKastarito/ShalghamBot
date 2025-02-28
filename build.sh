#!/bin/bash

# Install SFML if not installed
if ! pkg-config --exists sfml-all; then
    echo "SFML not found, installing..."
    if [ -f /etc/arch-release ]; then
        # Arch Linux
        sudo pacman -S --needed sfml
    elif [ -f /etc/debian_version ]; then
        # Debian/Ubuntu
        sudo apt-get update
        sudo apt-get install -y libsfml-dev
    elif [ -f /etc/redhat-release ]; then
        # Fedora/RHEL/CentOS
        sudo dnf install -y SFML-devel
    elif [ -f /etc/os-release ] && grep -q "ID=opensuse" /etc/os-release; then
        # openSUSE
        sudo zypper install -y SFML-devel
    elif [ "$(uname)" == "Darwin" ]; then
        # macOS
        brew install sfml
    else
        echo "Unsupported distribution. Please install SFML manually."
        exit 1
    fi
fi

# Create build directory
mkdir -p build
cd build

# Configure and build
cmake ..
cmake --build . -- -j$(nproc)

echo "Build complete! Run with: ./build/chess" 