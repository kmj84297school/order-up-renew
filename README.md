# Order Up!! 호환성 리패키징 (갤럭시 S23 설치 가능화)

2015년 빌드된 요리 게임 **Order Up!!** (`com.svsgames.orderup`, v1.65)을
최신 갤럭시 S23(Android 14/15)에서 **설치 가능**하도록 최소 수정한 리패키징입니다.

## 무엇이 문제였나

| 항목 | 원본 | 비고 |
|---|---|---|
| `targetSdkVersion` | **21** (Android 5.0) | ← **설치 차단의 원인** |
| `minSdkVersion` | 9 | 그대로 |
| 네이티브 라이브러리 | `armeabi-v7a` (32비트) | 문제 아님 (아래 참고) |
| 서명 | v1(JAR) 단독 | 수정 시 깨짐 → 재서명 필요 |

- **Android 14부터 `targetSdkVersion < 23` 앱은 설치 자체가 차단**됩니다
  (`INSTALL_FAILED_DEPRECATED_SDK_VERSION`). Android 15는 `< 24`를 차단.
  → 그래서 갤럭시 S23에 설치가 안 됐습니다.
- **32비트 라이브러리는 문제가 아닙니다.** 갤럭시 S23(SM8550)은
  `arm64-v8a, armeabi-v7a, armeabi`를 모두 지원하므로 32비트 게임도 정상 실행됩니다.

## 무엇을 바꿨나 (최소 침습)

- `AndroidManifest.xml`의 **`targetSdkVersion` 정수 한 개만 21 → 24** 로 변경.
  바이너리 매니페스트에서 해당 4바이트 정수만 교체하여 **파일 크기·오프셋·다른 모든 내용은 불변**.
- 변경으로 깨진 구 서명을 제거하고 **새 키로 v1/v2/v3 재서명**.
- 게임 코드(`classes.dex`), 리소스(`resources.arsc`), 네이티브 라이브러리(`libOrderUp.so`)는
  **일절 수정하지 않았습니다.**

결과물: **`OrderUp-1.65-sdk24-signed.apk`**

## 설치 방법

> 자체 서명 키로 재서명되었으므로 **원본 Play 스토어 버전과는 다른 서명**입니다.
> 기존에 설치된 Order Up!! 이 있다면 **먼저 제거**한 뒤 설치하세요.

1. 휴대폰에 APK 복사 후 파일 관리자에서 실행 → "출처를 알 수 없는 앱" 허용 후 설치.
2. 또는 PC에서 ADB:
   ```bash
   adb install OrderUp-1.65-sdk24-signed.apk
   ```

## 직접 다시 빌드하려면

```bash
sudo apt-get install -y zipalign apksigner default-jdk
./build.sh <원본.apk> 24
```

- `patch_apk.py` — 매니페스트 `targetSdkVersion` 패치 + 구 서명 제거 후 재패키징(압축 방식 보존).
- `build.sh` — 패치 → `zipalign` → `apksigner` 재서명 → 검증 전 과정.
- `orderup.keystore` — 재서명에 사용한 자체 서명 키(비밀번호 `orderup`). 향후 업데이트 시
  동일 키로 서명하면 덮어쓰기 설치가 됩니다.

## 한계 / 주의

- 이 리패키징은 **설치 차단(API 레벨) 문제만** 해결합니다. 설치는 결정론적으로 가능해지지만,
  2015년 당시의 **구형 Google Play Games / 인앱결제 SDK가 실행 중 오류**를 낼 가능성은 남아 있습니다.
  (게임 코어 플레이에는 보통 영향 없음.) 만약 실행 시 즉시 종료된다면 해당 SDK 초기화를
  비활성화하는 추가 작업이 필요할 수 있습니다.
- 개인 사용 목적의 호환성 패치입니다.
