// Copyright (c) 2026. Private personal project.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "Templates/SubclassOf.h"
#include "FarmCameraComponent.generated.h"

class UFarmCameraMode;
class UFarmCameraModeStack;

/**
 * The single camera for the farm. It owns a stack of camera modes and asks
 * that stack -- never the owning actor -- where the view should be.
 *
 * To add a new way of viewing the world (Cinematic Tour, Bench View, Photo
 * Mode) write a UFarmCameraMode subclass and call PushCameraMode. Nothing in
 * the Character or PlayerController needs to change.
 */
UCLASS(ClassGroup = (Farm), meta = (BlueprintSpawnableComponent))
class HAYDAYFARM_API UFarmCameraComponent : public UCameraComponent
{
	GENERATED_BODY()

public:
	UFarmCameraComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Finds the farm camera on an actor, if it has one. */
	UFUNCTION(BlueprintPure, Category = "Farm|Camera")
	static UFarmCameraComponent* FindFarmCameraComponent(const AActor* Actor);

	/** The actor being framed. Camera modes read their pivot from this. */
	AActor* GetTargetActor() const { return GetOwner(); }

	/** Blends to ModeClass. Safe to call every frame with the same class. */
	UFUNCTION(BlueprintCallable, Category = "Farm|Camera")
	void PushCameraMode(TSubclassOf<UFarmCameraMode> ModeClass);

	/** Returns to whatever DefaultCameraModeClass is set to (Free Walk). */
	UFUNCTION(BlueprintCallable, Category = "Farm|Camera")
	void ReturnToDefaultCameraMode();

	UFUNCTION(BlueprintPure, Category = "Farm|Camera")
	TSubclassOf<UFarmCameraMode> GetDefaultCameraModeClass() const { return DefaultCameraModeClass; }

	UFUNCTION(BlueprintCallable, Category = "Farm|Camera")
	void SetDefaultCameraModeClass(TSubclassOf<UFarmCameraMode> ModeClass);

	//~ Begin UCameraComponent interface
	virtual void GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView) override;
	//~ End UCameraComponent interface

protected:
	/** Pushed automatically the first time the camera is evaluated. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Farm|Camera")
	TSubclassOf<UFarmCameraMode> DefaultCameraModeClass;

private:
	/** Not Transient: this is a default subobject, recreated by the
	 *  constructor, and marking it transient confuses subobject fixup. */
	UPROPERTY()
	TObjectPtr<UFarmCameraModeStack> CameraModeStack;
};
