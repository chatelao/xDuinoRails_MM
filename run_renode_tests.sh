#!/bin/bash
set -e

# Configuration
RENODE_VERSION="1.16.1"
RENODE_DIR="test/renode-bin"
# Use the simulation-specific binary
FIRMWARE_ELF=".pio/build/seeed_xiao_rp2040_renode/firmware.elf"

# 1. Download and install Renode (portable)
if [ ! -d "$RENODE_DIR" ]; then
    echo "Downloading Renode $RENODE_VERSION..."
    mkdir -p "$RENODE_DIR"
    curl -L "https://github.com/renode/renode/releases/download/v${RENODE_VERSION}/renode-${RENODE_VERSION}.linux-portable.tar.gz" | tar -xz -C "$RENODE_DIR" --strip-components=1
fi

# 2. Install compatible dependencies
echo "Installing dependencies..."
# Pin robotframework to a version known to work with Renode 1.16.1
pip install -q "robotframework==6.1.1" psutil pyyaml

export PATH="$PWD/$RENODE_DIR:$PATH"
export PYTHONPATH="$PWD/$RENODE_DIR/tests:$PYTHONPATH"

# 3. Build firmware if not present
if [ ! -f "$FIRMWARE_ELF" ]; then
    echo "Building firmware..."
    pio run -e seeed_xiao_rp2040_renode
fi

# 4. Run Renode tests
echo "Running Renode tests..."
renode-test test/renode/tests/cli_tests.robot
