// Copyright (c) 2026. Private personal project.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Camera/FarmCameraTypes.h"
#include "FarmCameraModeStack.generated.h"

class UFarmCameraMode;

/**
 * An ordered stack of camera modes, blended bottom-to-top.
 *
 * Index 0 is the top (newest) mode. Pushing a mode blends it in over its own
 * BlendTime; once it reaches full weight the modes underneath it are dropped,
 * so the stack stays short even after a long session of entering and leaving
 * benches, tours and photo mode.
 *
 * Mode instances are cached and reused per class, which keeps a mode's
 * internal state (head-bob phase, smoothed rotation) continuous across
 * repeated pushes instead of popping on every re-entry.
 */
UCLASS()
class HAYDAYFARM_API UFarmCameraModeStack : public UObject
{
	GENERATED_BODY()

public:
	/** Brings ModeClass to the top of the stack, blending it in. */
	void PushCameraMode(TSubclassOf<UFarmCameraMode> ModeClass);

	/** Ticks every mode and produces the blended result. Returns false if the
	 *  stack is empty, in which case OutView is untouched. */
	bool EvaluateStack(float DeltaTime, FFarmCameraView& OutView);

	/** Class of the mode currently on top, or null if the stack is empty. */
	TSubclassOf<UFarmCameraMode> GetTopCameraModeClass() const;

	bool IsEmpty() const { return CameraModeStack.Num() == 0; }

private:
	/** Returns the cached instance for ModeClass, creating it on first use. */
	UFarmCameraMode* GetOrCreateCameraMode(TSubclassOf<UFarmCameraMode> ModeClass);

	void UpdateStack(float DeltaTime);
	void BlendStack(FFarmCameraView& OutView) const;

	/** Index 0 == top of stack. */
	UPROPERTY()
	TArray<TObjectPtr<UFarmCameraMode>> CameraModeStack;

	/** One instance per mode class, kept alive so state survives re-entry. */
	UPROPERTY()
	TArray<TObjectPtr<UFarmCameraMode>> CameraModeInstances;
};
