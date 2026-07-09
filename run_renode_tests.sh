#!/bin/bash
set -e

# Configuration
RENODE_VERSION="1.16.1"
RENODE_DIR="test/renode-bin"
FIRMWARE_ELF=".pio/build/seeed_xiao_rp2040/firmware.elf"

# 1. Download and install Renode (portable)
if [ ! -d "$RENODE_DIR" ]; then
    echo "Downloading Renode $RENODE_VERSION..."
    mkdir -p "$RENODE_DIR"
    curl -L "https://github.com/renode/renode/releases/download/v${RENODE_VERSION}/renode-${RENODE_VERSION}.linux-portable.tar.gz" | tar -xz -C "$RENODE_DIR" --strip-components=1
fi

# 2. Install dependencies from Renode's requirements
echo "Installing dependencies..."
pip install -q -r "$RENODE_DIR/tests/requirements.txt"

export PATH="$PWD/$RENODE_DIR:$PATH"

# 3. Build firmware if not present
if [ ! -f "$FIRMWARE_ELF" ]; then
    echo "Building firmware..."
    pio run -e seeed_xiao_rp2040
fi

# 4. Run Renode tests
echo "Running Renode tests..."
renode-test -v test/renode/tests/cli_tests.robot
