#!/bin/bash
set -e

# Configuration
RENODE_VERSION="1.16.1"
RENODE_DIR="./test/renode-bin"
RENODE_TAR="renode-${RENODE_VERSION}.linux-portable.tar.gz"
RENODE_URL="https://github.com/renode/renode/releases/download/v${RENODE_VERSION}/${RENODE_TAR}"

# 1. Install Renode if not present
if [ ! -d "$RENODE_DIR" ]; then
    echo "Installing Renode ${RENODE_VERSION}..."
    wget "$RENODE_URL"
    mkdir -p "$RENODE_DIR"
    tar -xzf "$RENODE_TAR" -C "$RENODE_DIR" --strip-components=1
    rm "$RENODE_TAR"
else
    echo "Renode already installed in $RENODE_DIR"
fi

# 2. Install Python dependencies
echo "Installing Python dependencies..."
pip install -r test/renode/tests/requirements.txt --quiet

# 3. Build firmware
echo "Building firmware for Seeed XIAO RP2040 (Renode compatible)..."
pio run -e seeed_xiao_rp2040_renode

# 4. Run Renode tests
echo "Running Renode simulation tests..."
ABS_ELF_PATH="$(pwd)/.pio/build/seeed_xiao_rp2040_renode/firmware.elf"
PYTHONPATH=$RENODE_DIR/tests $RENODE_DIR/renode-test \
    --variable RENODEKEYWORDS:$RENODE_DIR/tests/renode-keywords.robot \
    --variable ELF_FILE_VAR:"$ABS_ELF_PATH" \
    test/renode/tests/test_decoder.robot
