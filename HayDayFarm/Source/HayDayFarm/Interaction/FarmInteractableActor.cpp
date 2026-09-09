// Copyright (c) 2026. Private personal project.

#include "Interaction/FarmInteractableActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

#define LOCTEXT_NAMESPACE "FarmInteraction"

AFarmInteractableActor::AFarmInteractableActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	// Movable so the focus highlight can rescale it, and so subclasses can
	// rotate or tilt the actor at runtime.
	Mesh->SetMobility(EComponentMobility::Movable);
	Mesh->SetCollisionProfileName(TEXT("BlockAll"));

	// PLACEHOLDER geometry. Engine content, so no authored asset is needed.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMeshFinder.Succeeded())
	{
		Mesh->SetStaticMesh(CubeMeshFinder.Object);
	}

	InteractionPrompt = LOCTEXT("DefaultPropPrompt", "Interact");
}

void AFarmInteractableActor::BeginPlay()
{
	Super::BeginPlay();

	UnfocusedScale = Mesh->GetRelativeScale3D();
}

FText AFarmInteractableActor::GetInteractionPrompt_Implementation() const
{
	return InteractionPrompt.IsEmpty()
		? IFarmInteractable::GetInteractionPrompt_Implementation()
		: InteractionPrompt;
}

void AFarmInteractableActor::OnInteractionFocusChanged_Implementation(bool bIsFocused)
{
	Mesh->SetRelativeScale3D(bIsFocused ? UnfocusedScale * FocusScaleMultiplier : UnfocusedScale);

	// The real highlight. Inert until Phase 10 adds an outline post-process
	// material that reads custom depth -- harmless in the meantime.
	Mesh->SetRenderCustomDepth(bIsFocused);
}

#undef LOCTEXT_NAMESPACE
