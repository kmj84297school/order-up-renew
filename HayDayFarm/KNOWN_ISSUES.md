# KNOWN_ISSUES

Status values: **OPEN**, **BLOCKED**, **PLACEHOLDER** (works, meant to be
replaced), **RESOLVED**.

---

## KI-01 — Three engine API calls are unverified against real headers

**Status:** OPEN — resolve on first compile
**Severity:** High (blocks the build if wrong)
**File:** `Source/HayDayFarm/Environment/FarmWorldBootstrap.cpp`

The project has never been compiled (no Unreal installation was available).
Every line was written against known-stable engine API, with three exceptions
that carry real risk of not matching UE 5.4 exactly:

```cpp
SunLight->bAtmosphereSunLight = true;
SkyLight->SourceType = ESkyLightSourceType::SLS_CapturedScene;
SkyLight->bRealTimeCapture = true;
```

**If any fails to compile:** find the correct member or setter in
`Components/DirectionalLightComponent.h` / `Components/SkyLightComponent.h`
and fix it. Do **not** remove the sky setup to get a green build — without
the sun/atmosphere link the sky renders wrong, and Phase 7's time-of-day
presets are built on this.

More broadly: *any* compile error found on the first build belongs in this
file with its cause, not just a silent fix.

---

## KI-02 — `generate_bootstrap_level.py` is unverified

**Status:** OPEN — resolve on first run
**Severity:** Medium (there is a manual workaround)
**File:** `Tools/generate_bootstrap_level.py`

The script uses `unreal.LevelEditorSubsystem` and
`unreal.EditorActorSubsystem`. Both are the correct modern UE 5.x APIs, but
the script has never been executed, because running it requires the editor.

**Workaround if it fails:** place the two actors by hand — an
`AFarmWorldBootstrap` at the origin and a `PlayerStart` at `(0, 0, 120)` —
and save the level as `Content/Levels/L_FarmBlockout`. That path is what
`DefaultEngine.ini` already points `GameDefaultMap` and `EditorStartupMap` at.

---

## KI-03 — Reference imagery received; exact reconstruction has remaining gaps

**Status:** RESOLVED for missing references (2026-09-12)
**Severity:** Informational

15 attachments are stored as 13 unique PNGs in Docs/References with a hash
manifest. README.md classifies inspected imagery, and RESEARCH.md records
additional studio/artist links and their verification limits.
A reference-backed core-area study can proceed; a complete measured farm plan,
hidden building faces and a confirmed sunset reference are still unavailable.
Do not assume screenshots show one continuous farm. First Unreal build remains
blocked by KI-11 and precedes engine blockout. Reference receipt is not a
completed Phase 3 level.

---

## KI-04 — `GameDefaultMap` points at a level that does not exist yet

**Status:** OPEN
**Severity:** Low
**File:** `Config/DefaultEngine.ini`

`GameDefaultMap` and `EditorStartupMap` are both set to
`/Game/Levels/L_FarmBlockout`, which is not in the repository — a `.umap` is
binary and cannot be authored without the editor.

**Effect:** the editor will warn about a missing startup map on first open.
Harmless. Resolved by KI-02's script or by creating the level by hand.

---

## KI-05 — `AFarmWorldBootstrap` is a placeholder world

**Status:** PLACEHOLDER — replaced in Phase 3
**Severity:** None (working as intended)
**File:** `Source/HayDayFarm/Environment/FarmWorldBootstrap.cpp`

A flat 120 m × 120 m plane using `/Engine/BasicShapes/Plane`, one directional
light, a sky light, a sky atmosphere and light fog. Enough to stand on and
validate movement — nothing more. Sun angle and fog values are stand-ins, not
art direction.

It removes itself from the picture automatically: `AFarmGameMode` only spawns
it when the level contains no `AFarmWorldBootstrap`, so Phase 3's level
simply supersedes it. Deleting the class is a Phase 3 or Phase 9 cleanup.

---

## KI-06 — Interact key is wired to nothing

**Status:** RESOLVED (2026-09-09)
**File:** `Source/HayDayFarm/Player/FarmCharacter.cpp`

`UFarmInteractionComponent` now subscribes to
`AFarmCharacter::OnInteractPressed` in `BeginPlay`, so **E** reaches whatever
the player is looking at. The delegate seam was kept rather than calling the
component directly — see DECISIONS.md D-11.

Unverified in engine like everything else (see KI-01).

---

## KI-07 — No content assets of any kind

**Status:** BLOCKED / PLACEHOLDER
**Severity:** Informational — this is simply where the project is

Nothing has been authored: no meshes, materials, textures, animations,
animal assets, audio, fonts, or levels. `Content/` holds only an empty
`Levels/` folder.

Phase 9 is where this gets addressed properly, and the brief says to inventory
required assets and report before starting it. That inventory does not exist
yet and should be produced at the end of Phase 3, when the blockout says what
is actually needed.

---

## KI-08 — Esc in Play-In-Editor also stops play

**Status:** OPEN — low priority
**Severity:** Low

**E**sc is bound to the menu-mode toggle, but in PIE the editor also treats
Esc as "stop playing". Testing the toggle needs a standalone build, or a
temporary rebind.

Not worth working around now. Revisit when the Phase 4 or later UI gives Esc
something real to do.

---

