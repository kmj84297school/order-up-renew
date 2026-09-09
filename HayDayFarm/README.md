# HayDay Farm — UE5

A high-quality stylized 3D farm you can walk around in, built in Unreal
Engine 5. The goal is atmosphere and presence rather than simulation: enter
the farm, walk at a relaxed pace, look at things, interact with a handful of
objects and animals.

Private personal project. The environment is reconstructed by hand from
reference imagery using original and legally obtained assets — no assets are
extracted from the original game.

## Status

**Phases 0–2 written. Never compiled.** This project was authored in a
headless Linux container with no Unreal Engine installation, so no build,
editor session or PIE run has happened yet.

Read these four files before doing anything, in this order:

| File | What it holds |
|---|---|
| [PROJECT_STATE.md](PROJECT_STATE.md) | Where the project is right now |
| [NEXT_TASK.md](NEXT_TASK.md) | The exact next objective, self-contained |
| [DECISIONS.md](DECISIONS.md) | Technical decisions and why — do not silently redo these |
| [KNOWN_ISSUES.md](KNOWN_ISSUES.md) | Bugs, placeholders, blockers |

## Getting it running

Requires Unreal Engine 5.4 on Windows. See NEXT_TASK.md for the full
first-build checklist, including the three lines most likely to fail.

```bash
python3 Tools/validate_project.py     # fast structural check, no engine needed
```

Then: right-click `HayDayFarm.uproject` → *Generate Visual Studio project
files* → build `HayDayFarmEditor | Development Editor | Win64`.

There is no level in the repository yet (`.umap` is binary and needs the
editor). Create it with:

```
UnrealEditor-Cmd.exe HayDayFarm.uproject -run=pythonscript ^
    -script="<repo>\HayDayFarm\Tools\generate_bootstrap_level.py"
```

The project is built to run with **zero content assets** — if no level, no
input asset and no lighting exist, code-defined defaults stand in and say so
in the log. See DECISIONS.md D-03.

## Controls

| Input | Action |
|---|---|
| W A S D / arrows | Move |
| Mouse | Look |
| E | Interact *(seam only — Phase 4 implements it)* |
| Esc | Toggle mouse cursor |
| Left Shift | Run (optional) |

No jump, by design.

## Layout

```
Config/          Engine, game, input and editor .ini
Content/         Assets. Currently empty except Levels/
Docs/References/ Drop Hay Day reference imagery here — see its README
Source/          C++ module (Core, Player, Camera, Environment)
Tools/           validate_project.py, generate_bootstrap_level.py
```

## Contributing to this project across sessions

Development happens over many sessions, any of which may end abruptly. The
rule is that the repository, not any conversation, is the source of truth.
After a meaningful milestone: validate, then update PROJECT_STATE.md,
NEXT_TASK.md and KNOWN_ISSUES.md so the next session can pick up cold.
