// Copyright (c) 2026. Private personal project.

#include "Interaction/FarmInteractable_Rotator.h"
#include "Components/StaticMeshComponent.h"
#include "Core/FarmLog.h"
#include "GameFramework/RotatingMovementComponent.h"

#define LOCTEXT_NAMESPACE "FarmInteraction"

AFarmInteractable_Rotator::AFarmInteractable_Rotator()
{
	RotatingMovement = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingMovement"));
	RotatingMovement->RotationRate = SpinRate;

	// A tall thin box stands in for a windmill so the rotation is legible
	// against the ground plane.
	Mesh->SetRelativeScale3D(FVector(0.35f, 0.35f, 2.0f));

	StartPrompt = LOCTEXT("StartWindmill", "Start the windmill");
	StopPrompt = LOCTEXT("StopWindmill", "Stop the windmill");
	InteractionPrompt = StartPrompt;
}

void AFarmInteractable_Rotator::BeginPlay()
{
	Super::BeginPlay();

	RotatingMovement->RotationRate = SpinRate;
	SetSpinning(bStartsSpinning);
}

void AFarmInteractable_Rotator::SetSpinning(bool bNewSpinning)
{
	bSpinning = bNewSpinning;

	// Disabling the component's tick is unambiguous; SetActive has extra
	// semantics in UMovementComponent that are easy to get subtly wrong.
	RotatingMovement->SetComponentTickEnabled(bSpinning);
}

FText AFarmInteractable_Rotator::GetInteractionPrompt_Implementation() const
{
	return bSpinning ? StopPrompt : StartPrompt;
}

void AFarmInteractable_Rotator::OnInteract_Implementation(AActor* Interactor)
{
	SetSpinning(!bSpinning);

	UE_LOG(LogFarm, Log, TEXT("%s is now %s."), *GetName(), bSpinning ? TEXT("spinning") : TEXT("stopped"));
}

#undef LOCTEXT_NAMESPACE
