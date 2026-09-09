// Copyright (c) 2026. Private personal project.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "FarmCharacter.generated.h"

class UFarmCameraComponent;
class UFarmInputConfig;

/** Broadcast when the player presses the interact key. Phase 4's interaction
 *  component subscribes to this so the character never has to know what an
 *  interactable is. */
DECLARE_MULTICAST_DELEGATE(FFarmInteractPressedSignature);

/**
 * The player pawn for relaxed farm exploration.
 *
 * Responsibilities are deliberately narrow: translate Enhanced Input into
 * movement and look, and host the camera component. It owns no camera
 * behaviour (that is UFarmCameraMode) and no interaction logic (Phase 4).
 *
 * Movement is tuned well below shooter defaults. Unreal's stock character
 * walks at 600 cm/s with a 2048 cm/s^2 stop, which reads as urgent; the farm
 * walks at 200 cm/s and takes about a third of a second to reach it.
 */
UCLASS()
class HAYDAYFARM_API AFarmCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AFarmCharacter();

	UFUNCTION(BlueprintPure, Category = "Farm|Camera")
	UFarmCameraComponent* GetFarmCamera() const { return FarmCamera; }

	/** Fires on interact key press. Bound by the Phase 4 interaction system. */
	FFarmInteractPressedSignature OnInteractPressed;

protected:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	//~ Input handlers
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Interact(const FInputActionValue& Value);
	void ToggleMenu(const FInputActionValue& Value);
	void StartRunning(const FInputActionValue& Value);
	void StopRunning(const FInputActionValue& Value);

	/** Returns InputConfig when it is fully authored, otherwise a code-built
	 *  config. Never mutates the assigned asset. */
	UFarmInputConfig* ResolveInputConfig();

	/** Optional. Leave unset to use the built-in WASD/mouse defaults. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Farm|Input")
	TObjectPtr<UFarmInputConfig> InputConfig;

	/** Comfortable stroll. Roughly 2 m/s. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Farm|Movement", meta = (ClampMin = "10.0", ClampMax = "600.0"))
	float WalkSpeed = 200.0f;

	/** Used only while the run key is held, and only if bAllowRunning. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Farm|Movement", meta = (ClampMin = "10.0", ClampMax = "900.0"))
	float RunSpeed = 340.0f;

	/** Running is explicitly optional in this project; off keeps the pace calm. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Farm|Movement")
	bool bAllowRunning = true;

	/** Multiplier on mouse delta. Deliberately not smoothed -- see DECISIONS.md. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Farm|Look", meta = (ClampMin = "0.05", ClampMax = "5.0"))
	float LookSensitivity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Farm|Look")
	bool bInvertLookY = false;

private:
	/** First-person view. Behaviour comes from the camera mode stack. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Farm|Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UFarmCameraComponent> FarmCamera;

	/** The config actually in use this session (asset or generated). */
	UPROPERTY(Transient)
	TObjectPtr<UFarmInputConfig> ActiveInputConfig;
};
