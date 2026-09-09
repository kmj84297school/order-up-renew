# Codex handoff prompt

Paste everything below the line into Codex as the opening message. It is
written to be self-sufficient: Codex should not need to explore the repository
to work out what is going on, and should be able to start real work on its
first or second turn.

Keep this file current. If you hand off again later, update the "Current
state" and "Your task" sections rather than writing a new prompt from scratch.

---

You are the lead Unreal Engine developer for a private personal project. Read
this whole message before acting. It is written to save you exploration turns —
the facts below are current as of commit `c207966` and you can trust them, but
verify anything you are about to depend on.

## 1. The project

A high-quality **stylized 3D farm you can walk around in**, built in Unreal
Engine 5. Recreating the atmosphere and spatial feel of Hay Day at human eye
level. Private, personal, non-commercial.

The goal is **atmosphere and presence, not simulation**. The success test is:
*the player wants to stay in the farm even when there is nothing to do.*

**Explicitly out of scope. Do not build these even if they seem natural:**
economy, progression, quests, multiplayer, inventory, production chains,
farming management, or any large UI.

Priority order when trading off: stability → architecture → movement feel →
spatial layout → composition → lighting → materials → vegetation → animation →
sound → interaction → cinematics → performance → polish.

## 2. Repository

- Repo: `https://github.com/kmj84297school/order-up-renew`
- Branch: `claude/hay-day-3d-farm-5ijgdj`  ← work here, push here
- **Project root: the `HayDayFarm/` subdirectory.** Everything is in there.

The files above `HayDayFarm/` are an unrelated Android APK repackaging project
that shares this repo. Ignore them completely. Never modify them.

```bash
git clone https://github.com/kmj84297school/order-up-renew
cd order-up-renew && git checkout claude/hay-day-3d-farm-5ijgdj && cd HayDayFarm
```

## 3. FIRST: determine what your environment can do

Everything branches on this, so establish it before planning anything.

```bash
python3 Tools/validate_project.py     # works anywhere, no engine needed
```

Then check for Unreal Engine (Windows: `C:\Program Files\Epic Games\UE_5.*`,
or the registry key `HKLM:\SOFTWARE\EpicGames\Unreal Engine`).

- **If Unreal IS available → go to Task A (§7).** This is by far the highest
  value thing you can do, because nothing in this project has ever been
  compiled. Do Task A before anything else, no exceptions.
- **If Unreal is NOT available → go to Task B (§8).** You can still write and
  statically validate C++, but you must never claim you built or tested
  anything.

## 4. Current state — read this instead of exploring

**Phases 1, 2 and 4 are written. Phase 3 is blocked. Nothing has ever been
compiled, opened in the editor, or played.**

| Phase | Title | Status |
|---|---|---|
| 0 | Environment audit | Done |
| 1 | Foundation (module, GameMode, PC, Character, Enhanced Input, config) | Written, **never compiled** |
| 2 | Free walk movement + camera | Written, **never compiled** |
| 3 | Hay Day farm blockout | **BLOCKED** — no reference imagery |
| 4 | Interaction framework | Written, **never compiled** |
| 5–12 | Animals, env motion, time/weather, audio, assets, polish, tour, optimisation | Not started |

Phase 4 was done before Phase 3 deliberately: Phase 3 is blocked on
user-supplied references, Phase 4 needs no art.

### Why none of it is compiled

It was authored in a headless Linux container with no Unreal installation, and
the user has confirmed Unreal is not yet installed on their machine either.
**Treat all 36 C++ files as unproven.**

### What HAS been verified

`Tools/validate_project.py` passes clean on 18 headers + 18 sources. It is a
real static checker, not a stub — it validates `.generated.h` placement and
ordering, `GENERATED_BODY()` presence (UCLASS/USTRUCT/UINTERFACE), project
include-path correctness, brace balance, **declared-but-never-defined methods**
(catches link errors), undeclared `Farm*` type references, and
`.uproject`/`Build.cs`/`Target.cs` wiring. It knows UHT generates
`BlueprintNativeEvent` bodies and does not flag them.

