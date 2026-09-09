// Copyright (c) 2026. Private personal project.

#include "Camera/FarmPlayerCameraManager.h"

AFarmPlayerCameraManager::AFarmPlayerCameraManager()
{
	// Stopping short of straight up/down keeps the horizon in frame and
	// avoids the disorienting gimbal-ish feel at the poles. There is nothing
	// directly overhead in the farm that needs looking at.
	ViewPitchMin = -78.0f;
	ViewPitchMax = 78.0f;
}
