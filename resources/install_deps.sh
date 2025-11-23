#!/bin/bash
# Dependency install script for docgen
# Usage: bash resources/install_deps.sh

set -e

# System build tools
sudo apt-get update
sudo apt-get install -y build-essential cmake pkg-config

# GTK3 and WebKit2GTK
sudo apt-get install -y libgtk-3-dev libwebkit2gtk-4.1-dev

# Optional: Google Test (for tests)
# This is fetched automatically by CMake, but you can install system version if needed
# sudo apt-get install -y libgtest-dev

# Optional: lcov/genhtml for coverage
sudo apt-get install -y lcov
sudo apt-get install -y genhtml

echo "All dependencies installed. You can now run cmake and make to build docgen."
