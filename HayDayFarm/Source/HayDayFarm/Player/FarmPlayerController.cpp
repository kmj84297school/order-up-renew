// Copyright (c) 2026. Private personal project.

#include "Player/FarmPlayerController.h"
#include "Camera/FarmPlayerCameraManager.h"
#include "Core/FarmLog.h"

AFarmPlayerController::AFarmPlayerController()
{
	PlayerCameraManagerClass = AFarmPlayerCameraManager::StaticClass();
}

void AFarmPlayerController::BeginPlay()
{
	Super::BeginPlay();

	EnterExplorationInputMode();
}

void AFarmPlayerController::EnterExplorationInputMode()
{
	bMenuInputMode = false;

	FInputModeGameOnly InputMode;
	InputMode.SetConsumeCaptureMouseDown(true);
	SetInputMode(InputMode);

	bShowMouseCursor = false;

	UE_LOG(LogFarmPlayer, Verbose, TEXT("Input mode: exploration (mouse captured)."));
}

void AFarmPlayerController::EnterMenuInputMode()
{
	bMenuInputMode = true;

	// GameAndUI rather than UIOnly: the world keeps running and the ambience
	// keeps playing behind an open menu, which suits a relaxing experience.
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);

	bShowMouseCursor = true;

	UE_LOG(LogFarmPlayer, Verbose, TEXT("Input mode: menu (cursor visible)."));
}

void AFarmPlayerController::ToggleMenuInputMode()
{
	if (bMenuInputMode)
	{
		EnterExplorationInputMode();
	}
	else
	{
		EnterMenuInputMode();
	}
}
