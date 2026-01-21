#!/bin/bash
set -euo pipefail

VERSION=$1
QT_VERSION=6.9.1
AUTHOR=Calebh101

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PARENT_DIR="$(dirname "$SCRIPT_DIR")"
HELPER_DIR=$PARENT_DIR/linux-helper
OUTPUT_DIR=$PARENT_DIR/Output

APP_BUILD=$SCRIPT_DIR/build/Desktop_Qt_6_9_1-Release
HELPER_BUILD=$HELPER_DIR/build/Desktop_Qt_6_9_1-Release

if [ "$(uname -s)" != "Linux" ]; then
  echo "This script must be run on Linux."
  exit 1
fi

if [ -z "$VERSION" ]; then
  echo "Usage: $0 <version>"
  exit 1
fi

echo "Building AboutThisPC $VERSION by $AUTHOR..."
cd "$HELPER_DIR"

cmake -S $HELPER_DIR -B $HELPER_BUILD -DCMAKE_PREFIX_PATH=$HOME/Qt/$QT_VERSION/gcc_64
cmake --build $HELPER_BUILD --target all
cp $HELPER_BUILD/linux-helper $SCRIPT_DIR/binaries/linux-helper

cd "$SCRIPT_DIR"
cmake -S $SCRIPT_DIR -B $APP_BUILD -DCMAKE_PREFIX_PATH=$HOME/Qt/$QT_VERSION/gcc_64
cmake --build $APP_BUILD --target all

cd $PARENT_DIR
mkdir -p Output

echo "Processing Linux output..."
rm -rf $OUTPUT_DIR/linux
mkdir -p $OUTPUT_DIR/linux
cd $OUTPUT_DIR/linux

mkdir -p x64
cp $APP_BUILD/AboutThisPC x64/AboutThisPC
cp $PARENT_DIR/README.md x64/README.md
cp $PARENT_DIR/LICENSE.md x64/LICENSE.md
cp $PARENT_DIR/SECURITY.md x64/SECURITY.md
cp $PARENT_DIR/CONTRIBUTING.md x64/CONTRIBUTING.md
cp $PARENT_DIR/CODE_OF_CONDUCT.md x64/CODE_OF_CONDUCT.md
cp $PARENT_DIR/INSTALLING.md x64/INSTALLING.md
sed -e "s/\[\[APPVERSION\]\]/$VERSION/g" -e "s/\[\[AUTHOR\]\]/$AUTHOR/g" "$SCRIPT_DIR/Sample.desktop" > x64/AboutThisPC.desktop

x64zip=$OUTPUT_DIR/AboutThisPC-$VERSION-linux-x64.zip
rm -f $x64zip
cd x64
zip -r "$x64zip" .

cd $OUTPUT_DIR
echo "Application built!"