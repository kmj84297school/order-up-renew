# HayDay Farm — UE5

A high-quality stylized 3D farm you can walk around in, built in Unreal
Engine 5. The goal is atmosphere and presence rather than simulation: enter
the farm, walk at a relaxed pace, look at things, interact with a handful of
objects and animals.

Private personal project. The environment is reconstructed by hand from
reference imagery using original and legally obtained assets — no assets are
extracted from the original game.

## Status

**Phases 0–2 and 4 written. Never compiled.** This project was authored in a
headless Linux container with no Unreal Engine installation, so no build,
editor session or PIE run has happened yet. Unreal is not yet installed on the
developer's machine either — that is the current top blocker.

Read these four files before doing anything, in this order:

| File | What it holds |
|---|---|
| [RUNNING.md](RUNNING.md) | **How to actually build and run it** (Korean) |
| [PROJECT_STATE.md](PROJECT_STATE.md) | Where the project is right now |
| [NEXT_TASK.md](NEXT_TASK.md) | The exact next objective, self-contained |
| [DECISIONS.md](DECISIONS.md) | Technical decisions and why — do not silently redo these |
| [KNOWN_ISSUES.md](KNOWN_ISSUES.md) | Bugs, placeholders, blockers |

## Getting it running

Needs Windows, Visual Studio 2022 with the C++ workload, and Unreal Engine
5.4. **[RUNNING.md](RUNNING.md) is the step-by-step guide.** The short version:

```powershell
.\Tools\Setup.ps1     # locate engine, build, create the level, open the editor
```

For a structural check with no engine involved (works anywhere):

```bash
python3 Tools/validate_project.py
```

There is no level in the repository (`.umap` is binary and needs the editor);
`Setup.ps1` generates it, and RUNNING.md section 3 covers doing it by hand.

The project is built to run with **zero content assets** — if no level, no
input asset and no lighting exist, code-defined defaults stand in and say so
in the log. See DECISIONS.md D-03.

## Controls

| Input | Action |
|---|---|
| W A S D / arrows | Move |
| Mouse | Look |
| E | Interact with whatever you are looking at |
| Esc | Toggle mouse cursor |
| Left Shift | Run (optional) |

No jump, by design.

## Layout

```
Config/          Engine, game, input and editor .ini
Content/         Assets. Currently empty except Levels/
Docs/References/ Drop Hay Day reference imagery here — see its README
Source/          C++ module (Core, Player, Camera, Interaction, Environment)
Tools/           validate_project.py, generate_bootstrap_level.py
```

## Contributing to this project across sessions

Development happens over many sessions, any of which may end abruptly. The
rule is that the repository, not any conversation, is the source of truth.
After a meaningful milestone: validate, then update PROJECT_STATE.md,
NEXT_TASK.md and KNOWN_ISSUES.md so the next session can pick up cold.
