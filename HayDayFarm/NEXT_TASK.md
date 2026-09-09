# NEXT_TASK

Written for a session with no memory of previous conversations. Read
PROJECT_STATE.md, DECISIONS.md and KNOWN_ISSUES.md first, then this.

---

## Objective: get the project to compile and run for the first time

Phases 1 and 2 are written but have **never been built**. Everything below is
gated on that. Do not start Phase 3 until the project runs.

This task requires a machine with Unreal Engine installed. It cannot be done
in a headless Linux container.

### Current state

- 13 headers, 13 sources under `Source/HayDayFarm/`.
- `python3 Tools/validate_project.py` passes (0 errors). Run it first — it
  is fast and catches structural mistakes before UnrealHeaderTool does.
- No `Content/` assets, no `.umap`, no `Binaries/`, no `Intermediate/`.

### Steps

1. **Confirm the engine version.** `HayDayFarm.uproject` says `"5.4"`. If a
   different 5.x is installed, change that field, and change
   `EngineIncludeOrderVersion.Unreal5_4` in both
   `Source/HayDayFarm.Target.cs` and `Source/HayDayFarmEditor.Target.cs`.

2. **Generate project files.** Right-click `HayDayFarm.uproject` →
   *Generate Visual Studio project files*.

3. **Build** the `HayDayFarmEditor` target, `Development Editor | Win64`.

4. **Fix compile errors.** Three specific lines are unverified against real
   engine headers and are the most likely failures. They are listed in
   KNOWN_ISSUES.md as **KI-01**, and are all in
   `Source/HayDayFarm/Environment/FarmWorldBootstrap.cpp`:
   - `SunLight->bAtmosphereSunLight = true;`
   - `SkyLight->SourceType = ESkyLightSourceType::SLS_CapturedScene;`
   - `SkyLight->bRealTimeCapture = true;`
   If any of these do not exist under the installed engine version, find the
   correct member/setter in the engine headers and fix it. Do **not** delete
   the sky setup to make the build pass — without the sun link the sky
   renders wrong, and Phase 7 depends on this working.

5. **Create the level.** Once the editor opens:
   ```
   "<UE>\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" ^
       "<repo>\HayDayFarm\HayDayFarm.uproject" ^
       -run=pythonscript -script="<repo>\HayDayFarm\Tools\generate_bootstrap_level.py"
   ```
   or in the editor: *Tools → Execute Python Script…* and pick
   `Tools/generate_bootstrap_level.py`. This script is **unverified**
   (KI-02); if the subsystem API differs, place the two actors by hand
   instead — an `AFarmWorldBootstrap` at the origin and a `PlayerStart` at
   `(0, 0, 120)` — and save as `Content/Levels/L_FarmBlockout`.

6. **Play in editor and check each item:**

   | Check | Expected |
   |---|---|
   | Player spawns | Standing on the ground plane, not falling or embedded |
   | W A S D | Moves relative to where the camera is looking |
   | Diagonals | No faster than a straight line |
   | Mouse | Looks around; **up moves the view up** (if inverted, see D-04) |
   | Pitch limit | Stops at roughly ±78°, no flipping |
   | Accel / decel | ~⅓ s to reach speed, soft stop; no sliding, no snapping |
   | Camera height | ~162 cm; a 2 m fence should read as clearly above eye line |
   | Head sway | Barely perceptible while walking, fully still when stopped |
   | Shift | Speeds up, and returns to walk speed on release |
   | Esc | Toggles the mouse cursor (in PIE, Esc may also stop play) |
   | E | Logs `Interact pressed` under `LogFarmPlayer` at Verbose |
   | Output log | No errors from `LogFarm*` categories |

7. **Record the result.** Update PROJECT_STATE.md's "Build status" section
   with what actually happened, and clear or amend KI-01/KI-02 in
   KNOWN_ISSUES.md based on what the build showed.

### Completion criteria

- Editor target builds with no errors.
- Editor opens the project.
- PIE runs, the player spawns on the ground and can walk and look.
- No `LogFarm*` errors in the output log.
- Every row of the table in step 6 passes, or is written up in
  KNOWN_ISSUES.md with a diagnosis.

---

## Then: Phase 3 — farm blockout

**Blocked.** See KNOWN_ISSUES.md KI-03.

Phase 3 reconstructs the farm's spatial layout, and there is currently
nothing to reconstruct it *from*. Reference imagery goes in
`Docs/References/` — that folder's README lists what is most useful.

Do not invent a farm layout to unblock this. A guessed layout is worse than
none: it would have to be thrown away and it would silently become the
reference for building scale, path widths and sight lines.

**Work that can proceed without references**, in preference order:

1. Everything in the "get it to compile" task above.
2. **Phase 4 — interaction framework.** Fully unblocked. The seam already
   exists: `AFarmCharacter::OnInteractPressed` is a multicast delegate that
   currently has no listeners. Build `UFarmInteractionComponent` (trace for
   targets, range check, prompt) and an `IFarmInteractable` interface, with
   two test interactables. This needs no art.
3. **A `UFarmCameraMode_Bench` subclass** to prove the camera stack blends
   correctly between modes. Small, and it validates the Phase 2
   architecture before Phase 11 depends on it.

Phases 5–12 should not start before 3 and 4.
