// Copyright (c) 2026. Private personal project.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FarmWorldBootstrap.generated.h"

class UDirectionalLightComponent;
class UExponentialHeightFogComponent;
class USkyAtmosphereComponent;
class USkyLightComponent;
class UStaticMeshComponent;

/**
 * PLACEHOLDER. A minimal lit, walkable world so the project is playable
 * before any level has been authored.
 *
 * Everything here is a default subobject rather than a runtime-spawned actor,
 * so nothing depends on deferred spawning or mobility fixups, and the whole
 * thing is one actor to delete once Phase 3 produces a real blockout level.
 *
 * Ground geometry uses /Engine/BasicShapes/Plane. That is engine content, not
 * project content -- it needs no authored asset and is guaranteed present in
 * every Unreal install.
 *
 * The GameMode spawns this automatically when a level does not already
 * contain one. See AFarmGameMode::bSpawnPlaceholderWorldIfMissing.
 */
UCLASS()
class HAYDAYFARM_API AFarmWorldBootstrap : public AActor
{
	GENERATED_BODY()

public:
	AFarmWorldBootstrap();

	/** Half-extent of the placeholder ground in cm. Read by the GameMode when
	 *  deciding where a fallback PlayerStart is safe to put. */
	static constexpr float GroundHalfExtent = 6000.0f;

protected:
	/** 120 m x 120 m of flat ground with collision. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Farm|Placeholder")
	TObjectPtr<UStaticMeshComponent> Ground;

	/** Warm late-afternoon sun. Phase 7 replaces this with the time-of-day
	 *  presets; the angle here is a deliberate stand-in, not a final look. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Farm|Placeholder")
	TObjectPtr<UDirectionalLightComponent> SunLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Farm|Placeholder")
	TObjectPtr<USkyLightComponent> SkyLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Farm|Placeholder")
	TObjectPtr<USkyAtmosphereComponent> SkyAtmosphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Farm|Placeholder")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Farm|Placeholder")
	TObjectPtr<UExponentialHeightFogComponent> HeightFog;
};
