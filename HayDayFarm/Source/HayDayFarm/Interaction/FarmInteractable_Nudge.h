// Copyright (c) 2026. Private personal project.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/FarmInteractableActor.h"
#include "FarmInteractable_Nudge.generated.h"

/**
 * Leans away from the player and springs back.
 *
 * The second Phase 4 test interactable, and the prototype for the small
 * tactile reactions the brief asks for from crops and animals: a brief,
 * damped movement with no state left behind.
 *
 * Ticks only while the reaction is playing -- the actor starts with tick
 * disabled, enables it on interact, and disables it again when the motion
 * settles. See DECISIONS.md D-08.
 */
UCLASS()
class HAYDAYFARM_API AFarmInteractable_Nudge : public AFarmInteractableActor
{
	GENERATED_BODY()

public:
	AFarmInteractable_Nudge();

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

	virtual void OnInteract_Implementation(AActor* Interactor) override;

	/** Peak lean in degrees. Small: this should read as a nudge, not a shove. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Farm|Nudge", meta = (ClampMin = "0.5", ClampMax = "45.0"))
	float NudgeAngle = 9.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Farm|Nudge", meta = (ClampMin = "0.1"))
	float NudgeDuration = 0.75f;

	/** Full oscillations before it settles. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Farm|Nudge", meta = (ClampMin = "0.5"))
	float NudgeOscillations = 1.75f;

private:
	void StopNudge();

	/** Rotation to return to. Captured at BeginPlay. */
	FRotator RestRotation = FRotator::ZeroRotator;

	/** Horizontal direction to lean toward (away from the interactor). */
	FVector LeanDirection = FVector::ForwardVector;

	/** Seconds into the reaction; negative means not animating. */
	float NudgeElapsed = -1.0f;
};
