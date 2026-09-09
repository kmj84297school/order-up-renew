"""
Creates Content/Levels/L_FarmBlockout.umap.

A .umap is a binary asset and can only be produced by the Unreal Editor, so
this script is the project's substitute for checking one in. It is run once,
on a machine with Unreal installed, and produces the level that
DefaultEngine.ini's GameDefaultMap already points at.

Headless:
    "<UE>\\Engine\\Binaries\\Win64\\UnrealEditor-Cmd.exe" ^
        "<repo>\\HayDayFarm\\HayDayFarm.uproject" ^
        -run=pythonscript -script="<repo>\\HayDayFarm\\Tools\\generate_bootstrap_level.py"

In the editor:
    Tools > Execute Python Script..., then pick this file.

Safe to re-run: it refuses to overwrite an existing level unless you pass
OVERWRITE = True below, so it cannot destroy hand-authored work.
"""

import unreal

LEVEL_PACKAGE = "/Game/Levels/L_FarmBlockout"
OVERWRITE = False

PLAYER_START_LOCATION = unreal.Vector(0.0, 0.0, 120.0)
PLAYER_START_ROTATION = unreal.Rotator(0.0, 0.0, 0.0)


def log(message):
    unreal.log("[HayDayFarm] {}".format(message))


def main():
    level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

    if unreal.EditorAssetLibrary.does_asset_exist(LEVEL_PACKAGE) and not OVERWRITE:
        log("{} already exists; nothing to do. Set OVERWRITE = True to rebuild it."
            .format(LEVEL_PACKAGE))
        return

    log("Creating empty level {}".format(LEVEL_PACKAGE))
    if not level_subsystem.new_level(LEVEL_PACKAGE):
        unreal.log_error("[HayDayFarm] Failed to create {}".format(LEVEL_PACKAGE))
        return

    # AFarmWorldBootstrap carries the ground, sun, sky light, atmosphere and
    # fog as components, so placing this one actor yields a lit, walkable
    # world with no other assets involved.
    bootstrap = actor_subsystem.spawn_actor_from_class(
        unreal.FarmWorldBootstrap, unreal.Vector(0.0, 0.0, 0.0), unreal.Rotator(0.0, 0.0, 0.0))
    if bootstrap:
        bootstrap.set_actor_label("FarmWorldBootstrap_PLACEHOLDER")
        log("Placed placeholder world.")

    # AFarmGameMode can spawn a transient PlayerStart at runtime, but an
    # authored one means the level behaves the same in PIE and in a package.
    player_start = actor_subsystem.spawn_actor_from_class(
        unreal.PlayerStart, PLAYER_START_LOCATION, PLAYER_START_ROTATION)
    if player_start:
        player_start.set_actor_label("PlayerStart_FarmEntrance")
        log("Placed PlayerStart at {}".format(PLAYER_START_LOCATION))

    level_subsystem.save_current_level()
    log("Saved {}. Set it as the editor/game default map if it is not already.".format(LEVEL_PACKAGE))


if __name__ == "__main__":
    main()
