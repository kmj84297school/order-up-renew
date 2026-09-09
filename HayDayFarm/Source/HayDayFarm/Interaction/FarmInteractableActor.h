// Copyright (c) 2026. Private personal project.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/FarmInteractable.h"
#include "FarmInteractableActor.generated.h"

class UStaticMeshComponent;

/**
 * Convenience base for interactables that are a single mesh in the world.
 *
 * Optional -- IFarmInteractable is the real contract and anything can
 * implement it. This exists so the common case (a prop with a prompt and a
 * focus highlight) is not retyped for every crate and bench.
 */
UCLASS(Abstract)
class HAYDAYFARM_API AFarmInteractableActor : public AActor, public IFarmInteractable
{
	GENERATED_BODY()

public:
	AFarmInteractableActor();

protected:
	virtual void BeginPlay() override;

	//~ Begin IFarmInteractable
	virtual FText GetInteractionPrompt_Implementation() const override;
	virtual void OnInteractionFocusChanged_Implementation(bool bIsFocused) override;
	//~ End IFarmInteractable

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Farm|Interaction")
	FText InteractionPrompt;

	/** Scale applied while focused. A placeholder highlight -- Phase 10
	 *  replaces it with a post-process outline driven by custom depth, which
	 *  is already written below but has no outline material yet. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Farm|Interaction", meta = (ClampMin = "1.0", ClampMax = "1.5"))
	float FocusScaleMultiplier = 1.04f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Farm|Interaction")
	TObjectPtr<UStaticMeshComponent> Mesh;

private:
	/** Captured at BeginPlay so a level-authored scale is preserved. */
	FVector UnfocusedScale = FVector::OneVector;
};
