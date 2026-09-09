// Copyright (c) 2026. Private personal project.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FarmInputConfig.generated.h"

class UInputAction;
class UInputMappingContext;

/**
 * The set of Enhanced Input actions the farm needs, plus the mapping context
 * that binds them to keys.
 *
 * Two ways to supply this:
 *
 *  1. Author a UFarmInputConfig data asset in the editor and assign it on the
 *     character. This is the long-term path -- designers can rebind keys
 *     without a recompile.
 *
 *  2. Leave it unset. CreateRuntimeConfig() builds an equivalent set of
 *     actions and a WASD/mouse/E/Esc mapping context entirely in C++.
 *
 * Path 2 exists so the project boots and is playable with no content assets
 * at all. That is what makes the module verifiable independently of any
 * .uasset, and it is why nothing here holds a hard reference to an asset.
 */
UCLASS(BlueprintType)
class HAYDAYFARM_API UFarmInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Axis2D. X = forward/back, Y = right/left. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Farm|Input")
	TObjectPtr<UInputAction> MoveAction;

	/** Axis2D. X = yaw delta, Y = pitch delta (already negated for mouse). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Farm|Input")
	TObjectPtr<UInputAction> LookAction;

	/** Boolean, fires once on press. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Farm|Input")
	TObjectPtr<UInputAction> InteractAction;

	/** Boolean, held. Optional by design -- running is not a priority. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Farm|Input")
	TObjectPtr<UInputAction> RunAction;

	/** Boolean, fires once on press. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Farm|Input")
	TObjectPtr<UInputAction> MenuAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Farm|Input")
	TObjectPtr<UInputMappingContext> MappingContext;

	/** Priority the context is added to the Enhanced Input subsystem with. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Farm|Input")
	int32 MappingContextPriority = 0;

	/** True when every action and the mapping context are assigned. */
	UFUNCTION(BlueprintPure, Category = "Farm|Input")
	bool IsComplete() const;

	/** Human-readable list of the fields that are still null. For logging. */
	FString DescribeMissingFields() const;

	/**
	 * Builds a complete, code-defined config (actions + WASD/mouse mapping
	 * context) owned by Outer.
	 *
	 * Deliberately ignores any partially-filled asset rather than patching it:
	 * a half-authored asset silently merged with generated defaults is far
	 * harder to debug than an all-or-nothing rule.
	 */
	static UFarmInputConfig* CreateRuntimeConfig(UObject* Outer);
};
