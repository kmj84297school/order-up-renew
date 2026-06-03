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

## 중요: 게임 데이터(OBB 확장 파일)가 반드시 필요합니다

이 게임의 실제 콘텐츠(그래픽/오디오/레벨)는 APK가 아니라 **OBB 확장 파일** 2개에 들어 있습니다.
APK만 설치하면 첫 화면 "Order Up!! To Go"에서
**"Download failed because the resources could not be found"** (0%)로 멈춥니다.
원래는 구글 플레이가 OBB를 내려받지만, 사이드로딩이라 라이선스 검증이 실패해 다운로드가 안 됩니다.

필요한 OBB (정품 XAPK `Order Up To Go_v1.65` 안에 포함, **버전은 13**):

| 파일 | 크기 |
|---|---|
| `main.13.com.svsgames.orderup.obb` | 86,803,648 bytes |
| `patch.13.com.svsgames.orderup.obb` | 79,173,735 bytes |

> 이 OBB들은 APK가 기대하는 값과 **정확히 일치**하므로, 올바른 경로에 두기만 하면
> 앱이 라이선스/다운로드를 건너뛰고 바로 게임으로 진입합니다. **추가 코드 패치 불필요.**

## 설치 방법 (둘 중 택1)

### 경로 A (권장): XAPK 인스톨러로 한 번에 설치
정품 XAPK 안의 APK(targetSdk=21, 설치 차단됨)만 이 저장소의 sdk24 APK로 교체한 새 XAPK를 만들어,
APK 설치 + OBB 자동 배치를 한 번에 처리합니다.

```bash
# PC에서 (Python 필요). OBB는 그대로 보존되고 내부 APK만 교체됩니다.
python3 repack_xapk.py "Order Up To Go_v1.65_apkfab.com.xapk" \
    OrderUp-1.65-sdk24-signed.apk OrderUp-1.65-s23.xapk
```

생성된 `OrderUp-1.65-s23.xapk`를 휴대폰에 옮긴 뒤 **SAI(Split APKs Installer)** 같은
XAPK 인스톨러 앱으로 설치하면 끝입니다.

### 경로 B: APK 설치 + OBB 수동 배치
1. `OrderUp-1.65-sdk24-signed.apk` 설치 (`adb install ...` 또는 휴대폰에서 직접).
2. 정품 XAPK는 ZIP이므로 압축을 풀어 OBB 2개를 꺼냅니다.
3. 두 파일을 **원래 이름 그대로** 아래 경로에 넣습니다(폴더 없으면 생성):
   ```
   /내장메모리/Android/obb/com.svsgames.orderup/main.13.com.svsgames.orderup.obb
   /내장메모리/Android/obb/com.svsgames.orderup/patch.13.com.svsgames.orderup.obb
   ```
   > Android 11+ 부터 `Android/obb`에 다른 앱 파일을 쓰는 것이 제한됩니다.
   > 파일관리자에 "모든 파일 접근" 권한을 주거나, 가능하면 경로 A(인스톨러)를 사용하세요.
4. 게임 실행 → 다운로드 화면을 건너뛰고 시작됩니다.

## 직접 다시 빌드하려면 (APK targetSdk 패치)

```bash
sudo apt-get install -y zipalign apksigner default-jdk
./build.sh <원본.apk> 24
```

- `patch_apk.py` — 매니페스트 `targetSdkVersion` 패치 + 구 서명 제거 후 재패키징(압축 방식 보존).
- `build.sh` — 패치 → `zipalign` → `apksigner` 재서명 → 검증 전 과정.
- `repack_xapk.py` — 정품 XAPK 안의 베이스 APK만 sdk24 APK로 교체(OBB 보존 + manifest 갱신).
- `orderup.keystore` — 재서명에 사용한 자체 서명 키(비밀번호 `orderup`). 향후 업데이트 시
  동일 키로 서명하면 덮어쓰기 설치가 됩니다.

## 한계 / 주의

- **OBB가 없으면 게임은 실행되지 않습니다.** OBB는 게임 데이터이므로 코드 수정으로 만들 수 없으며,
  정품 XAPK/백업 등에서 확보해야 합니다.
- 이 리패키징은 **설치 차단(API 레벨) 문제만** APK 측에서 해결합니다. 2015년 당시의
  구형 Google Play Games / 인앱결제 SDK가 실행 중 오류를 낼 가능성은 남아 있지만,
  게임 코어 플레이에는 보통 영향이 없습니다.
- 개인 사용 목적의 호환성 패치입니다.
