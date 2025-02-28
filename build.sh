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

# Check for Qt5 and install if needed
if ! pkg-config --exists Qt5Core Qt5Widgets Qt5Gui; then
    echo "Qt5 not found, installing..."
    if [ -f /etc/arch-release ]; then
        # Arch Linux
        sudo pacman -S --needed qt5-base
    elif [ -f /etc/debian_version ]; then
        # Debian/Ubuntu
        sudo apt-get update
        sudo apt-get install -y qtbase5-dev
    elif [ -f /etc/redhat-release ]; then
        # Fedora/RHEL/CentOS
        sudo dnf install -y qt5-qtbase-devel
    elif [ -f /etc/os-release ] && grep -q "ID=opensuse" /etc/os-release; then
        # openSUSE
        sudo zypper install -y libqt5-qtbase-devel
    elif [ "$(uname)" == "Darwin" ]; then
        # macOS
        brew install qt@5
    else
        echo "Unsupported distribution. Please install Qt manually if you want to build the Qt version."
    fi
fi

# Create build directory
mkdir -p build
cd build

# Configure and build
cmake ..
cmake --build . -- -j$(nproc)

echo "Build complete!"
echo "Run SFML version with: ./build/chess"

# Check if Qt version was built
if [ -f chess ]; then
    echo "Run Qt version with: ./build/qtchess"
fi 