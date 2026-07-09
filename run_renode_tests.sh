#!/bin/bash
set -e

# Configuration
RENODE_VERSION="1.16.1"
RENODE_DIR="./test/renode-bin"
RENODE_URL="https://builds.antmicro.com/renode/builds/renode-${RENODE_VERSION}.linux-portable.tar.gz"

# 1. Install Renode if not present
if [ ! -d "$RENODE_DIR" ] || [ ! -f "$RENODE_DIR/renode" ]; then
    echo "Installing Renode ${RENODE_VERSION}..."
    mkdir -p "$RENODE_DIR"
    curl -L "$RENODE_URL" | tar -xz -C "$RENODE_DIR" --strip-components=1
fi

export PATH="$PWD/$RENODE_DIR:$PATH"

# 2. Build firmware if not present
if [ ! -f ".pio/build/seeed_xiao_rp2040/firmware.elf" ]; then
    echo "Building firmware for seeed_xiao_rp2040..."
    pio run -e seeed_xiao_rp2040
fi

# 3. Install Robot Framework dependencies
echo "Installing Robot Framework and dependencies..."
pip install robotframework psutil pyyaml

# 4. Run tests
echo "Running Renode tests..."
mkdir -p robot_outputs

# Use renode-test from the portable install
# We use --variable to pass the firmware path if needed, but it's currently hardcoded in RESC relative to origin
renode-test --results-dir robot_outputs test/renode/tests/basic_test.robot
