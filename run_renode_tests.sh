#!/bin/bash
set -e

# Configuration
RENODE_VERSION="1.16.1"
RENODE_DIR="test/renode-bin"
RENODE_URL="https://github.com/renode/renode/releases/download/v${RENODE_VERSION}/renode-${RENODE_VERSION}.linux-portable.tar.gz"

# 1. Download Renode if not present
if [ ! -d "$RENODE_DIR" ]; then
    echo "Downloading Renode v${RENODE_VERSION}..."
    mkdir -p "$RENODE_DIR"
    curl -L "$RENODE_URL" | tar -xz -C "$RENODE_DIR" --strip-components=1
fi

export PATH="$PWD/$RENODE_DIR:$PATH"

# 2. Build firmware for Renode environment
echo "Building firmware for Renode..."
pio run -e seeed_xiao_rp2040_renode

# 3. Run Robot Framework tests
echo "Running Renode simulation tests..."
# Renode's robot runner is usually called 'renode-test'
# We might need to install robotframework if not present in the environment
pip install robotframework==6.1

renode-test test/renode/tests/cli_tests.robot
