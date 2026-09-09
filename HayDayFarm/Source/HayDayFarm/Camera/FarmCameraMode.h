// Copyright (c) 2026. Private personal project.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Camera/FarmCameraTypes.h"
#include "FarmCameraMode.generated.h"

class AActor;
class APawn;
class APlayerController;
class UFarmCameraComponent;

/**
 * Base class for a single way of framing the world.
 *
 * A mode is a plain UObject, not an actor or component, so adding Cinematic
 * Tour / Bench View / Photo Mode later means adding a subclass and pushing it
 * -- no change to the Character, Controller or the camera component itself.
 * This is the reason camera behaviour deliberately does not live on
 * AFarmCharacter.
 *
 * Subclasses override UpdateView() and write into View.
 */
UCLASS(Abstract, NotBlueprintable, BlueprintType)
class HAYDAYFARM_API UFarmCameraMode : public UObject
{
	GENERATED_BODY()

public:
	UFarmCameraMode();

	/** Owning camera component. Valid for the whole lifetime of the mode. */
	UFarmCameraComponent* GetFarmCameraComponent() const;

	/** The actor the camera is framing (the possessed pawn, in Free Walk). */
	AActor* GetTargetActor() const;

	/** Convenience accessors; may return null while the pawn is unpossessed. */
	APawn* GetTargetPawn() const;
	APlayerController* GetTargetPlayerController() const;

	/** Recomputes View for this frame, then advances the blend weight. */
	void UpdateCameraMode(float DeltaTime);

	const FFarmCameraView& GetView() const { return View; }
	float GetBlendWeight() const { return BlendWeight; }
	float GetBlendTime() const { return BlendTime; }

	/**
	 * Sets the blend weight directly and back-solves BlendAlpha to match.
	 *
	 * Needed because mode instances are cached and reused: a mode that was
	 * fully blended in, then dropped, would otherwise still hold BlendAlpha
	 * of 1.0 and snap straight back in when pushed again instead of blending.
	 */
	void SetBlendWeight(float Weight);

	/** Called when the mode is pushed onto / removed from the stack. */
	virtual void OnActivation() {}
	virtual void OnDeactivation() {}

protected:
	/** Subclass hook: fill in View for this frame. */
	virtual void UpdateView(float DeltaTime);

	/** Eye-height view point of the target, or its origin if it is not a pawn. */
	FVector GetPivotLocation() const;

	/** Control rotation of the target's controller, or the actor's rotation. */
	FRotator GetPivotRotation() const;

	/** Horizontal (XY) speed of the target in cm/s. Drives motion-based effects. */
	float GetTargetHorizontalSpeed() const;

	/** The view produced by this mode for the current frame. */
	FFarmCameraView View;

	/** Vertical FOV in degrees. 75 keeps the farm readable without the
	 *  wide-angle distortion that makes stylised buildings look wrong. */
	UPROPERTY(EditDefaultsOnly, Category = "Farm|Camera", meta = (ClampMin = "40.0", ClampMax = "120.0"))
	float FieldOfView = 75.0f;

	/** Seconds to blend in. 0 snaps. */
	UPROPERTY(EditDefaultsOnly, Category = "Farm|Camera|Blending", meta = (ClampMin = "0.0"))
	float BlendTime = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Farm|Camera|Blending")
	EFarmCameraBlendFunction BlendFunction = EFarmCameraBlendFunction::EaseInOut;

	/** Exponent for the Ease* blend functions. Higher = longer flat start. */
	UPROPERTY(EditDefaultsOnly, Category = "Farm|Camera|Blending", meta = (ClampMin = "1.0"))
	float BlendExponent = 2.0f;

	/** Linear 0..1 progress through the blend. */
	float BlendAlpha = 0.0f;

	/** BlendAlpha with BlendFunction applied. This is what the stack uses. */
	float BlendWeight = 0.0f;

private:
	void UpdateBlending(float DeltaTime);
};
