// Copyright (c) 2026. Private personal project.

#include "Camera/FarmCameraComponent.h"
#include "Camera/FarmCameraMode.h"
#include "Camera/FarmCameraModeStack.h"
#include "Camera/FarmCameraTypes.h"
#include "Core/FarmLog.h"
#include "GameFramework/Actor.h"

UFarmCameraComponent::UFarmCameraComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// The mode stack drives the transform, so letting the base class also
	// apply the pawn's control rotation would double-apply it.
	bUsePawnControlRotation = false;

	CameraModeStack = ObjectInitializer.CreateDefaultSubobject<UFarmCameraModeStack>(this, TEXT("CameraModeStack"));
}

UFarmCameraComponent* UFarmCameraComponent::FindFarmCameraComponent(const AActor* Actor)
{
	return Actor ? Actor->FindComponentByClass<UFarmCameraComponent>() : nullptr;
}

void UFarmCameraComponent::PushCameraMode(TSubclassOf<UFarmCameraMode> ModeClass)
{
	if (CameraModeStack && ModeClass)
	{
		CameraModeStack->PushCameraMode(ModeClass);
	}
}

void UFarmCameraComponent::ReturnToDefaultCameraMode()
{
	PushCameraMode(DefaultCameraModeClass);
}

void UFarmCameraComponent::SetDefaultCameraModeClass(TSubclassOf<UFarmCameraMode> ModeClass)
{
	DefaultCameraModeClass = ModeClass;
}

void UFarmCameraComponent::GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView)
{
	if (!CameraModeStack)
	{
		Super::GetCameraView(DeltaTime, DesiredView);
		return;
	}

	if (CameraModeStack->IsEmpty())
	{
		if (!DefaultCameraModeClass)
		{
			// No modes configured at all: behave like a stock camera rather
			// than leaving the player looking at the world origin.
			UE_LOG(LogFarmCamera, Warning,
				TEXT("%s has no DefaultCameraModeClass set; falling back to UCameraComponent."),
				*GetNameSafe(GetOwner()));
			Super::GetCameraView(DeltaTime, DesiredView);
			return;
		}

		CameraModeStack->PushCameraMode(DefaultCameraModeClass);
	}

	FFarmCameraView CameraModeView;
	if (!CameraModeStack->EvaluateStack(DeltaTime, CameraModeView))
	{
		Super::GetCameraView(DeltaTime, DesiredView);
		return;
	}

	// Keep the component transform in sync so anything parented to the camera
	// (audio listener, post-process, future photo-mode widgets) follows it.
	SetWorldLocationAndRotation(CameraModeView.Location, CameraModeView.Rotation);
	FieldOfView = CameraModeView.FieldOfView;

	DesiredView.Location = CameraModeView.Location;
	DesiredView.Rotation = CameraModeView.Rotation;
	DesiredView.FOV = CameraModeView.FieldOfView;
	DesiredView.OrthoWidth = OrthoWidth;
	DesiredView.OrthoNearClipPlane = OrthoNearClipPlane;
	DesiredView.OrthoFarClipPlane = OrthoFarClipPlane;
	DesiredView.AspectRatio = AspectRatio;
	DesiredView.bConstrainAspectRatio = bConstrainAspectRatio;
	DesiredView.bUseFieldOfViewForLOD = bUseFieldOfViewForLOD;
	DesiredView.ProjectionMode = ProjectionMode;

	DesiredView.PostProcessBlendWeight = PostProcessBlendWeight;
	if (PostProcessBlendWeight > 0.0f)
	{
		DesiredView.PostProcessSettings = PostProcessSettings;
	}
}
