#!/usr/bin/env bash
# Order Up!! APK 재패키징 빌드 스크립트
# 원본 APK -> targetSdkVersion 패치 -> 정렬 -> 재서명
#
# 필요 도구: python3, zipalign, apksigner, keytool (Java)
#   Ubuntu: sudo apt-get install -y zipalign apksigner default-jdk
#
# 사용법: ./build.sh <원본.apk> [target_sdk=24]
set -euo pipefail

SRC="${1:?원본 APK 경로를 인자로 주세요}"
TARGET="${2:-24}"
OUT="OrderUp-1.65-sdk${TARGET}-signed.apk"
KS="orderup.keystore"
KS_PASS="orderup"

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

echo "[1/4] AndroidManifest targetSdkVersion 패치 + 구 서명 제거"
python3 patch_apk.py "$SRC" "$WORK/unsigned.apk" "$TARGET"

echo "[2/4] zipalign"
zipalign -f -p 4 "$WORK/unsigned.apk" "$WORK/aligned.apk"

if [ ! -f "$KS" ]; then
  echo "[*] 키스토어 생성 ($KS)"
  keytool -genkeypair -keystore "$KS" -storepass "$KS_PASS" -keypass "$KS_PASS" \
    -alias orderup -keyalg RSA -keysize 2048 -validity 10000 \
    -dname "CN=OrderUp Repack, OU=Personal, O=Personal, L=NA, S=NA, C=NA"
fi

echo "[3/4] 재서명 (v1+v2+v3)"
apksigner sign --ks "$KS" --ks-pass "pass:$KS_PASS" --key-pass "pass:$KS_PASS" \
  --v1-signing-enabled true --v2-signing-enabled true --v3-signing-enabled true \
  --out "$OUT" "$WORK/aligned.apk"

echo "[4/4] 서명 검증"
apksigner verify --print-certs "$OUT" | head -6

echo "완료: $OUT"
