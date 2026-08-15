#!/usr/bin/env bash
# build-demo-apk.sh — Linux equivalent of build-demo-apk.bat
# Run from the android/ directory or from the project root.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

ANDROID_TOOLS="$PROJECT_ROOT/.android-tools"
export JAVA_HOME="$ANDROID_TOOLS/jdk/jdk-17.0.20+8"
export ANDROID_HOME="$ANDROID_TOOLS/sdk"
export GRADLE_USER_HOME="$ANDROID_TOOLS/gradle-home"
export PATH="$JAVA_HOME/bin:$ANDROID_HOME/platform-tools:$PATH"

if [[ ! -x "$JAVA_HOME/bin/java" ]]; then
    echo "ERROR: JDK not found at $JAVA_HOME"
    echo "Run: bash $PROJECT_ROOT/tools/setup-linux-jdk.sh"
    exit 1
fi

echo "Java: $("$JAVA_HOME/bin/java" -version 2>&1 | head -1)"
echo "Building APK..."

"$SCRIPT_DIR/gradlew" -p "$SCRIPT_DIR" :app:assembleRelease

SIGNING_DIR="$SCRIPT_DIR/.signing"
KEYSTORE="$SIGNING_DIR/hanhan-demo.jks"
DIST_DIR="$PROJECT_ROOT/dist"

mkdir -p "$SIGNING_DIR" "$DIST_DIR"

if [[ ! -f "$KEYSTORE" ]]; then
    echo "Generating debug keystore..."
    "$JAVA_HOME/bin/keytool" -genkeypair \
        -keystore "$KEYSTORE" \
        -storepass android -keypass android \
        -alias hanhan-demo \
        -keyalg RSA -keysize 2048 -validity 10000 \
        -dname "CN=Ha Nhan Demo,O=Local Development,C=VN"
fi

UNSIGNED_APK="$SCRIPT_DIR/app/build/outputs/apk/release/app-release-unsigned.apk"
SIGNED_APK="$DIST_DIR/ha-nhan-demo-arm64.apk"

"$ANDROID_HOME/build-tools/35.0.0/apksigner" sign \
    --ks "$KEYSTORE" \
    --ks-key-alias hanhan-demo \
    --ks-pass pass:android \
    --key-pass pass:android \
    --out "$SIGNED_APK" \
    "$UNSIGNED_APK"

"$ANDROID_HOME/build-tools/35.0.0/apksigner" verify --verbose --print-certs "$SIGNED_APK"

echo ""
echo "APK: $SIGNED_APK"