## KI-12 — `Tools/Setup.ps1` is unverified

**Status:** OPEN — resolve on first run
**Severity:** Low (RUNNING.md section 3 is the manual equivalent)
**File:** `Tools/Setup.ps1`

Automates locate-engine → validate → build → generate level → launch. Written
without a Windows machine or an Unreal installation to test against, so every
path in it is reasoned rather than observed. The parts most likely to be wrong:

- engine auto-detection from `HKLM:\SOFTWARE\EpicGames\Unreal Engine`
- the `Build.bat <Target> Win64 Development -Project=...` invocation
- `-run=pythonscript` exiting non-zero even on success

If it fails, RUNNING.md section 3 has the same steps by hand, and nothing
about the project depends on this script.

---

## KI-10 — Interaction prompt is on-screen debug text

**Status:** PLACEHOLDER — replaced when the UI phase lands
**Severity:** None (working as intended)
**File:** `Source/HayDayFarm/Interaction/FarmInteractionComponent.cpp`

The prompt is drawn with `GEngine->AddOnScreenDebugMessage`, wrapped in
`#if !UE_BUILD_SHIPPING`. A real prompt needs a UMG widget, which is a binary
asset and cannot be authored without the editor.

The framework itself is complete: `UFarmInteractionComponent::OnFocusChanged`
broadcasts the focused actor and its prompt text, so a widget subscribes to
that and the component does not change. Toggle with `bShowDebugPrompt`.

Related: the focus highlight uses a 4% scale-up as a stand-in.
`SetRenderCustomDepth(true)` is already called and is the real hook, but it is
inert until Phase 10 supplies an outline post-process material.

---

## KI-11 — Unreal Engine is not installed on the user's machine

**Status:** BLOCKED — needs the user
**Severity:** High (blocks all engine-side verification)

Confirmed by the user on 2026-09-09. Until Unreal 5.4 (or another 5.x, with
the three-line version change in NEXT_TASK.md Task A) is installed, nothing in
this project can be compiled, opened, or played, and KI-01, KI-02, KI-04 and
KI-08 cannot be closed.

The scheduled daily sessions run in a headless Linux container and are subject
to the same limit. They must continue writing and statically validating code,
and must never report a build or a test they could not run.

---

## KI-09 — GitHub repository could not be created from the session

**Status:** RESOLVED (2026-09-09) — staying in `order-up-renew` by decision
**Severity:** None

The request was to develop in a newly created repository. Creating one via
the GitHub API failed with `403 Resource not accessible by integration` — the
session's GitHub app is not permitted to create repositories, and its access
is scoped to `kmj84297school/order-up-renew`.

**What was done instead:** the project is entirely self-contained in the
`HayDayFarm/` directory, sharing nothing with the APK project around it.

**Decided 2026-09-09:** rather than wait on a new repository, the project
stays here, in `HayDayFarm/` on branch `claude/hay-day-3d-farm-5ijgdj`. It
shares no files, no build system and no history with the APK project around
it, so this costs nothing today.

The move stays available at any time — the commands below still apply — but
nothing is blocked on it, and no further work should wait for it.

**To move it to its own repository:**

```bash
# on your machine, after creating an empty repo on GitHub
git clone https://github.com/kmj84297school/order-up-renew tmp-clone
cd tmp-clone && git checkout claude/hay-day-3d-farm-5ijgdj
cp -r HayDayFarm ../hayday-farm-ue5
cd ../hayday-farm-ue5
git init && git lfs install
git add . && git commit -m "Initial commit: HayDay farm UE5 project"
git remote add origin https://github.com/<you>/<new-repo>
git push -u origin main
```

Git history for these files is short and not worth preserving across the
move; the working tree is what matters.


## KI-13 — Bench view awaits engine verification

**Status:** OPEN / PLACEHOLDER (2026-09-12)
**Severity:** Medium

Bench camera and interaction code passes static checks but has never been
compiled or played. No engine was found during the Windows audit. The seat is
an engine cube, view-only: the pawn remains at its original safe approach
position. No body animation or camera obstruction sweep is implemented.
A seat placed near walls could clip the camera during transition. Keep the
fixture in the open test area until obstruction handling is validated.
Execute `Docs/BENCH_VERIFICATION.md` on first build, especially quick toggle,
seat destruction, controller replacement and movement-lock restoration.

## KI-14 — Interaction prompt stayed stale on the same actor

**Status:** RESOLVED in source (2026-09-12); runtime verification pending
**Severity:** Low

`SetFocusedActor` returned early for an unchanged actor without re-reading
its prompt. Toggling a rotator therefore retained its previous prompt.
The component now compares text on refresh, broadcasts changed text, refreshes
immediately after interaction, and clears debug text when there is no focus.
The same path supports the bench's persistent "Stand up" prompt.


## KI-15 — Missing Farm header escaped static include validation

**Status:** RESOLVED (2026-09-12)
**Severity:** Medium

Fault injection found that an include with a nonexistent basename was always
assumed to be an engine/plugin header. `Camera/FarmMissingMode.h` escaped.
The checker now identifies the project's Farm basename convention, while
continuing to allow engine headers in the shared Camera directory.
Verification: four regression tests pass via `python Tools/test_validate_project.py`;
three isolated seeded faults (generated header, undefined method, missing
project header) are detected, with a clean restored fixture.
