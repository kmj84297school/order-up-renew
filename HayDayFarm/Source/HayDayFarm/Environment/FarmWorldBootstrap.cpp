// Copyright (c) 2026. Private personal project.

#include "Environment/FarmWorldBootstrap.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

AFarmWorldBootstrap::AFarmWorldBootstrap()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SceneRoot->SetMobility(EComponentMobility::Movable);
	SetRootComponent(SceneRoot);

	Ground = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Ground"));
	Ground->SetupAttachment(SceneRoot);
	// Movable, not Static: the GameMode spawns this actor at runtime, and
	// SpawnActor sets a transform, which Unreal refuses to do on a static
	// component. Lumen lights it dynamically either way, so nothing is lost.
	Ground->SetMobility(EComponentMobility::Movable);
	Ground->SetCollisionProfileName(TEXT("BlockAll"));

	// The engine plane is 100 x 100 cm, so this yields a 120 m square.
	const float GroundScale = (GroundHalfExtent * 2.0f) / 100.0f;
	Ground->SetRelativeScale3D(FVector(GroundScale, GroundScale, 1.0f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMeshFinder(TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (PlaneMeshFinder.Succeeded())
	{
		Ground->SetStaticMesh(PlaneMeshFinder.Object);
	}

	SunLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLight"));
	SunLight->SetupAttachment(SceneRoot);
	SunLight->SetMobility(EComponentMobility::Movable);
	SunLight->Intensity = 8.0f;
	SunLight->LightColor = FColor(255, 244, 224);

	// A low-ish sun rakes across the ground and gives buildings long, readable
	// shadows -- the closest single angle to the warm Hay Day look.
	SunLight->SetRelativeRotation(FRotator(-38.0f, -35.0f, 0.0f));

	// Drives the sky colour of the SkyAtmosphere below.
	SunLight->bAtmosphereSunLight = true;

	SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphere"));
	SkyAtmosphere->SetupAttachment(SceneRoot);
	SkyAtmosphere->SetMobility(EComponentMobility::Movable);

	SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
	SkyLight->SetupAttachment(SceneRoot);
	SkyLight->SetMobility(EComponentMobility::Movable);

	// Real-time capture of the atmosphere means ambient light stays correct
	// when Phase 7 starts moving the sun, with no cubemap asset to author.
	SkyLight->SourceType = ESkyLightSourceType::SLS_CapturedScene;
	SkyLight->bRealTimeCapture = true;
	SkyLight->Intensity = 1.0f;

	HeightFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("HeightFog"));
	HeightFog->SetupAttachment(SceneRoot);
	HeightFog->SetMobility(EComponentMobility::Movable);

	// Light haze only. Enough to separate distant objects and soften the
	// horizon without the scene reading as murky.
	HeightFog->FogDensity = 0.006f;
	HeightFog->FogHeightFalloff = 0.15f;
	HeightFog->StartDistance = 800.0f;
}
