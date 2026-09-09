// Copyright (c) 2026. Private personal project.

#include "Interaction/FarmInteractable_Nudge.h"
#include "Components/StaticMeshComponent.h"
#include "Core/FarmLog.h"

#define LOCTEXT_NAMESPACE "FarmInteraction"

AFarmInteractable_Nudge::AFarmInteractable_Nudge()
{
	// Can tick, but does not until something actually needs animating.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	Mesh->SetRelativeScale3D(FVector(0.6f, 0.6f, 0.6f));

	InteractionPrompt = LOCTEXT("NudgePrompt", "Touch");
}

void AFarmInteractable_Nudge::BeginPlay()
{
	Super::BeginPlay();

	RestRotation = GetActorRotation();
}

void AFarmInteractable_Nudge::OnInteract_Implementation(AActor* Interactor)
{
	if (Interactor)
	{
		// Lean away from whoever touched it. GetSafeNormal2D keeps the lean
		// horizontal even if the player is looking down at it.
		const FVector Away = (GetActorLocation() - Interactor->GetActorLocation()).GetSafeNormal2D();
		if (!Away.IsNearlyZero())
		{
			LeanDirection = Away;
		}
	}

	// Restarting mid-reaction is intentional: repeated presses should feel
	// responsive rather than queue up or be ignored.
	NudgeElapsed = 0.0f;
	SetActorTickEnabled(true);

	UE_LOG(LogFarm, Verbose, TEXT("%s nudged."), *GetName());
}

void AFarmInteractable_Nudge::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (NudgeElapsed < 0.0f)
	{
		StopNudge();
		return;
	}

	NudgeElapsed += DeltaSeconds;

	const float Alpha = NudgeElapsed / FMath::Max(NudgeDuration, UE_KINDA_SMALL_NUMBER);
	if (Alpha >= 1.0f)
	{
		StopNudge();
		return;
	}

	// Damped oscillation: amplitude falls off linearly while the sine runs,
	// so it settles back to rest instead of stopping mid-swing.
	const float Damping = 1.0f - Alpha;
	const float Angle = NudgeAngle * Damping * FMath::Sin(Alpha * NudgeOscillations * UE_TWO_PI);

	// Tilt about the axis perpendicular to the lean direction.
	const FVector TiltAxis = FVector::CrossProduct(FVector::UpVector, LeanDirection).GetSafeNormal();
	if (TiltAxis.IsNearlyZero())
	{
		StopNudge();
		return;
	}

	const FQuat Tilt(TiltAxis, FMath::DegreesToRadians(Angle));
	SetActorRotation(Tilt * RestRotation.Quaternion());
}

void AFarmInteractable_Nudge::StopNudge()
{
	NudgeElapsed = -1.0f;
	SetActorRotation(RestRotation);
	SetActorTickEnabled(false);
}

#undef LOCTEXT_NAMESPACE
