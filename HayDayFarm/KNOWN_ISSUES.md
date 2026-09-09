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

## KI-03 — No Hay Day reference imagery; Phase 3 is blocked

**Status:** BLOCKED — needs the user
**Severity:** High (blocks the largest remaining phase)

`Docs/References/` is empty. Phase 3 reconstructs the farm's spatial layout,
and there is nothing to reconstruct it from.

**What is needed**, in priority order (the folder's README has detail): a
wide shot of the whole farm, path shapes, farmhouse and barn from a couple of
angles, animal areas with fences, the pond, and at least one sunset shot.

A guessed layout was deliberately not produced. It would have to be thrown
away, and in the meantime it would silently become the reference for building
scale, path widths and sight lines.

**Not blocked by this:** the first-build task, Phase 4 (interaction
framework), and additional camera modes. See NEXT_TASK.md.

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

**Status:** OPEN — user is creating the repository
**Severity:** Medium (affects where this code lives, not the code)

The request was to develop in a newly created repository. Creating one via
the GitHub API failed with `403 Resource not accessible by integration` — the
session's GitHub app is not permitted to create repositories, and its access
is scoped to `kmj84297school/order-up-renew`.

**What was done instead:** the project is entirely self-contained in the
`HayDayFarm/` directory, sharing nothing with the APK project around it.

**Agreed 2026-09-09:** the user creates an empty repository on GitHub and
names it, then the project is pushed there as its own repository.

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
