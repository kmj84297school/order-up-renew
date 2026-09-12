// Copyright (c) 2026. Private personal project.

#include "Interaction/FarmInteractable_Bench.h"
#include "Components/StaticMeshComponent.h"
#include "Interaction/FarmInteractionComponent.h"

#define LOCTEXT_NAMESPACE "FarmBench"

AFarmInteractable_Bench::AFarmInteractable_Bench()
{
	// 160 x 55 x 45 cm seat block; no authored content is required.
	Mesh->SetRelativeScale3D(FVector(0.55f, 1.6f, 0.45f));
	FocusScaleMultiplier = 1.0f;
	InteractionPrompt = LOCTEXT("Sit", "Sit on bench");
}

bool AFarmInteractable_Bench::CanInteract_Implementation(AActor* Interactor) const
{
	const UFarmInteractionComponent* Interaction = Interactor
		? Interactor->FindComponentByClass<UFarmInteractionComponent>() : nullptr;
	return Interaction && !Interaction->IsSeated();
}

void AFarmInteractable_Bench::OnInteract_Implementation(AActor* Interactor)
{
	if (!CanInteract_Implementation(Interactor))
	{
		return;
	}

	UFarmInteractionComponent* Interaction = Interactor->FindComponentByClass<UFarmInteractionComponent>();
	Interaction->BeginSeatedView(this, GetActorLocation() + FVector(0.0f, 0.0f, EyeOffset));
}

#undef LOCTEXT_NAMESPACE
