// Copyright (c) 2026. Private personal project.

#include "Camera/FarmCameraTypes.h"

void FFarmCameraView::Blend(const FFarmCameraView& Other, float Weight)
{
	if (Weight <= 0.0f)
	{
		return;
	}

	if (Weight >= 1.0f)
	{
		*this = Other;
		return;
	}

	Location = FMath::Lerp(Location, Other.Location, Weight);

	// Rotators must go through the shortest-arc delta, otherwise a blend that
	// crosses the +/-180 yaw seam whips the camera all the way around.
	const FRotator DeltaRotation = (Other.Rotation - Rotation).GetNormalized();
	Rotation = Rotation + (DeltaRotation * Weight);

	FieldOfView = FMath::Lerp(FieldOfView, Other.FieldOfView, Weight);
}
