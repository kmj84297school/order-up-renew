// Copyright (c) 2026. Private personal project.

#include "Camera/FarmCameraMode.h"
#include "Camera/FarmCameraComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

UFarmCameraMode::UFarmCameraMode()
{
	View.FieldOfView = FieldOfView;
}

UFarmCameraComponent* UFarmCameraMode::GetFarmCameraComponent() const
{
	// Modes are always constructed with the camera component as their outer.
	return CastChecked<UFarmCameraComponent>(GetOuter());
}

AActor* UFarmCameraMode::GetTargetActor() const
{
	const UFarmCameraComponent* Component = GetFarmCameraComponent();
	return Component ? Component->GetTargetActor() : nullptr;
}

APawn* UFarmCameraMode::GetTargetPawn() const
{
	return Cast<APawn>(GetTargetActor());
}

APlayerController* UFarmCameraMode::GetTargetPlayerController() const
{
	const APawn* Pawn = GetTargetPawn();
	return Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
}

void UFarmCameraMode::UpdateCameraMode(float DeltaTime)
{
	UpdateView(DeltaTime);
	UpdateBlending(DeltaTime);
}

void UFarmCameraMode::UpdateView(float DeltaTime)
{
	View.Location = GetPivotLocation();
	View.Rotation = GetPivotRotation();
	View.FieldOfView = FieldOfView;
}

FVector UFarmCameraMode::GetPivotLocation() const
{
	if (const APawn* Pawn = GetTargetPawn())
	{
		// Honours BaseEyeHeight and the crouch/eye-height interpolation the
		// movement component applies, so the view never sits inside the mesh.
		return Pawn->GetPawnViewLocation();
	}

	const AActor* TargetActor = GetTargetActor();
	return TargetActor ? TargetActor->GetActorLocation() : FVector::ZeroVector;
}

FRotator UFarmCameraMode::GetPivotRotation() const
{
	if (const APawn* Pawn = GetTargetPawn())
	{
		// GetViewRotation() returns the controller's control rotation for a
		// possessed pawn, which is what mouse look drives.
		return Pawn->GetViewRotation();
	}

	const AActor* TargetActor = GetTargetActor();
	return TargetActor ? TargetActor->GetActorRotation() : FRotator::ZeroRotator;
}

float UFarmCameraMode::GetTargetHorizontalSpeed() const
{
	const AActor* TargetActor = GetTargetActor();
	if (!TargetActor)
	{
		return 0.0f;
	}

	return TargetActor->GetVelocity().Size2D();
}

void UFarmCameraMode::SetBlendWeight(float Weight)
{
	BlendWeight = FMath::Clamp(Weight, 0.0f, 1.0f);

	// Invert the easing so that continuing to tick from here follows the same
	// curve it would have done had it blended to this weight naturally.
	const float SafeExponent = FMath::Max(BlendExponent, 1.0f);
	const float InverseExponent = 1.0f / SafeExponent;

	switch (BlendFunction)
	{
	case EFarmCameraBlendFunction::Linear:
		BlendAlpha = BlendWeight;
		break;

	case EFarmCameraBlendFunction::EaseIn:
		BlendAlpha = FMath::InterpEaseIn(0.0f, 1.0f, BlendWeight, InverseExponent);
		break;

	case EFarmCameraBlendFunction::EaseOut:
		BlendAlpha = FMath::InterpEaseOut(0.0f, 1.0f, BlendWeight, InverseExponent);
		break;

	case EFarmCameraBlendFunction::EaseInOut:
		BlendAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, BlendWeight, InverseExponent);
		break;

	default:
		checkf(false, TEXT("UFarmCameraMode: unhandled EFarmCameraBlendFunction."));
		BlendAlpha = BlendWeight;
		break;
	}
}

void UFarmCameraMode::UpdateBlending(float DeltaTime)
{
	if (BlendTime > UE_KINDA_SMALL_NUMBER)
	{
		BlendAlpha = FMath::Min(BlendAlpha + (DeltaTime / BlendTime), 1.0f);
	}
	else
	{
		BlendAlpha = 1.0f;
	}

	const float SafeExponent = FMath::Max(BlendExponent, 1.0f);

	switch (BlendFunction)
	{
	case EFarmCameraBlendFunction::Linear:
		BlendWeight = BlendAlpha;
		break;

	case EFarmCameraBlendFunction::EaseIn:
		BlendWeight = FMath::InterpEaseIn(0.0f, 1.0f, BlendAlpha, SafeExponent);
		break;

	case EFarmCameraBlendFunction::EaseOut:
		BlendWeight = FMath::InterpEaseOut(0.0f, 1.0f, BlendAlpha, SafeExponent);
		break;

	case EFarmCameraBlendFunction::EaseInOut:
		BlendWeight = FMath::InterpEaseInOut(0.0f, 1.0f, BlendAlpha, SafeExponent);
		break;

	default:
		checkf(false, TEXT("UFarmCameraMode: unhandled EFarmCameraBlendFunction."));
		BlendWeight = BlendAlpha;
		break;
	}
}
