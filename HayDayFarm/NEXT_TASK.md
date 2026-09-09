# NEXT_TASK

Written for a session with no memory of previous conversations. Read
PROJECT_STATE.md, DECISIONS.md and KNOWN_ISSUES.md first, then this.

---

## The one thing standing in front of everything else

**Nothing in this project has ever been compiled**, and as of 2026-09-09 the
user has confirmed **Unreal Engine is not installed on their machine yet**.

Phases 1, 2 and 4 are written and pass static validation. None of it has been
through UnrealHeaderTool, a compiler, or a single frame of PIE. Treat all of
it as unproven until it builds.

### Task A — first build (user's Windows machine; cannot be done in the container)

Prerequisite: install Unreal Engine 5.4 via the Epic Games Launcher. If a
different 5.x is installed instead, change `EngineAssociation` in
`HayDayFarm.uproject` and `EngineIncludeOrderVersion.Unreal5_4` in both
`Source/HayDayFarm.Target.cs` and `Source/HayDayFarmEditor.Target.cs`.

1. `python3 Tools/validate_project.py` — fast, needs no engine, catches
   structural mistakes before UHT does. Fix anything it reports first.
2. Right-click `HayDayFarm.uproject` → *Generate Visual Studio project files*.
3. Build `HayDayFarmEditor | Development Editor | Win64`.
4. **Fix compile errors.** The likely failures are listed in KNOWN_ISSUES.md
   as **KI-01** — three engine API lines in
   `Source/HayDayFarm/Environment/FarmWorldBootstrap.cpp`. Do not delete the
   sky setup to get a green build; Phase 7 depends on it.
5. Create the level:
   ```
   "<UE>\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" ^
       "<repo>\HayDayFarm\HayDayFarm.uproject" ^
       -run=pythonscript -script="<repo>\HayDayFarm\Tools\generate_bootstrap_level.py"
   ```
   or in the editor: *Tools → Execute Python Script…*. The script is
   unverified (KI-02); if it fails, place an `AFarmWorldBootstrap` at the
   origin and a `PlayerStart` at `(0, 0, 120)` by hand and save as
   `Content/Levels/L_FarmBlockout`.

6. **Play in editor and check each row:**

   | Check | Expected |
   |---|---|
   | Player spawns | Standing on the ground, not falling or embedded |
   | W A S D | Moves relative to where the camera is looking |
   | Diagonals | No faster than a straight line |
   | Mouse | Looks around; **up moves the view up** (if inverted, see D-04) |
   | Pitch limit | Stops at roughly ±78°, no flipping |
   | Accel / decel | ~⅓ s to reach speed, soft stop; no sliding, no snapping |
   | Camera height | ~162 cm; a 2 m fence should read as above eye line |
   | Head sway | Barely perceptible walking, fully still when stopped |
   | Shift | Speeds up, returns to walk speed on release |
   | Esc | Toggles the cursor (in PIE, Esc may also stop play — KI-08) |
   | Two boxes ahead | A tall one left, a small one right, ~5 m forward |
   | Look at one | On-screen prompt appears; the box grows slightly (~4%) |
   | Look away | Prompt disappears, box returns to normal size |
   | E on the tall box | Rotation starts/stops; prompt text swaps |
   | E on the small box | Leans away from you and springs back |
   | Output log | No errors from any `LogFarm*` category |

7. **Record what actually happened** in PROJECT_STATE.md's "Build status", and
   close or amend KI-01/KI-02 based on the result. If something in the table
   fails, write the diagnosis into KNOWN_ISSUES.md rather than leaving it.

### Task B — what a container session can do meanwhile

Scheduled daily sessions run headless with no engine. They cannot build,
open the editor, or author `.uasset`/`.umap` files, and must never claim
otherwise. Useful work that remains, in order:

1. **A `UFarmCameraMode_Bench` subclass.** Small, no art, and it is the only
   way to exercise the Phase 2 camera-mode blending before Phase 11 depends
   on it — right now `UFarmCameraMode_FreeWalk` is the only mode, so the
   stack's blend path has never run with two modes in it. Add an
   interactable bench that pushes the mode, and returns to Free Walk on a
   second press.
2. **Interaction polish:** `CanInteract` currently always returns true;
   consider a facing-angle check so the player must roughly face a thing,
   not merely have it under the crosshair.
3. **Validator coverage:** it does not yet check that `UPROPERTY` is followed
   by a declaration, or that `UCLASS` specifiers are well-formed.

Do not start Phases 5–12. Do not start Phase 3 (see below).

---

## Phase 3 — farm blockout: BLOCKED

See KNOWN_ISSUES.md **KI-03**.

`Docs/References/` is empty. Phase 3 reconstructs the farm's spatial layout
and there is nothing to reconstruct it from.

**Do not invent a farm layout to unblock this.** A guessed layout is worse
than none: it would be thrown away, and in the meantime it silently becomes
the reference for building scale, path widths and sight lines.

A session should check whether images have appeared in `Docs/References/`. If
they have, Phase 3 is unblocked and becomes the priority — the folder's
README lists what is most useful and why.
