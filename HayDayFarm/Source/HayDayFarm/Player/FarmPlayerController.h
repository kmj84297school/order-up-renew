// Copyright (c) 2026. Private personal project.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FarmPlayerController.generated.h"

/**
 * Player controller for the farm.
 *
 * Owns input *mode* (is the mouse captured for looking, or free for a menu?)
 * rather than input *bindings*, which live on the character. Keeping the two
 * apart means a future Cinematic Tour can release the cursor without the
 * character knowing anything about it.
 */
UCLASS()
class HAYDAYFARM_API AFarmPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AFarmPlayerController();

	/** Captures the mouse for looking and hides the cursor. */
	UFUNCTION(BlueprintCallable, Category = "Farm|Input")
	void EnterExplorationInputMode();

	/** Releases the mouse so UI can be clicked. */
	UFUNCTION(BlueprintCallable, Category = "Farm|Input")
	void EnterMenuInputMode();

	/** Swaps between the two modes above. Bound to Esc by the character. */
	UFUNCTION(BlueprintCallable, Category = "Farm|Input")
	void ToggleMenuInputMode();

	UFUNCTION(BlueprintPure, Category = "Farm|Input")
	bool IsInMenuInputMode() const { return bMenuInputMode; }

protected:
	virtual void BeginPlay() override;

private:
	bool bMenuInputMode = false;
};
