#!/bin/bash
VERSION=0.0.0A
AUTHOR=Calebh101

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PARENT_DIR="$(dirname "$SCRIPT_DIR")"
HELPER_DIR=$PARENT_DIR/linux-helper
OUTPUT_DIR=$PARENT_DIR/Output

echo "Building AboutThisPC $VERSION by $AUTHOR..."
cd "$SCRIPT_DIR"

/usr/bin/cmake --build $HELPER_DIR/build/Desktop_Qt6_6_9_1-Release --target all
cp $HELPER_DIR/build/Desktop_Qt6_6_9_1-Release/linux-helper $SCRIPT_DIR/binaries/linux-helper
/usr/bin/cmake --build $SCRIPT_DIR/build/Desktop_Qt_6_9_1-Release --target all

cd $PARENT_DIR
mkdir -p Output

echo "Processing Linux output..."
rm -rf $OUTPUT_DIR/linux
mkdir -p $OUTPUT_DIR/linux
cd $OUTPUT_DIR/linux

mkdir -p x64
cp $SCRIPT_DIR/build/Desktop_Qt_6_9_1-Release/AboutThisPC x64/AboutThisPC
cp $PARENT_DIR/README.md x64/README.md
cp $PARENT_DIR/LICENSE.md x64/LICENSE.md
sed -e "s/\[\[APPVERSION\]\]/$VERSION/g" -e "s/\[\[AUTHOR\]\]/$AUTHOR/g" "$SCRIPT_DIR/Sample.desktop" > x64/AboutThisPC.desktop

x64zip=$OUTPUT_DIR/AboutThisPC-$VERSION-linux-x64.zip
rm -f $x64zip
cd x64
zip -r "$x64zip" .

cd $OUTPUT_DIR
echo "Application built!"