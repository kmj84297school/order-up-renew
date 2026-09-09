// Copyright (c) 2026. Private personal project.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "FarmInteractionComponent.generated.h"

/** Focus changed. NewFocus is null when nothing is focused. */
DECLARE_MULTICAST_DELEGATE_TwoParams(FFarmFocusChangedSignature, AActor* /*NewFocus*/, const FText& /*Prompt*/);

/**
 * Finds what the player is looking at and routes interact presses to it.
 *
 * Runs on a timer (about 12 times a second) rather than every frame. Focus
 * detection at 60 Hz is imperceptibly different from 12 Hz -- the player's
 * head does not move that fast -- and a sphere sweep every frame for the
 * whole session is exactly the kind of cost this project should not pay.
 * See DECISIONS.md D-08.
 */
UCLASS(ClassGroup = (Farm), meta = (BlueprintSpawnableComponent))
class HAYDAYFARM_API UFarmInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFarmInteractionComponent();

	/** Fires when the focused interactable changes, not every refresh. */
	FFarmFocusChangedSignature OnFocusChanged;

	/** Interacts with whatever is focused. Bound to the character's
	 *  OnInteractPressed delegate. Safe to call with nothing focused. */
	UFUNCTION(BlueprintCallable, Category = "Farm|Interaction")
	void TryInteract();

	UFUNCTION(BlueprintPure, Category = "Farm|Interaction")
	AActor* GetFocusedActor() const;

	UFUNCTION(BlueprintPure, Category = "Farm|Interaction")
	FText GetFocusedPrompt() const { return FocusedPrompt; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** How far the player can reach. 250 cm is about two and a half strides:
	 *  close enough that you have walked up to something deliberately. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Farm|Interaction", meta = (ClampMin = "20.0"))
	float InteractionRange = 250.0f;

	/** Sweep radius. A pure line trace demands pixel-accurate aiming, which
	 *  is at odds with a relaxed experience; this forgives a near miss. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Farm|Interaction", meta = (ClampMin = "0.0"))
	float InteractionRadius = 14.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Farm|Interaction", meta = (ClampMin = "0.02"))
	float FocusRefreshInterval = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Farm|Interaction")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	/** PLACEHOLDER prompt display: prints the prompt on screen. Replaced by a
	 *  real UMG widget in the UI phase. See KNOWN_ISSUES.md KI-10. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Farm|Interaction|Debug")
	bool bShowDebugPrompt = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Farm|Interaction|Debug")
	bool bDrawDebugTrace = false;

private:
	void RefreshFocus();

	/** The sweep. Returns the focused actor, or null. */
	AActor* FindFocusTarget() const;

	/** Eye position and look direction of the owning pawn. */
	bool GetViewPoint(FVector& OutLocation, FVector& OutDirection) const;

	void SetFocusedActor(AActor* NewFocus);

	/** Weak: an interactable can be destroyed while focused. */
	TWeakObjectPtr<AActor> FocusedActor;

	FText FocusedPrompt;

	FTimerHandle FocusTimerHandle;
};
