// Copyright (c) 2026. Private personal project.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FarmInteractable.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UFarmInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * Anything the player can look at and press E on.
 *
 * An interface rather than a base actor class on purpose: farm interactables
 * will be all sorts of things -- a windmill, a bench, a chicken, a crop
 * patch -- and forcing them to share an inheritance chain would mean the
 * chicken inheriting from something that knows about static meshes.
 *
 * Every function is a BlueprintNativeEvent, so an interactable can be built
 * in C++ or entirely in Blueprint. The defaults are sensible enough that a
 * minimal implementation only overrides OnInteract.
 */
class HAYDAYFARM_API IFarmInteractable
{
	GENERATED_BODY()

public:
	/** Shown while the player is looking at this. Keep it short: "Sit",
	 *  "Start the windmill", "Pet the cow". */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Farm|Interaction")
	FText GetInteractionPrompt() const;

	/** Whether interaction is currently allowed. A false result hides the
	 *  prompt entirely rather than showing a prompt that does nothing. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Farm|Interaction")
	bool CanInteract(AActor* Interactor) const;

	/** Do the thing. Called once per press. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Farm|Interaction")
	void OnInteract(AActor* Interactor);

	/** Focus entered or left. The hook for highlight effects; Phase 10 gives
	 *  it a real post-process outline. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Farm|Interaction")
	void OnInteractionFocusChanged(bool bIsFocused);
};
