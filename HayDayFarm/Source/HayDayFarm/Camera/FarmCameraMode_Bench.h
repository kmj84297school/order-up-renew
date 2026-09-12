// Copyright (c) 2026. Private personal project.

#pragma once

#include "CoreMinimal.h"
#include "Camera/FarmCameraMode.h"
#include "FarmCameraMode_Bench.generated.h"

/** Fixed seated eye position with responsive mouse look and no walking sway. */
UCLASS()
class HAYDAYFARM_API UFarmCameraMode_Bench : public UFarmCameraMode
{
	GENERATED_BODY()

public:
	UFarmCameraMode_Bench();
	virtual void OnActivation() override;

protected:
	virtual void UpdateView(float DeltaTime) override;

private:
	// Retained during blend-out, after the interaction session has ended.
	FVector SeatedLocation = FVector::ZeroVector;
};
