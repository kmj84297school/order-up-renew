// Copyright (c) 2026. Private personal project.

#include "Camera/FarmCameraModeStack.h"
#include "Camera/FarmCameraMode.h"
#include "Core/FarmLog.h"

void UFarmCameraModeStack::PushCameraMode(TSubclassOf<UFarmCameraMode> ModeClass)
{
	if (!ModeClass)
	{
		UE_LOG(LogFarmCamera, Warning, TEXT("PushCameraMode called with a null mode class."));
		return;
	}

	UFarmCameraMode* Mode = GetOrCreateCameraMode(ModeClass);
	if (!Mode)
	{
		return;
	}

	const int32 StackSize = CameraModeStack.Num();
	if (StackSize > 0 && CameraModeStack[0] == Mode)
	{
		// Already the active mode; pushing again must not restart the blend.
		return;
	}

	// Work out how much of the final image this mode is currently responsible
	// for, so re-pushing a mode that is halfway blended out resumes from what
	// the player can actually see rather than jumping back to zero.
	int32 ExistingIndex = INDEX_NONE;
	float ExistingContribution = 1.0f;

	for (int32 Index = 0; Index < StackSize; ++Index)
	{
		if (CameraModeStack[Index] == Mode)
		{
			ExistingIndex = Index;
			ExistingContribution *= Mode->GetBlendWeight();
			break;
		}

		// Modes above it hide it in proportion to their own weight.
		ExistingContribution *= (1.0f - CameraModeStack[Index]->GetBlendWeight());
	}

	if (ExistingIndex != INDEX_NONE)
	{
		CameraModeStack.RemoveAt(ExistingIndex);
	}
	else
	{
		ExistingContribution = 0.0f;
		Mode->OnActivation();
	}

	// An empty stack has nothing to blend from, so the first mode is instant.
	const bool bShouldBlend = (StackSize > 0) && (Mode->GetBlendTime() > 0.0f);
	Mode->SetBlendWeight(bShouldBlend ? ExistingContribution : 1.0f);

	CameraModeStack.Insert(Mode, 0);

	// The bottom of the stack is what everything else blends over, so it must
	// be fully opaque or the blended result darkens toward an undefined view.
	CameraModeStack.Last()->SetBlendWeight(1.0f);

	UE_LOG(LogFarmCamera, Verbose, TEXT("Pushed camera mode %s (starting weight %.2f)."),
		*GetNameSafe(Mode), Mode->GetBlendWeight());
}

UFarmCameraMode* UFarmCameraModeStack::GetOrCreateCameraMode(TSubclassOf<UFarmCameraMode> ModeClass)
{
	for (UFarmCameraMode* Instance : CameraModeInstances)
	{
		if (Instance && Instance->GetClass() == ModeClass)
		{
			return Instance;
		}
	}

	// The camera component is the stack's outer, and modes expect the camera
	// component as *their* outer (see UFarmCameraMode::GetFarmCameraComponent).
	UFarmCameraMode* NewMode = NewObject<UFarmCameraMode>(GetOuter(), ModeClass, NAME_None, RF_NoFlags);
	CameraModeInstances.Add(NewMode);
	return NewMode;
}

bool UFarmCameraModeStack::EvaluateStack(float DeltaTime, FFarmCameraView& OutView)
{
	if (CameraModeStack.Num() == 0)
	{
		return false;
	}

	UpdateStack(DeltaTime);
	BlendStack(OutView);
	return true;
}

void UFarmCameraModeStack::UpdateStack(float DeltaTime)
{
	const int32 StackSize = CameraModeStack.Num();

	for (int32 Index = 0; Index < StackSize; ++Index)
	{
		CameraModeStack[Index]->UpdateCameraMode(DeltaTime);
	}

	// Once a mode is fully blended in, everything below it is invisible.
	for (int32 Index = 0; Index < StackSize; ++Index)
	{
		if (CameraModeStack[Index]->GetBlendWeight() >= 1.0f)
		{
			const int32 FirstRemoved = Index + 1;
			for (int32 RemoveIndex = FirstRemoved; RemoveIndex < StackSize; ++RemoveIndex)
			{
				CameraModeStack[RemoveIndex]->OnDeactivation();
			}

			if (FirstRemoved < StackSize)
			{
				CameraModeStack.RemoveAt(FirstRemoved, StackSize - FirstRemoved);
			}
			break;
		}
	}
}

void UFarmCameraModeStack::BlendStack(FFarmCameraView& OutView) const
{
	const int32 StackSize = CameraModeStack.Num();
	if (StackSize == 0)
	{
		return;
	}

	// Start at the bottom (fully opaque by definition) and blend upward.
	OutView = CameraModeStack.Last()->GetView();

	for (int32 Index = StackSize - 2; Index >= 0; --Index)
	{
		const UFarmCameraMode* Mode = CameraModeStack[Index];
		OutView.Blend(Mode->GetView(), Mode->GetBlendWeight());
	}
}

TSubclassOf<UFarmCameraMode> UFarmCameraModeStack::GetTopCameraModeClass() const
{
	return CameraModeStack.Num() > 0 ? CameraModeStack[0]->GetClass() : nullptr;
}
