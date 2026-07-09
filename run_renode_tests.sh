#!/bin/bash
set -e

# Configuration
RENODE_VERSION="1.16.1"
RENODE_DIR="./test/renode-bin"
RENODE_TAR="renode-${RENODE_VERSION}.linux-portable.tar.gz"
RENODE_URL="https://github.com/renode/renode/releases/download/v${RENODE_VERSION}/${RENODE_TAR}"

# 1. Install Renode if not present
if [ ! -d "$RENODE_DIR" ] || [ ! -f "$RENODE_DIR/renode" ]; then
    echo "Installing Renode ${RENODE_VERSION}..."
    mkdir -p "$RENODE_DIR"
    # Use wget with retries and follow redirects
    wget -q --show-progress --tries=3 "$RENODE_URL" -O "$RENODE_TAR"
    tar -xzf "$RENODE_TAR" -C "$RENODE_DIR" --strip-components=1
    rm "$RENODE_TAR"
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
# We use the full path to renode-test to be safe
"$PWD/$RENODE_DIR/renode-test" --results-dir robot_outputs test/renode/tests/basic_test.robot
