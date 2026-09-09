// Copyright (c) 2026. Private personal project.

#include "Interaction/FarmInteractable.h"

#define LOCTEXT_NAMESPACE "FarmInteraction"

FText IFarmInteractable::GetInteractionPrompt_Implementation() const
{
	return LOCTEXT("DefaultInteractionPrompt", "Interact");
}

bool IFarmInteractable::CanInteract_Implementation(AActor* Interactor) const
{
	return true;
}

void IFarmInteractable::OnInteract_Implementation(AActor* Interactor)
{
	// Deliberately empty: an interactable that does nothing is a valid
	// placeholder while the thing it will eventually do is being built.
}

void IFarmInteractable::OnInteractionFocusChanged_Implementation(bool bIsFocused)
{
}

#undef LOCTEXT_NAMESPACE
