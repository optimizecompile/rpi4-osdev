#!/bin/bash
# Rebuild the patched uiohook-napi native module and install it into ./node_modules.
# Run from this directory AFTER `npm install` - a fresh npm install restores the
# stock prebuild, which freezes on the first mouse click (see the part12 README).
# Requires: git, node/npm, Xcode command line tools. Builds for Apple Silicon (arm64).
set -euo pipefail

HERE="$(cd "$(dirname "$0")" && pwd)"
if [ ! -d "$HERE/node_modules/uiohook-napi" ]; then
  echo "node_modules/uiohook-napi not found - run 'npm install' first" >&2
  exit 1
fi

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

echo "Cloning uiohook-napi v1.5.5..."
git clone --quiet --recurse-submodules https://github.com/SnosMe/uiohook-napi.git "$WORK/uiohook-napi"
cd "$WORK/uiohook-napi"

echo "Applying patches..."
npm run apply-libuiohook-patch          # the repo's own Windows/X11 patch (their CI step)
git -C libuiohook apply "$HERE/uiohook-darwin-event-tap.patch"   # our macOS click-freeze fix

echo "Building..."
npm ci --ignore-scripts
npm run prebuild -- --arch arm64

echo "Installing into node_modules..."
cp prebuilds/darwin-arm64/uiohook-napi.node \
   "$HERE/node_modules/uiohook-napi/prebuilds/darwin-arm64/uiohook-napi.node"

echo "Done - patched uiohook-napi installed."
