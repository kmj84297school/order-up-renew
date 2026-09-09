// Copyright (c) 2026. Private personal project.

#pragma once

#include "CoreMinimal.h"
#include "Camera/FarmCameraMode.h"
#include "FarmCameraMode_FreeWalk.generated.h"

/**
 * Eye-level first-person view for relaxed exploration.
 *
 * Deliberately understated: a very small step sway that fades in with walking
 * speed and fades out to nothing when standing still. The brief calls for no
 * aggressive camera effects, so every amplitude here is roughly an order of
 * magnitude below a typical FPS head-bob, and the whole effect can be turned
 * off with bEnableStepSway.
 */
UCLASS()
class HAYDAYFARM_API UFarmCameraMode_FreeWalk : public UFarmCameraMode
{
	GENERATED_BODY()

public:
	UFarmCameraMode_FreeWalk();

	virtual void OnActivation() override;

protected:
	virtual void UpdateView(float DeltaTime) override;

	/** Master switch for the walking sway. */
	UPROPERTY(EditDefaultsOnly, Category = "Farm|Camera|Sway")
	bool bEnableStepSway = true;

	/** Full steps per second at ReferenceSpeed. */
	UPROPERTY(EditDefaultsOnly, Category = "Farm|Camera|Sway", meta = (ClampMin = "0.1", EditCondition = "bEnableStepSway"))
	float StepFrequency = 1.05f;

	/** Speed (cm/s) at which the sway reaches full strength. Matches the
	 *  character's default walk speed. */
	UPROPERTY(EditDefaultsOnly, Category = "Farm|Camera|Sway", meta = (ClampMin = "1.0", EditCondition = "bEnableStepSway"))
	float ReferenceSpeed = 200.0f;

	/** Vertical travel in cm. Two of these happen per full step cycle. */
	UPROPERTY(EditDefaultsOnly, Category = "Farm|Camera|Sway", meta = (ClampMin = "0.0", ClampMax = "6.0", EditCondition = "bEnableStepSway"))
	float VerticalAmplitude = 1.2f;

	/** Side-to-side travel in cm. */
	UPROPERTY(EditDefaultsOnly, Category = "Farm|Camera|Sway", meta = (ClampMin = "0.0", ClampMax = "6.0", EditCondition = "bEnableStepSway"))
	float LateralAmplitude = 0.9f;

	/** Roll in degrees. Kept tiny; roll is the fastest way to cause discomfort. */
	UPROPERTY(EditDefaultsOnly, Category = "Farm|Camera|Sway", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "bEnableStepSway"))
	float RollAmplitude = 0.12f;

	/** How quickly the sway fades in and out as the player starts/stops. */
	UPROPERTY(EditDefaultsOnly, Category = "Farm|Camera|Sway", meta = (ClampMin = "0.1", EditCondition = "bEnableStepSway"))
	float SwayFadeSpeed = 3.5f;

	/** Below this speed (cm/s) the sway is treated as fully faded out. */
	UPROPERTY(EditDefaultsOnly, Category = "Farm|Camera|Sway", meta = (ClampMin = "0.0", EditCondition = "bEnableStepSway"))
	float SwayMinSpeed = 15.0f;

private:
	/** Advances the sway phase and returns the current 0..1 strength. */
	float UpdateSwayState(float DeltaTime, float HorizontalSpeed);

	/** Radians. Wrapped to keep float precision stable over long sessions. */
	float SwayPhase = 0.0f;

	/** Smoothed 0..1 sway strength. */
	float SwayStrength = 0.0f;
};
