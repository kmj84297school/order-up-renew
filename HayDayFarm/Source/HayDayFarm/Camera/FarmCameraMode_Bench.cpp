// Copyright (c) 2026. Private personal project.

#include "Camera/FarmCameraMode_Bench.h"
#include "GameFramework/Actor.h"
#include "Interaction/FarmInteractionComponent.h"

UFarmCameraMode_Bench::UFarmCameraMode_Bench()
{
	BlendTime = 0.75f;
	FieldOfView = 75.0f;
}

void UFarmCameraMode_Bench::OnActivation()
{
	Super::OnActivation();
	SeatedLocation = GetPivotLocation();
}

void UFarmCameraMode_Bench::UpdateView(float DeltaTime)
{
	if (const AActor* Target = GetTargetActor())
	{
		if (const UFarmInteractionComponent* Interaction = Target->FindComponentByClass<UFarmInteractionComponent>())
		{
			if (Interaction->IsSeated())
			{
				SeatedLocation = Interaction->GetSeatedViewLocation();
			}
		}
	}

	View.Location = SeatedLocation;
	View.Rotation = GetPivotRotation();
	View.FieldOfView = FieldOfView;
}
