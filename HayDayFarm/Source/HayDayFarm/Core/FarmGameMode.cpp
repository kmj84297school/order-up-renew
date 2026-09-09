// Copyright (c) 2026. Private personal project.

#include "Core/FarmGameMode.h"
#include "Core/FarmLog.h"
#include "EngineUtils.h"
#include "Environment/FarmWorldBootstrap.h"
#include "GameFramework/PlayerStart.h"
#include "Player/FarmCharacter.h"
#include "Player/FarmPlayerController.h"

AFarmGameMode::AFarmGameMode()
{
	DefaultPawnClass = AFarmCharacter::StaticClass();
	PlayerControllerClass = AFarmPlayerController::StaticClass();
	PlaceholderWorldClass = AFarmWorldBootstrap::StaticClass();

	// The camera manager class is set by AFarmPlayerController, not here --
	// PlayerCameraManagerClass lives on APlayerController, not AGameModeBase.
	//
	// Nothing about this experience needs a HUD, spectator or player state
	// beyond the engine defaults, so they are left alone deliberately.
}

void AFarmGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (!bSpawnPlaceholderWorldIfMissing || !PlaceholderWorldClass)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<AFarmWorldBootstrap> It(World); It; ++It)
	{
		UE_LOG(LogFarmEnvironment, Log,
			TEXT("Level already contains %s; skipping the placeholder world."), *It->GetName());
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.ObjectFlags |= RF_Transient;

	AActor* Placeholder = World->SpawnActor<AFarmWorldBootstrap>(
		PlaceholderWorldClass, FTransform::Identity, SpawnParams);

	UE_LOG(LogFarmEnvironment, Warning,
		TEXT("No level geometry found. Spawned the PLACEHOLDER world (%s). ")
		TEXT("Replace this with the Phase 3 blockout level."),
		*GetNameSafe(Placeholder));
}

AActor* AFarmGameMode::FindPlayerStart_Implementation(AController* Player, const FString& IncomingName)
{
	if (AActor* AuthoredStart = Super::FindPlayerStart_Implementation(Player, IncomingName))
	{
		return AuthoredStart;
	}

	if (IsValid(FallbackPlayerStart))
	{
		return FallbackPlayerStart;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	// Without this the engine simply refuses to spawn a pawn in a level that
	// has no PlayerStart, which is every level until one is authored.
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.ObjectFlags |= RF_Transient;

	FallbackPlayerStart = World->SpawnActor<APlayerStart>(
		APlayerStart::StaticClass(), FallbackSpawnLocation, FRotator::ZeroRotator, SpawnParams);

	UE_LOG(LogFarm, Warning,
		TEXT("Level has no PlayerStart. Spawned a transient fallback at %s."),
		*FallbackSpawnLocation.ToCompactString());

	return FallbackPlayerStart;
}
