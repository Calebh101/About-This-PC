#!/bin/bash
set -euo pipefail

QT_VERSION=6.9.1
AUTHOR=Calebh101

VERSION=$1
BUILD_APPIMAGE=false

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PARENT_DIR="$(dirname "$SCRIPT_DIR")"
HELPER_DIR=$PARENT_DIR/linux-helper
OUTPUT_DIR=$PARENT_DIR/Output

APP_BUILD=$SCRIPT_DIR/build/Desktop_Qt_6_9_1-Release
HELPER_BUILD=$HELPER_DIR/build/Desktop_Qt_6_9_1-Release
APPIMAGE=$PARENT_DIR/Output/linux-AppImage
x64zip=$OUTPUT_DIR/AboutThisPC-$VERSION-linux-x64.zip

LINUXDEPLOY=linuxdeploy-x86_64.AppImage
LINUXDEPLOYQT=linuxdeploy-plugin-qt-x86_64.AppImage

if [ "$(uname -s)" != "Linux" ]; then
  echo "This script must be run on Linux."
  exit 1
fi

if [ -z "$VERSION" ]; then
  echo "Usage: $0 <version>"
  exit 1
fi

for arg in "$@"; do
  if [ "$arg" == "--appimage" ]; then
    BUILD_APPIMAGE=true
  fi
done

echo "Building AboutThisPC $VERSION by $AUTHOR..."
cd "$HELPER_DIR"

export PATH=$HOME/Qt/$QT_VERSION/gcc_64:$PATH
export LD_LIBRARY_PATH=$HOME/Qt/$QT_VERSION/gcc_64/lib:${LD_LIBRARY_PATH:-}

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
mkdir -p x64

if [ "$BUILD_APPIMAGE" = true ]; then
  echo "Packaging AppImage..."
  mkdir -p $APPIMAGE
  sed -e "s/\[\[APPVERSION\]\]/$VERSION/g" -e "s/\[\[AUTHOR\]\]/$AUTHOR/g" "$SCRIPT_DIR/Sample.desktop" > $APPIMAGE/AboutThisPC.desktop
  mkdir -p $APPIMAGE/usr/bin

  cp $APP_BUILD/AboutThisPC $APPIMAGE/usr/bin/AboutThisPC
  cp $SCRIPT_DIR/runner.sh $APPIMAGE/AppRun
  cp $SCRIPT_DIR/appicon.png $APPIMAGE/AboutThisPC.png

  chmod +x $APPIMAGE/AppRun

  if [ ! -f "$LINUXDEPLOY" ]; then
    echo "Downloading $LINUXDEPLOY..."
    wget https://github.com/linuxdeploy/linuxdeploy/releases/download/1-alpha-20251107-1/linuxdeploy-x86_64.AppImage
    chmod +x "$LINUXDEPLOY"
  fi

  if [ ! -f "$LINUXDEPLOYQT" ]; then
    echo "Downloading $LINUXDEPLOYQT..."
    wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/1-alpha-20250213-1/linuxdeploy-plugin-qt-x86_64.AppImage
    chmod +x "$LINUXDEPLOYQT"
  fi

  rm -rf $PARENT_DIR/About*This*PC-*.AppImage
  export QMAKE=$HOME/Qt/$QT_VERSION/gcc_64/bin/qmake
  "./$LINUXDEPLOY" --appdir $APPIMAGE --executable $APPIMAGE/usr/bin/AboutThisPC --plugin qt --output appimage --extra-plugins platforms
fi

mkdir -p $OUTPUT_DIR/linux/x64
cd $OUTPUT_DIR/linux

if [ "$BUILD_APPIMAGE" = true ] && ls $PARENT_DIR/About*This*PC-*.AppImage >/dev/null 2>&1; then
  APPIMAGE_FILE=$(ls $PARENT_DIR/About*This*PC-*.AppImage | head -n1)
  cp "$APPIMAGE_FILE" x64/AboutThisPC.AppImage
else
  cp "$APP_BUILD/AboutThisPC" x64/AboutThisPC
fi

cp $PARENT_DIR/README.md x64/README.md
cp $PARENT_DIR/LICENSE.md x64/LICENSE.md
cp $PARENT_DIR/SECURITY.md x64/SECURITY.md
cp $PARENT_DIR/CONTRIBUTING.md x64/CONTRIBUTING.md
cp $PARENT_DIR/CODE_OF_CONDUCT.md x64/CODE_OF_CONDUCT.md

rm -f $x64zip
cd x64
zip -r "$x64zip" .

cd $OUTPUT_DIR
echo "Application built!"