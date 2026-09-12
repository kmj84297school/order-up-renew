# PROJECT_STATE

Last updated: 2026-09-12

## What this project is

A high-quality stylized 3D farm you can walk around in, built in Unreal
Engine 5. The goal is atmosphere and presence, not simulation: the player
enters the farm, walks at a relaxed pace, looks at things, and interacts with
a small number of objects and animals. Private personal project.

Explicitly **not** in scope: economy, progression, quests, multiplayer,
inventories, production chains, farming management, large UI.

## Current phase

**Phases 1, 2 and 4 written. None compiled. Phase 3 references received; engine blockout pending.**

| Phase | Title | Status |
|---|---|---|
| 0 | Environment & requirements audit | Done |
| 1 | Stable project foundation | Code complete, **unbuilt** (see below) |
| 2 | Free walk movement and camera | Code complete, **unverified in engine** |
| 3 | Hay Day farm blockout | References received; first engine build pending (KI-03/KI-11) |
| 4 | Interaction framework | Code complete, **unverified in engine** |
| 5+ | Animals, motion, time/weather, audio, assets, polish, tour, optimisation | Not started |

Phase 4 was taken out of order deliberately: Phase 3 is blocked on user-supplied
references, Phase 4 needs no art, and the brief says to continue whatever is not
blocked rather than stall.

## Build status

**Never compiled.** The development environment for this project so far has
been a headless Linux container with no Unreal Engine installation, so no
build, no editor session and no PIE run has happened.

2026-09-12 Windows audit: Epic Launcher is present, but its installation list
is empty; no usable UE 5.x installation was found in the default engine
folder or registered engine locations. The legacy 4.0 registry entry points
to an absent directory. No engine build or play test was attempted.

What *has* been verified:

- 2026-09-12: Four include-validator regression tests pass. Three isolated
  seeded faults were detected after correcting KI-15, and the restored copy
  passed. All Python tools parse; project JSON parses; `git diff --check`
  passes. These are static/tool checks, not Unreal gameplay tests.

- `Tools/validate_project.py` passes with 0 errors across 20 headers and
  20 sources. It checks generated.h placement, GENERATED_BODY presence
  (UCLASS/USTRUCT/UINTERFACE), include-path correctness, brace balance,
  declared-but-undefined methods, undeclared `Farm*` type references, and
  .uproject/Build.cs/Target.cs wiring. It understands that UHT generates the
  bodies of `BlueprintNativeEvent` functions and does not flag them.
- The validator was fault-injection tested: **eight seeded faults, one per
  check, all eight detected**, with the baseline clean before and after.
- All four `Config/*.ini` files parse.
- `HayDayFarm.uproject` is valid JSON.
- Both Python tools parse.

This is static checking. It is not a compile and does not prove the project
builds. **The first task in NEXT_TASK.md is to build it.**

## Engine

- **Target: Unreal Engine 5.4** (`EngineAssociation` in `HayDayFarm.uproject`).
- **The user has confirmed Unreal is not installed yet.** 5.4 is therefore
  still an unconfirmed default. If a different 5.x gets installed, change the
  one `EngineAssociation` field and the two `IncludeOrderVersion` lines in
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
    FarmWorldBootstrap.h/.cpp     PLACEHOLDER lit ground plane + test props
  Interaction/
    FarmInteractable.h/.cpp             IFarmInteractable interface
    FarmInteractionComponent.h/.cpp     Focus detection + interact routing
    FarmInteractableActor.h/.cpp        Optional mesh-prop base class
    FarmInteractable_Rotator.h/.cpp     Test prop / windmill prototype
    FarmInteractable_Nudge.h/.cpp       Test prop / crop-reaction prototype
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
| E | Interact — routed to `UFarmInteractionComponent` |
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
fonts, or levels. Most importantly, **13 unique Hay Day reference PNGs are now supplied**.
Their classification and additional research are in `Docs/References/README.md`
and `RESEARCH.md`. Engine installation and first build remain pending.

## Interaction (Phase 4)

`IFarmInteractable` is the contract — an interface, not a base class, so a
chicken and a windmill can both be interactable without sharing an ancestor.
All four functions are `BlueprintNativeEvent`, so interactables can be written
in C++ or Blueprint.

`UFarmInteractionComponent` lives on the character, sphere-sweeps from the
pawn's view point (250 cm range, 14 cm radius) on a **0.08 s timer rather than
every frame**, and routes presses via the character's `OnInteractPressed`
delegate. Two test interactables are spawned by the placeholder world.

The on-screen prompt is currently `AddOnScreenDebugMessage` — a declared
placeholder, see KI-10.

## Automation

**Currently DISABLED.** Development is being handed to another coding agent
(see `Docs/CODEX_HANDOFF.md`), and two agents pushing to the same branch on a
schedule would collide.

The Routine still exists and can be re-enabled without rebuilding it:
`trig_01XewNh9VVXigHdSSoqDqhkh`, cron `0 17 * * *` UTC = **02:00 KST**,
fresh session per firing, reads these four files and continues NEXT_TASK.md.

If it is re-enabled, note that those sessions run in a headless Linux
container and **cannot build or test in the editor** — that constraint is
stated in the Routine's own prompt.

## Bench view extension (2026-09-12)

Task B's first unit is written: `UFarmCameraMode_Bench` and
`AFarmInteractable_Bench`, with a seated session owned by the existing
interaction component. The camera blends to a fixed seat eye location;
mouse look remains available and E exits regardless of aim. Movement locks
are balanced on exit, seat loss, controller change and component teardown.
Character and PlayerController source files are unchanged.

The bootstrap spawns a placeholder seat block. The pawn remains at its
approach location; no seated animation or camera obstruction sweep exists.
Prompt text now refreshes even when focus stays on the same actor.
See `Docs/BENCH_VERIFICATION.md` for pending engine checks and KI-13/KI-14.

## Reference intake (2026-09-12)

Stored 15 user attachments as 13 unique original PNGs through Git LFS, with
SHA-256 manifest and duplicate mappings. Image 4 supports a limited core-area
adjacency study; images 5/6 support paths/planting. The regional map is not a
single farm plan. External studio/artist sources are listed with inspection
limits in `Docs/References/RESEARCH.md`. KI-03's missing-reference condition is
resolved. No engine level was authored and no runtime verification occurred.

## Recent major changes

- 2026-09-12: Added bench viewing and exit routing; fixed cached interaction
  prompt text. Static validation passes on 20 headers / 20 sources; Unreal
  compilation and runtime verification remain blocked.

- 2026-09-09: Project created from scratch. The repository previously held an
  unrelated Android APK repackaging project; that work is untouched on the
  default branch.
- 2026-09-09: Phase 4 interaction framework added; validator extended for
  UINTERFACE and BlueprintNativeEvent and re-fault-tested (8/8).
- 2026-09-09: Decided the project stays in `order-up-renew` rather than
  waiting on a new repository (KI-09 closed). Added `Tools/Setup.ps1` and
  `RUNNING.md` so the first build is one command.
- 2026-09-09: Added `Docs/CODEX_HANDOFF.md`, a paste-ready prompt for handing
  the project to another coding agent. **Keep it current on handoff** — its
  "Current state" and "Your task" sections go stale fastest.
- 2026-09-09: Disabled the daily Routine to avoid two agents pushing to the
  same branch. See "Automation" above for how to turn it back on.
