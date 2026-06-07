# 우리반 몰입 스터디 — Android WebView 래퍼 앱

기존 **Google Apps Script 웹앱**을 그대로 감싸 설치형 Android 앱(APK)으로 만든 프로젝트입니다.
화면·점수·서명·달력·랭킹·정지/재개 등 **모든 기능이 웹앱 그대로** 동작합니다.

## 동작 방식

- `MainActivity`가 전체 화면 `WebView`로 웹앱 주소(`app_url`)를 로드합니다.
- `JavaScript`와 `DOM Storage(localStorage)`를 켜서, 학번 **자동 로그인**과 서명 캔버스가 정상 동작합니다.
- 인터넷 연결이 필요합니다(웹앱이 서버에서 실행되므로).

## 웹앱 주소 바꾸기

`app/src/main/res/values/strings.xml` 의 `app_url` 한 줄만 수정하면 됩니다.

```xml
<string name="app_url">https://script.google.com/macros/s/.../exec</string>
```

> ⚠️ **중요:** WebView에서 로그인 없이 열리려면, Apps Script 배포 설정에서
> **액세스 권한을 "모든 사용자(Anyone)"** 로 두어야 합니다.
> (권한이 "본인만"이면 앱에서 403/로그인 화면이 뜹니다.)

## APK 빌드 방법

### 방법 1) GitHub Actions (권장 — 클릭 한 번)

이 저장소에는 `.github/workflows/build-apk.yml` 워크플로가 있습니다.

1. 변경사항을 `claude/brave-faraday-AqSkn` 브랜치에 push 하거나,
   GitHub → **Actions** 탭 → **Build Study App APK** → **Run workflow** 실행
2. 빌드가 끝나면 해당 실행 페이지 하단 **Artifacts → `molip-study-apk`** 다운로드
3. 압축을 풀면 `molip-study.apk` → 휴대폰에 설치

### 방법 2) Android Studio

1. Android Studio에서 `study-app/` 폴더 열기
2. `Build > Build App Bundle(s) / APK(s) > Build APK(s)`
3. 생성된 `app/build/outputs/apk/debug/app-debug.apk` 설치

### 방법 3) 명령줄 (로컬에 Android SDK 필요)

```bash
cd study-app
./gradlew assembleDebug
# 결과물: app/build/outputs/apk/debug/app-debug.apk
```

## 설치

휴대폰에 APK 복사 → 파일 관리자에서 실행 → "출처를 알 수 없는 앱" 허용 후 설치.

> 이 APK는 디버그 서명으로 빌드됩니다(사이드로딩·교실 배포에 충분).
> 정식 배포(스토어/장기 배포)가 필요하면 release 서명 키를 추가하세요.

## 사양

| 항목 | 값 |
|---|---|
| 패키지명 | `com.woolban.molipstudy` |
| minSdk | 26 (Android 8.0) |
| targetSdk / compileSdk | 34 (Android 14) |
| 언어 | Java |
| 의존성 | androidx.appcompat, androidx.webkit |
