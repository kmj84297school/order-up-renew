# 실행 방법

이 문서만 한국어입니다. 나머지 문서(PROJECT_STATE / NEXT_TASK / DECISIONS /
KNOWN_ISSUES)는 매일 자동으로 도는 세션이 읽는 파일이라 영어로 둡니다.

> **아직 한 번도 빌드된 적 없는 프로젝트입니다.** 아래 3단계에서 컴파일
> 에러가 날 수 있고, 그건 정상입니다. KNOWN_ISSUES.md의 **KI-01**에 가장
> 깨질 확률이 높은 3줄을 미리 적어놨습니다.

---

## 0. 준비물

없으면 아무것도 안 됩니다. 순서대로 설치하세요.

| 필요한 것 | 비고 |
|---|---|
| **Windows PC** | 리눅스/맥에서는 이 프로젝트를 돌릴 수 없습니다 |
| **Visual Studio 2022** (Community 무료) | 설치 시 **"C++를 사용한 데스크톱 개발"** 워크로드를 반드시 체크. 이거 없으면 UE가 C++을 컴파일 못 합니다 |
| **Unreal Engine 5.4** | Epic Games Launcher → 언리얼 엔진 → 라이브러리 → `+` → 5.4 설치 |
| **Git** | 코드 받는 용도 |
| **Python 3** (선택) | 있으면 빌드 전에 정적 검사를 자동으로 돌려줍니다 |

설치 용량이 큽니다. 언리얼만 약 40~60GB, Visual Studio가 약 10GB입니다.

> 5.4 말고 다른 5.x를 설치했다면 그대로 두셔도 됩니다. 언리얼이 "버전
> 변환할까요?" 하고 물어보는데 예를 누르면 됩니다. 그 물음창이 싫으면
> NEXT_TASK.md의 Task A에 3줄만 고치는 방법이 있습니다.

---

## 1. 코드 받기

```powershell
git clone https://github.com/kmj84297school/order-up-renew
cd order-up-renew
git checkout claude/hay-day-3d-farm-5ijgdj
cd HayDayFarm
```

프로젝트는 `HayDayFarm/` 폴더 안에 전부 들어 있습니다. 그 위에 있는 APK
파일들은 이 저장소의 원래 프로젝트라 상관없습니다.

---

## 2. 한 줄로 실행 (권장)

PowerShell을 열고 `HayDayFarm` 폴더에서:

```powershell
.\Tools\Setup.ps1
```

이 스크립트가 알아서 합니다:

1. 언리얼 설치 위치를 레지스트리와 기본 경로에서 찾고
2. 정적 검사를 돌리고 (python이 있을 때)
3. 에디터 타겟을 컴파일하고
4. 레벨(`Content/Levels/L_FarmBlockout`)을 만들고
5. 에디터를 엽니다

에디터가 열리면 **Alt+P** 또는 상단의 ▶ Play 버튼을 누르면 농장 안으로
들어갑니다.

실행 정책 때문에 막히면:

```powershell
powershell -ExecutionPolicy Bypass -File .\Tools\Setup.ps1
```

언리얼을 못 찾으면 경로를 직접 알려주세요:

```powershell
.\Tools\Setup.ps1 -EnginePath "C:\Program Files\Epic Games\UE_5.4"
```

**첫 빌드는 10~30분 걸립니다.** 엔진 헤더까지 같이 컴파일해서 그렇고,
두 번째부터는 훨씬 빠릅니다.

---

## 3. 손으로 하는 방법 (스크립트가 안 될 때)

1. `HayDayFarm.uproject` **우클릭 → Generate Visual Studio project files**
2. 생긴 `HayDayFarm.sln`을 Visual Studio로 열기
3. 상단 드롭다운을 **Development Editor** / **Win64** 로 맞추고 **F7**(빌드)
4. 빌드가 끝나면 `HayDayFarm.uproject` 더블클릭
5. 에디터에서 **Tools → Execute Python Script…** → `Tools/generate_bootstrap_level.py` 선택
   - 이게 안 되면(KI-02) 직접 만드세요: 새 레벨을 만들고, `FarmWorldBootstrap`
     액터를 원점(0,0,0)에, `PlayerStart`를 (0, 0, 120)에 놓고
     `Content/Levels/L_FarmBlockout` 으로 저장
6. ▶ Play

---

## 4. 들어가서 확인할 것

지금 보이는 건 **전부 임시 배치물**입니다. 회색 바닥과 상자 두 개뿐이고,
농장은 아직 없습니다(참고 이미지가 있어야 Phase 3에서 만듭니다).

| 조작 | 기대되는 동작 |
|---|---|
| W A S D | 보는 방향 기준으로 이동 |
| 마우스 | 시점 회전, 위아래 78도에서 멈춤 |
| Shift | 조금 빨라짐, 떼면 원래 속도 |
| 앞쪽 상자 두 개 | 왼쪽에 길쭉한 것, 오른쪽에 작은 것 |
| 상자를 쳐다보기 | 화면에 안내 문구가 뜨고 상자가 살짝(4%) 커짐 |
| 긴 상자에 **E** | 회전이 켜졌다 꺼졌다 함 (풍차의 원형입니다) |
| 작은 상자에 **E** | 나를 피하듯 기울었다가 돌아옴 (작물 반응의 원형입니다) |
| **Esc** | 마우스 커서 토글. 단, 에디터 안에서는 Esc가 플레이 종료도 겸합니다 (KI-08) |

체크할 항목 전체 목록은 NEXT_TASK.md의 Task A 6번 표에 있습니다.

---

## 5. 문제가 생기면

| 증상 | 볼 곳 |
|---|---|
| 컴파일 에러 | **첫 번째** 에러부터 읽으세요. KNOWN_ISSUES.md **KI-01** |
| 레벨이 안 만들어짐 | **KI-02**, 위 3번의 손으로 하는 방법 |
| 시작할 때 "map not found" 경고 | **KI-04**. 레벨을 아직 안 만들어서 그렇고, 무해합니다 |
| 마우스 상하가 뒤집힘 | DECISIONS.md **D-04** |
| 그 외 | 에디터 하단 Output Log에서 `LogFarm` 으로 검색 |

에러 메시지를 그대로 붙여넣어 주시면 제가 고치겠습니다. 고친 내용은
KNOWN_ISSUES.md에 기록해 두는 게 이 프로젝트의 규칙입니다.
