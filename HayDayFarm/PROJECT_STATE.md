# PROJECT_STATE

Last updated: 2026-09-09

## What this project is

A high-quality stylized 3D farm you can walk around in, built in Unreal
Engine 5. The goal is atmosphere and presence, not simulation: the player
enters the farm, walks at a relaxed pace, looks at things, and interacts with
a small number of objects and animals. Private personal project.

Explicitly **not** in scope: economy, progression, quests, multiplayer,
inventories, production chains, farming management, large UI.

## Current phase

**Phase 2 complete (code-side). Phase 3 not started.**

| Phase | Title | Status |
|---|---|---|
| 0 | Environment & requirements audit | Done |
| 1 | Stable project foundation | Code complete, **unbuilt** (see below) |
| 2 | Free walk movement and camera | Code complete, **unverified in engine** |
| 3 | Hay Day farm blockout | Not started — blocked on reference imagery |
| 4 | Interaction framework | Not started |
| 5+ | Animals, motion, time/weather, audio, assets, polish, tour, optimisation | Not started |

## Build status

**Never compiled.** The development environment for this project so far has
been a headless Linux container with no Unreal Engine installation, so no
build, no editor session and no PIE run has happened.

What *has* been verified:

- `Tools/validate_project.py` passes with 0 errors across 13 headers and
  13 sources. It checks generated.h placement, GENERATED_BODY presence,
  include-path correctness, brace balance, declared-but-undefined methods,
  undeclared `Farm*` type references, and .uproject/Build.cs/Target.cs
  wiring. The validator was itself fault-injection tested — all six seeded
  faults were detected.
- All four `Config/*.ini` files parse.
- `HayDayFarm.uproject` is valid JSON.
- Both Python tools parse.

This is static checking. It is not a compile and does not prove the project
builds. **The first task in NEXT_TASK.md is to build it.**

## Engine

- **Target: Unreal Engine 5.4** (`EngineAssociation` in `HayDayFarm.uproject`).
- This was chosen as a stable default, *not* because it was measured against
  an installed engine. If a different 5.x is installed, change the one
  `EngineAssociation` field and the two `IncludeOrderVersion` lines in
  `Source/*.Target.cs`. See DECISIONS.md D-01.
- Platform target: Windows PC, D3D12.

## Architecture

Module: `HayDayFarm` (Runtime). `Source/HayDayFarm` is on the public include
path, so headers are included as `"Camera/FarmCameraComponent.h"` etc.

```
Source/HayDayFarm/
  Core/
    FarmLog.h/.cpp          Log categories, split per subsystem
    FarmGameMode.h/.cpp     Spawns placeholder world + fallback PlayerStart
  Player/
    FarmCharacter.h/.cpp    Movement, look, input binding. No Tick.
    FarmPlayerController.h/.cpp   Input *mode* (captured vs menu cursor)
    FarmInputConfig.h/.cpp  Enhanced Input actions + code-built defaults
  Camera/
    FarmCameraTypes.h/.cpp        FFarmCameraView + shortest-arc blending
    FarmCameraMode.h/.cpp         Base mode, blend weight/easing
    FarmCameraMode_FreeWalk.h/.cpp  Eye-level walking view + gentle sway
    FarmCameraModeStack.h/.cpp    Ordered stack, bottom-to-top blend
    FarmCameraComponent.h/.cpp    UCameraComponent that asks the stack
    FarmPlayerCameraManager.h/.cpp  Pitch clamp; home for future fades
  Environment/
    FarmWorldBootstrap.h/.cpp     PLACEHOLDER lit ground plane
```

Two decisions shape everything downstream (details in DECISIONS.md):

1. **Camera behaviour is not on the Character.** It lives in
   `UFarmCameraMode` subclasses on a stack. Cinematic Tour, Bench View and
   Photo Mode are each a new subclass plus a `PushCameraMode` call — no
   change to the Character or Controller.
2. **The project runs with zero content assets.** If no input config asset is
   assigned, `UFarmInputConfig::CreateRuntimeConfig()` builds the actions and
   the WASD/mouse mapping context in C++. If the level has no ground or no
   PlayerStart, the GameMode supplies both. This is what made Phases 1–2
   writable at all in an environment with no editor.

## Controls (as implemented)

| Input | Action |
|---|---|
| W A S D / arrows | Move |
| Mouse | Look (pitch clamped to ±78°) |
| E | Interact — fires `OnInteractPressed`; **nothing listens yet (Phase 4)** |
| Esc | Toggle mouse capture / cursor |
| Left Shift | Run (optional, `bAllowRunning`) |

No jump, by design.

## Movement tuning

Walk 200 cm/s, run 340 cm/s, max acceleration 600, braking deceleration 700,
ground friction 5. Capsule 34 r × 90 half-height, eye height 72 → **view sits
at ~162 cm**. Phase 3 building and fence scale must be judged against that
number.

## Assets

**Available:** engine content only (`/Engine/BasicShapes/Plane` for the
placeholder ground).

**Missing — see KNOWN_ISSUES.md for the full list.** Nothing has been
authored: no meshes, materials, textures, animations, animal assets, audio,
fonts, or levels. Most importantly, **no Hay Day reference imagery has been
supplied**, which is what blocks Phase 3.

## Recent major changes

- Project created from scratch in this repository (2026-09-09). The
  repository previously held an unrelated Android APK repackaging project;
  that work is untouched on the default branch.
