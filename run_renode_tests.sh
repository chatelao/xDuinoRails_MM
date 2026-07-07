#!/bin/bash
set -e

# Configuration
RENODE_VERSION="1.16.1"
RENODE_DIR="test/renode-bin"
RENODE_URL="https://github.com/renode/renode/releases/download/v${RENODE_VERSION}/renode-${RENODE_VERSION}.linux-portable.tar.gz"

# 1. Install dependencies
echo "Checking dependencies..."
pip install robotframework psutil --quiet

# 2. Download Renode if not present
if [ ! -f "$RENODE_DIR/renode" ]; then
    echo "Downloading Renode v${RENODE_VERSION}..."
    mkdir -p "$RENODE_DIR"
    curl -L "$RENODE_URL" | tar -xzC "$RENODE_DIR" --strip-components=1
fi

# 3. Build Firmware
echo "Building firmware..."
pio run -e seeed_xiao_rp2040

# 4. Run Tests
echo "Running Renode simulation tests..."
"$RENODE_DIR/renode-test" test/renode/basic_verification.robot
