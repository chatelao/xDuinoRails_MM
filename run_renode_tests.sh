#!/bin/bash
set -e

# Absolute path to the directory containing this script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR"
RENODE_DIR="${PROJECT_ROOT}/test/renode-bin"

# 1. Install Renode (portable) if not present
RENODE_VERSION="1.16.1"
RENODE_TAR="renode-${RENODE_VERSION}.linux-portable.tar.gz"
RENODE_URL="https://github.com/renode/renode/releases/download/v${RENODE_VERSION}/${RENODE_TAR}"

if [ ! -d "$RENODE_DIR" ]; then
    echo "Downloading and installing Renode ${RENODE_VERSION}..."
    wget -q "$RENODE_URL"
    mkdir -p "$RENODE_DIR"
    tar -xzf "$RENODE_TAR" -C "$RENODE_DIR" --strip-components=1
    rm "$RENODE_TAR"
else
    echo "Renode already installed in $RENODE_DIR"
fi

# 2. Install dependencies (Robot Framework, etc.)
echo "Installing Python dependencies..."
# We use the Renode portable's requirements to ensure compatibility
pip install -q -r "${RENODE_DIR}/tests/requirements.txt"
pip install -q psutil pyyaml

# 3. Build firmware
echo "Building firmware for Seeed XIAO RP2040..."
pio run -e seeed_xiao_rp2040

# 4. Run Renode Robot tests
echo "Running Renode Robot tests..."
export PATH="${RENODE_DIR}:${PATH}"

# We use renode-test which is a wrapper for Robot Framework in Renode
renode-test "${PROJECT_ROOT}/test/renode/tests/basic_uart.robot"
renode-test "${PROJECT_ROOT}/test/renode/tests/motor_telemetry.robot"

echo "Renode tests completed successfully!"
