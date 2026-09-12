// Copyright (c) 2026. Private personal project.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/FarmInteractableActor.h"
#include "FarmInteractable_Bench.generated.h"

/** PLACEHOLDER seat geometry; the pawn stays at its safe approach position. */
UCLASS()
class HAYDAYFARM_API AFarmInteractable_Bench : public AFarmInteractableActor
{
	GENERATED_BODY()

public:
	AFarmInteractable_Bench();

protected:
	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual void OnInteract_Implementation(AActor* Interactor) override;

	/** Centimetres above the seat actor origin, independent of mesh scale. */
	UPROPERTY(EditAnywhere, Category = "Farm|Bench", meta = (ClampMin = "50.0"))
	float EyeOffset = 95.0f;
};
