// Copyright (c) 2026. Private personal project.

#include "Interaction/FarmInteractionComponent.h"
#include "Core/FarmLog.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Interaction/FarmInteractable.h"
#include "TimerManager.h"

namespace FarmInteractionInternal
{
	/** Stable key so repeated on-screen prompts replace rather than stack. */
	static constexpr uint64 DebugPromptKey = 0x4641524Du; // 'FARM'
}

UFarmInteractionComponent::UFarmInteractionComponent()
{
	// Focus runs on a timer instead. See the class comment.
	PrimaryComponentTick.bCanEverTick = false;
}

void UFarmInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			FocusTimerHandle, this, &UFarmInteractionComponent::RefreshFocus,
			FocusRefreshInterval, /*bLoop=*/true);
	}
}

void UFarmInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FocusTimerHandle);
	}

	// Anything still focused should hear that it no longer is, or a highlight
	// left on at level teardown reappears if the actor is reused.
	SetFocusedActor(nullptr);

	Super::EndPlay(EndPlayReason);
}

AActor* UFarmInteractionComponent::GetFocusedActor() const
{
	return FocusedActor.Get();
}

bool UFarmInteractionComponent::GetViewPoint(FVector& OutLocation, FVector& OutDirection) const
{
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		return false;
	}

	// The pawn's view point, not the camera component's: this stays correct
	// when a non-Free-Walk camera mode has moved the camera elsewhere, so
	// sitting on a bench does not let the player reach across the farm.
	OutLocation = OwnerPawn->GetPawnViewLocation();
	OutDirection = OwnerPawn->GetViewRotation().Vector();
	return true;
}

AActor* UFarmInteractionComponent::FindFocusTarget() const
{
	UWorld* World = GetWorld();
	FVector ViewLocation;
	FVector ViewDirection;

	if (!World || !GetViewPoint(ViewLocation, ViewDirection))
	{
		return nullptr;
	}

	const FVector TraceEnd = ViewLocation + (ViewDirection * InteractionRange);

	FCollisionQueryParams QueryParams(TEXT("FarmInteractionFocus"), /*bTraceComplex=*/false, GetOwner());

	FHitResult Hit;
	const bool bHit = World->SweepSingleByChannel(
		Hit, ViewLocation, TraceEnd, FQuat::Identity, TraceChannel,
		FCollisionShape::MakeSphere(InteractionRadius), QueryParams);

#if ENABLE_DRAW_DEBUG
	if (bDrawDebugTrace)
	{
		DrawDebugLine(World, ViewLocation, TraceEnd, bHit ? FColor::Green : FColor::Silver,
			false, FocusRefreshInterval, 0, 1.0f);
	}
#endif

	if (!bHit)
	{
		return nullptr;
	}

	AActor* HitActor = Hit.GetActor();
	if (!HitActor || !HitActor->Implements<UFarmInteractable>())
	{
		return nullptr;
	}

	// Ask before offering: an interactable that says no should show no prompt
	// at all, rather than a prompt that turns out to do nothing.
	if (!IFarmInteractable::Execute_CanInteract(HitActor, GetOwner()))
	{
		return nullptr;
	}

	return HitActor;
}

void UFarmInteractionComponent::RefreshFocus()
{
	SetFocusedActor(FindFocusTarget());

#if !UE_BUILD_SHIPPING
	if (bShowDebugPrompt && GEngine && FocusedActor.IsValid())
	{
		// Slightly longer than the refresh interval so it does not flicker
		// between refreshes.
		GEngine->AddOnScreenDebugMessage(
			FarmInteractionInternal::DebugPromptKey, FocusRefreshInterval * 1.5f,
			FColor::White, FString::Printf(TEXT("[E]  %s"), *FocusedPrompt.ToString()));
	}
#endif
}

void UFarmInteractionComponent::SetFocusedActor(AActor* NewFocus)
{
	AActor* PreviousFocus = FocusedActor.Get();
	if (PreviousFocus == NewFocus)
	{
		return;
	}

	if (PreviousFocus)
	{
		IFarmInteractable::Execute_OnInteractionFocusChanged(PreviousFocus, false);
	}

	FocusedActor = NewFocus;
	FocusedPrompt = NewFocus
		? IFarmInteractable::Execute_GetInteractionPrompt(NewFocus)
		: FText::GetEmpty();

	if (NewFocus)
	{
		IFarmInteractable::Execute_OnInteractionFocusChanged(NewFocus, true);
	}

	OnFocusChanged.Broadcast(NewFocus, FocusedPrompt);

	UE_LOG(LogFarm, Verbose, TEXT("Interaction focus: %s"), *GetNameSafe(NewFocus));
}

void UFarmInteractionComponent::TryInteract()
{
	// Focus can be up to FocusRefreshInterval stale, and the player may have
	// looked away in between, so re-test rather than trusting the cache.
	AActor* Target = FindFocusTarget();
	SetFocusedActor(Target);

	if (!Target)
	{
		return;
	}

	UE_LOG(LogFarm, Log, TEXT("Interacting with %s."), *GetNameSafe(Target));
	IFarmInteractable::Execute_OnInteract(Target, GetOwner());
}
