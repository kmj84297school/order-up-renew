// Copyright (c) 2026. Private personal project.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/FarmInteractableActor.h"
#include "FarmInteractable_Rotator.generated.h"

class URotatingMovementComponent;

/**
 * Toggles continuous rotation on and off.
 *
 * A test interactable for Phase 4, and the working prototype of the windmill
 * from Phase 6 -- the brief asks for exactly this ("Windmill: start/stop
 * rotation"), so it is built as the real mechanism rather than a throwaway.
 * Swapping the placeholder cube for a windmill mesh is all that is left.
 *
 * Rotation is done by URotatingMovementComponent, so the actor itself never
 * ticks.
 */
UCLASS()
class HAYDAYFARM_API AFarmInteractable_Rotator : public AFarmInteractableActor
{
	GENERATED_BODY()

public:
	AFarmInteractable_Rotator();

protected:
	virtual void BeginPlay() override;

	virtual FText GetInteractionPrompt_Implementation() const override;
	virtual void OnInteract_Implementation(AActor* Interactor) override;

	/** Degrees per second. Slow by default: a fast spin reads as machinery,
	 *  a slow one reads as a breeze. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Farm|Rotator")
	FRotator SpinRate = FRotator(0.0f, 0.0f, 35.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Farm|Rotator")
	bool bStartsSpinning = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Farm|Rotator")
	FText StartPrompt;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Farm|Rotator")
	FText StopPrompt;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Farm|Rotator")
	TObjectPtr<URotatingMovementComponent> RotatingMovement;

private:
	void SetSpinning(bool bNewSpinning);

	bool bSpinning = false;
};