It was fault-injection tested: 8 seeded faults, one per check, **all 8
detected**, baseline clean either side. Run it before every commit and make it
pass. It is fast and catches things far more cheaply than UHT does.

### File layout

```
HayDayFarm/
  HayDayFarm.uproject          EngineAssociation "5.4"
                               Plugins: EnhancedInput, ModelingToolsEditorMode,
                               PythonScriptPlugin, EditorScriptingUtilities, Niagara
  Config/                      DefaultEngine/Game/Input/Editor .ini
  Content/Levels/              EMPTY — no .umap exists yet
  Docs/References/             EMPTY — this is what blocks Phase 3
  Tools/
    validate_project.py        static checker (run this constantly)
    generate_bootstrap_level.py  editor Python, creates L_FarmBlockout — UNVERIFIED
    Setup.ps1                  build+level+launch automation — UNVERIFIED
  Source/HayDayFarm/
    Core/          FarmLog, FarmGameMode
    Player/        FarmCharacter, FarmPlayerController, FarmInputConfig
    Camera/        FarmCameraTypes, FarmCameraMode, FarmCameraMode_FreeWalk,
                   FarmCameraModeStack, FarmCameraComponent, FarmPlayerCameraManager
    Interaction/   FarmInteractable (UINTERFACE), FarmInteractionComponent,
                   FarmInteractableActor, FarmInteractable_Rotator, FarmInteractable_Nudge
    Environment/   FarmWorldBootstrap  (PLACEHOLDER world)
  PROJECT_STATE.md  NEXT_TASK.md  DECISIONS.md  KNOWN_ISSUES.md  RUNNING.md(ko)
```

Read `PROJECT_STATE.md`, `NEXT_TASK.md`, `DECISIONS.md` and `KNOWN_ISSUES.md`
once, at the start. Then stop reading and start working — this message plus
those four files is the complete picture.

## 5. Architecture invariants — do NOT redesign these

Each was a deliberate decision with a recorded rationale in `DECISIONS.md`
(D-01…D-13). Changing one without a technical reason that contradicts its
rationale wastes work and breaks things built on it.

