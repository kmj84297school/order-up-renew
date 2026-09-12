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

Current audit (2026-09-12): Windows host, Epic Launcher present, no registered
or default-folder UE 5.x installation found. Static validation is clean on
20 headers + 20 sources. Reference directory now contains 13 unique PNGs, a manifest, an analysis README and RESEARCH.md.
**First build takes priority even if reference images become available.**

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

6. **Play in editor and check each row, then run `Docs/BENCH_VERIFICATION.md`:**

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

1. **Bench view is now written, still unverified.** The exact implementation
   is in `Camera/FarmCameraMode_Bench.*`, `Interaction/FarmInteractable_Bench.*`,
   `Interaction/FarmInteractionComponent.*`, and the bootstrap spawn in
   `Environment/FarmWorldBootstrap.cpp`. On the first available engine build,
   execute every step in `Docs/BENCH_VERIFICATION.md`, including interrupted
   blends, destroyed seats, possession changes and balanced input locks.
   Done means the editor target builds and the checks pass; record build
   success separately from runtime failures. Do not claim this is done from
   the static validator alone.
2. **Interaction polish:** `CanInteract` currently always returns true;
   consider a facing-angle check so the player must roughly face a thing,
   not merely have it under the crosshair.
3. **Validator coverage:** run `python Tools/test_validate_project.py` along
   with the normal validator. Missing Farm includes are now covered (KI-15); it does not yet check that `UPROPERTY` is followed
   by a declaration, or that `UCLASS` specifiers are well-formed.

Do not start Phases 5–12. Phase 3 reference analysis is now possible; engine blockout follows first build.

---

## Phase 3 — references received; engine blockout follows first build

KI-03's missing-reference condition is resolved as of 2026-09-12.
Read Docs/References/README.md, manifest.json and RESEARCH.md.
Image 4 is the primary local adjacency reference; 5/6 inform paths and planting.
Do not reconstruct image 3 as a single farm or merge different screenshots as
if they were a surveyed plan. No scale measurements are supplied.

After Task A passes, produce a limited core blockout (road, house/barn/silo,
field and pens) preserving observed adjacency; mark inferred dimensions.
Validate collision, clear walking routes and sight lines at 162 cm eye height.
No authored level or gameplay check has happened in the reference intake.
Hidden building faces and sunset reference remain gaps; additional search
sources are recorded in RESEARCH.md without claiming their videos were watched.
