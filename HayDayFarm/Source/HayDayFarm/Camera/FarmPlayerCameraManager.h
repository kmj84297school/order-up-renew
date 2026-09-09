// Copyright (c) 2026. Private personal project.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "FarmPlayerCameraManager.generated.h"

/**
 * Project camera manager.
 *
 * Right now its only job is to clamp look pitch to a comfortable range, but
 * it exists as a named class from the start because it is the correct home
 * for the camera-adjacent work in later phases: cinematic fades for the
 * Cinematic Tour, and any global post-process blending for time-of-day.
 */
UCLASS(NotBlueprintable)
class HAYDAYFARM_API AFarmPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:
	AFarmPlayerCameraManager();
};