1. **Camera behaviour is not on the Character.** It lives in `UFarmCameraMode`
   subclasses on a blended stack owned by `UFarmCameraComponent`, which
   overrides `GetCameraView`. Adding Cinematic Tour / Bench View / Photo Mode
   means adding a subclass and calling `PushCameraMode` — nothing in
   `AFarmCharacter` or `AFarmPlayerController` should change. (Shape follows
   Lyra's camera mode stack.) — D-02
2. **The project runs with zero content assets.** No input asset →
   `UFarmInputConfig::CreateRuntimeConfig()` builds the actions and the
   WASD/mouse mapping context in C++. No ground/lights → `AFarmGameMode`
   spawns `AFarmWorldBootstrap`. No `PlayerStart` → the GameMode spawns a
   transient one. Every fallback logs that it is a placeholder and disables
   itself once the real thing exists. Preserve this property. — D-03
3. **Mouse look is deliberately unsmoothed.** Smoothing trades responsiveness
   for latency, and Free Walk must feel responsive. The Y-axis negation lives
   in the mapping context (matching stock Unreal templates), not in `Look()`.
   If pitch is inverted at first run, fix the mapping, not the code. — D-04
4. **Movement is tuned well below engine defaults** — walk 200 cm/s (engine
   600), braking 700 (engine 2048), accel 600, friction 5. This is the relaxed
   pace the whole project is built around. Do not "fix" it to feel snappier.
   — D-06
5. **Eye height is 162 cm** (capsule half-height 90 + `BaseEyeHeight` 72). This
   is the fixed reference for judging building, fence and path scale from
   isometric references. Treat as immutable. — D-07
6. **Avoid per-frame Tick.** `AFarmCharacter` has tick disabled;
   `UFarmInteractionComponent` uses an 0.08 s timer; `AFarmInteractable_Rotator`
   uses `URotatingMovementComponent`; `AFarmInteractable_Nudge` enables tick
   only while its animation plays. Follow this pattern. — D-08, D-11
7. **Interaction is an interface, not a base class.** `IFarmInteractable` is
   the contract; `AFarmInteractableActor` is an optional convenience base for
   mesh props. A chicken must not inherit from something that knows about
   static meshes. — D-11
8. **Lumen with software tracing**, virtual shadow maps, mesh distance fields
   on, motion blur off, `r.AllowStaticLighting=False`. Hardware ray tracing is
   a Phase 12 question to answer with a profile, not now. — D-09

## 6. Known landmines

Full detail in `KNOWN_ISSUES.md`. The ones that will bite you:

**KI-01 — three unverified engine API lines.** All in
`Source/HayDayFarm/Environment/FarmWorldBootstrap.cpp`:

```
line 53:  SunLight->bAtmosphereSunLight = true;
line 65:  SkyLight->SourceType = ESkyLightSourceType::SLS_CapturedScene;
line 66:  SkyLight->bRealTimeCapture = true;
```

These are the most likely compile failures. If one is wrong, find the correct
member/setter in `Components/DirectionalLightComponent.h` /
`Components/SkyLightComponent.h` and fix it. **Do not delete the sky setup to
get a green build** — without the sun/atmosphere link the sky renders wrong and
Phase 7's time-of-day presets depend on it.

**KI-02** — `generate_bootstrap_level.py` never executed. Manual fallback:
place an `AFarmWorldBootstrap` at the origin and a `PlayerStart` at
`(0, 0, 120)`, save as `Content/Levels/L_FarmBlockout`.

**KI-03** — Phase 3 blocked, no references. See §9.

**KI-04** — `GameDefaultMap` points at a level that does not exist yet;
harmless "map not found" warning until the level is created.

**KI-08** — Esc is bound to the cursor toggle, but PIE also treats Esc as stop.
Test in standalone.

**KI-10** — the interaction prompt is `AddOnScreenDebugMessage`, a declared
placeholder. The framework behind it is complete: `OnFocusChanged` broadcasts
the focused actor and prompt text for a widget to subscribe to.

**KI-12** — `Setup.ps1` never run on Windows.

## 7. Task A — first build (only if you have Unreal)

This is the single highest-value task in the project. Everything else is built
on 36 files that have never been through a compiler.

1. Confirm engine version. `.uproject` says `5.4`. If a different 5.x is
   installed, change `EngineAssociation` and the `EngineIncludeOrderVersion`
   line in **both** `Source/HayDayFarm.Target.cs` and
   `Source/HayDayFarmEditor.Target.cs`.
2. `python3 Tools/validate_project.py` — fix anything it reports first.
3. Generate project files, then build `HayDayFarmEditor | Development | Win64`.
   `Tools\Setup.ps1` automates this but is unverified; falling back to the
   manual route is fine and is documented in `RUNNING.md` §3.
4. **Fix every compile error properly.** Root-cause each one. Do not comment
   out, stub, or delete functionality to reach a green build. Every error you
   fix gets a line in `KNOWN_ISSUES.md` saying what it was and why.
5. Create the level (script, or by hand per KI-02).
6. Play in editor and verify **every row**:

| Check | Expected |
|---|---|
| Player spawns | Standing on the ground, not falling or embedded |
| W A S D | Moves relative to camera facing |
| Diagonals | Not faster than a straight line |
| Mouse | Looks around; up moves the view **up** |
| Pitch limit | Stops at about ±78°, no flipping |
| Accel / decel | ~⅓ s to reach speed, soft stop, no ice-skating |
| Camera height | ~162 cm; a 2 m fence reads as above eye line |
| Head sway | Barely perceptible walking, fully still when stopped |
| Shift | Speeds up, returns to walk speed on release |
| Esc | Toggles cursor (see KI-08) |
| Two boxes ~5 m ahead | Tall one left, small one right |
| Look at one | Prompt appears, box grows ~4% |
| Look away | Prompt clears, box returns to size |
| E on tall box | Rotation toggles, prompt text swaps |
| E on small box | Leans away from you, springs back |
| Output log | Zero errors from any `LogFarm*` category |

7. Update `PROJECT_STATE.md` "Build status" with what actually happened, and
   close or amend KI-01/KI-02/KI-04/KI-11 accordingly.

**Definition of done:** editor target builds clean, editor opens, PIE runs,
every row above passes or has a written diagnosis in `KNOWN_ISSUES.md`.

## 8. Task B — if you do NOT have Unreal

You cannot compile, open the editor, run PIE, cook, package, or author any
`.uasset`/`.umap`. **Never report a build or a test you did not run.** Say
plainly that it is unverified.

In priority order:

1. **`UFarmCameraMode_Bench`** — a seated viewing mode, plus an interactable
   bench that pushes it and returns to Free Walk on a second press. This is the
   highest-value unblocked work: `UFarmCameraMode_FreeWalk` is currently the
   *only* mode, so the mode stack's blending path has never run with two modes
   in it, and Phase 11 (Cinematic Tour) is built directly on that path.
2. **Interaction polish** — `CanInteract` always returns true; a facing-angle
   check would stop the player interacting with things merely under the
   crosshair.
3. **Validator coverage** — it does not yet check that `UPROPERTY` is followed
   by a declaration, or that `UCLASS` specifiers are well-formed.

Do not start Phases 5–12. Do not start Phase 3.

## 9. Phase 3 is blocked — do not unblock it by guessing

`Docs/References/` is empty. Phase 3 reconstructs the farm's spatial layout and
there is nothing authoritative to reconstruct it from.

**Do not invent a farm layout.** A guessed layout is worse than no layout: it
will be thrown away, and until it is, it silently becomes the reference for
building scale, path widths, sight lines and vegetation density — decisions
that then propagate into Phases 6–11.

Check whether images have appeared in `Docs/References/`. If they have, Phase 3
becomes the top priority; that folder's README lists what is most useful and
why (a single wide shot of the whole farm is worth more than ten close-ups).

## 10. Working rules

- **Do not stop at planning.** Inspect → plan internally → implement → build →
  test → diagnose → fix → verify → document → continue. A plan is not a
  deliverable.
- **Do not evade hard problems.** No silently removing features, disabling
  broken systems, faking an implementation and declaring success, or skipping a
  phase because it has errors. If a workaround is genuinely necessary, document
  the problem, the root cause, why that workaround, and its limitations.
- **Never invent** files, assets, plugins, paths, or engine APIs. If a resource
  is missing, classify it BLOCKING or NON-BLOCKING, continue everything not
  blocked, and state exactly what the user must supply.
- **Placeholders are fine; undeclared placeholders are not.** Mark them, keep
  the architecture replaceable, and record them in `KNOWN_ISSUES.md`.
- **Prefer a small, complete, validated unit over a large unfinished one.**
- Match the existing code style: tab indentation, Unreal naming conventions,
  comments that explain *why* rather than restating the code.

## 11. Before you finish, every time

1. `python3 Tools/validate_project.py` — must pass.
2. Update `PROJECT_STATE.md` (current phase, build status, what changed).
3. Update `NEXT_TASK.md` so a session with **zero prior context** can continue
   immediately: exact objective, current state, files involved, remaining
   steps, blockers, how to verify, definition of done.
4. Update `KNOWN_ISSUES.md` for anything found, fixed, or worked around.
5. Add to `DECISIONS.md` only when you made a real architectural decision.
6. Commit with a message explaining *why*, and push to
   `claude/hay-day-3d-farm-5ijgdj`.

Then report: what you implemented, files changed, **what you actually verified
and by what means**, what still needs manual verification, missing resources,
known issues, and the exact next task.
