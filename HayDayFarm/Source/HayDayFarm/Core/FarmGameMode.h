// Copyright (c) 2026. Private personal project.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FarmGameMode.generated.h"

class AFarmWorldBootstrap;

/**
 * Game mode for the farm.
 *
 * There is no match, no score and no rules to arbitrate, so this class does
 * only two things, both of which exist to keep the project runnable while
 * content is still being built:
 *
 *  - spawns a placeholder lit ground plane when the level has none
 *  - provides a fallback spawn point when the level has no PlayerStart
 *
 * Both behaviours switch themselves off the moment a real level supplies the
 * equivalent, so Phase 3 does not have to come back and remove them.
 */
UCLASS()
class HAYDAYFARM_API AFarmGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFarmGameMode();

protected:
	virtual void BeginPlay() override;
	virtual AActor* FindPlayerStart_Implementation(AController* Player, const FString& IncomingName) override;

	/** Spawn the placeholder world if the level does not contain one. Turn
	 *  this off once a real blockout level exists. */
	UPROPERTY(EditDefaultsOnly, Category = "Farm|Placeholder")
	bool bSpawnPlaceholderWorldIfMissing = true;

	UPROPERTY(EditDefaultsOnly, Category = "Farm|Placeholder")
	TSubclassOf<AFarmWorldBootstrap> PlaceholderWorldClass;

	/** Where the player appears when the level has no PlayerStart. Stands
	 *  clear of the placeholder ground plane's surface. */
	UPROPERTY(EditDefaultsOnly, Category = "Farm|Placeholder")
	FVector FallbackSpawnLocation = FVector(0.0f, 0.0f, 120.0f);

private:
	/** Kept alive so repeated respawns reuse one transient start actor. */
	UPROPERTY(Transient)
	TObjectPtr<AActor> FallbackPlayerStart;
};
