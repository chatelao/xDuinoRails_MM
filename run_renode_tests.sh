#!/bin/bash

# Configuration
RENODE_VERSION="1.16.1"
RENODE_DIR="test/renode-bin"
RENODE_PKG="renode-${RENODE_VERSION}.linux-portable.tar.gz"
RENODE_URL="https://github.com/renode/renode/releases/download/v${RENODE_VERSION}/${RENODE_PKG}"

# Setup Renode
if [ ! -d "$RENODE_DIR" ]; then
    echo "Downloading Renode v${RENODE_VERSION}..."
    mkdir -p "$RENODE_DIR"
    wget -q "$RENODE_URL" -O "${RENODE_DIR}/${RENODE_PKG}"
    tar -xzf "${RENODE_DIR}/${RENODE_PKG}" -C "$RENODE_DIR" --strip-components=1
    rm "${RENODE_DIR}/${RENODE_PKG}"
fi

# Install dependencies for Robot Framework
echo "Installing Python dependencies..."
pip install -q robotframework psutil pyyaml

# If in CI, install system dependencies
if [ "$GITHUB_ACTIONS" == "true" ]; then
    echo "Installing system dependencies for Renode..."
    # Using || true for apt-get update to tolerate transient issues with unrelated repos
    sudo apt-get update -y || true
    sudo apt-get install -y -q libgtk2.0-0 libglib2.0-0 libfontconfig1 libx11-6 libxcursor1 libxext6 libxi6 libxrender1 libxtst6
fi

# Run tests
echo "Running Renode simulation tests..."
export PATH="$PATH:$(pwd)/$RENODE_DIR"
./$RENODE_DIR/renode-test test/renode/decoder_test.robot
TEST_EXIT_CODE=$?

# Print logs if failed
if [ $TEST_EXIT_CODE -ne 0 ]; then
    echo "Tests failed! Printing logs..."
    if [ -d "logs" ]; then
        for log_file in logs/*.log; do
            echo "--- $log_file ---"
            cat "$log_file"
        done
    fi
fi

exit $TEST_EXIT_CODE
