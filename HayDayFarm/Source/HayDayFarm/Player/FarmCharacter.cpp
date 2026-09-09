// Copyright (c) 2026. Private personal project.

#include "Player/FarmCharacter.h"
#include "Camera/FarmCameraComponent.h"
#include "Camera/FarmCameraMode_FreeWalk.h"
#include "Components/CapsuleComponent.h"
#include "Core/FarmLog.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputMappingContext.h"
#include "Interaction/FarmInteractionComponent.h"
#include "Player/FarmInputConfig.h"
#include "Player/FarmPlayerController.h"

AFarmCharacter::AFarmCharacter()
{
	// Nothing here needs per-frame work: movement is driven by the movement
	// component and the camera by GetCameraView, which the camera manager
	// already calls once a frame.
	PrimaryActorTick.bCanEverTick = false;

	// 90 cm half-height with a 72 cm eye offset puts the view at about 162 cm,
	// which is adult eye level. Building and fence scale in Phase 3 is judged
	// against this number, so it should not be changed casually.
	GetCapsuleComponent()->InitCapsuleSize(34.0f, 90.0f);
	BaseEyeHeight = 72.0f;
	CrouchedEyeHeight = 40.0f;

	// The capsule yaws with the camera; pitch and roll stay upright.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	MoveComp->MaxWalkSpeed = WalkSpeed;
	MoveComp->MinAnalogWalkSpeed = 0.0f;

	// ~0.33 s to reach walk speed and a soft, unhurried stop. Unreal's
	// defaults (2048 braking) stop dead, which feels abrupt at this pace.
	MoveComp->MaxAcceleration = 600.0f;
	MoveComp->BrakingDecelerationWalking = 700.0f;
	MoveComp->GroundFriction = 5.0f;
	MoveComp->bUseSeparateBrakingFriction = false;

	MoveComp->bOrientRotationToMovement = false;
	MoveComp->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// No jumping by design. Zeroing JumpZVelocity as well as clearing the nav
	// flag means even a stray Jump() call cannot lift the player.
	MoveComp->JumpZVelocity = 0.0f;
	MoveComp->AirControl = 0.15f;
	MoveComp->NavAgentProps.bCanJump = false;
	MoveComp->NavAgentProps.bCanCrouch = false;

	// A generous step height and perch threshold let the player walk over
	// kerbs, low props and path edges without catching on them.
	MoveComp->MaxStepHeight = 35.0f;
	MoveComp->PerchRadiusThreshold = 12.0f;
	MoveComp->SetWalkableFloorAngle(46.0f);

	FarmCamera = CreateDefaultSubobject<UFarmCameraComponent>(TEXT("FarmCamera"));
	FarmCamera->SetupAttachment(GetCapsuleComponent());
	FarmCamera->SetDefaultCameraModeClass(UFarmCameraMode_FreeWalk::StaticClass());

	InteractionComponent = CreateDefaultSubobject<UFarmInteractionComponent>(TEXT("InteractionComponent"));
}

void AFarmCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (InteractionComponent)
	{
		OnInteractPressed.AddUObject(InteractionComponent.Get(), &UFarmInteractionComponent::TryInteract);
	}
}

UFarmInputConfig* AFarmCharacter::ResolveInputConfig()
{
	if (InputConfig && InputConfig->IsComplete())
	{
		return InputConfig;
	}

	if (InputConfig)
	{
		// All-or-nothing on purpose: silently merging generated defaults into
		// a half-authored asset hides which bindings are actually live.
		UE_LOG(LogFarmPlayer, Warning,
			TEXT("InputConfig '%s' is incomplete (missing: %s). Ignoring it and using built-in defaults."),
			*GetNameSafe(InputConfig), *InputConfig->DescribeMissingFields());
	}

	return UFarmInputConfig::CreateRuntimeConfig(this);
}

void AFarmCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	ActiveInputConfig = ResolveInputConfig();
	if (!ActiveInputConfig || !ActiveInputConfig->IsComplete())
	{
		UE_LOG(LogFarmPlayer, Error, TEXT("No usable input config; the player will not respond to input."));
		return;
	}

	if (const APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				// Not ClearAllMappings(): later phases add their own contexts
				// (menus, photo mode) and must not be wiped by a re-possess.
				Subsystem->AddMappingContext(ActiveInputConfig->MappingContext, ActiveInputConfig->MappingContextPriority);
			}
		}
	}

	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInput)
	{
		UE_LOG(LogFarmPlayer, Error,
			TEXT("PlayerInputComponent is %s, not a UEnhancedInputComponent. Check DefaultInputComponentClass in DefaultEngine.ini."),
			*GetNameSafe(PlayerInputComponent));
		return;
	}

	EnhancedInput->BindAction(ActiveInputConfig->MoveAction, ETriggerEvent::Triggered, this, &AFarmCharacter::Move);
	EnhancedInput->BindAction(ActiveInputConfig->LookAction, ETriggerEvent::Triggered, this, &AFarmCharacter::Look);
	EnhancedInput->BindAction(ActiveInputConfig->InteractAction, ETriggerEvent::Triggered, this, &AFarmCharacter::Interact);
	EnhancedInput->BindAction(ActiveInputConfig->MenuAction, ETriggerEvent::Triggered, this, &AFarmCharacter::ToggleMenu);
	EnhancedInput->BindAction(ActiveInputConfig->RunAction, ETriggerEvent::Started, this, &AFarmCharacter::StartRunning);
	EnhancedInput->BindAction(ActiveInputConfig->RunAction, ETriggerEvent::Completed, this, &AFarmCharacter::StopRunning);

	UE_LOG(LogFarmPlayer, Log, TEXT("Farm input bound using config '%s'."), *GetNameSafe(ActiveInputConfig));
}

void AFarmCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MoveAxis = Value.Get<FVector2D>();
	if (MoveAxis.IsNearlyZero() || !Controller)
	{
		return;
	}

	// Move relative to where the player is looking, but flattened: looking at
	// the sky must not slow forward movement.
	const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
	const FRotationMatrix YawMatrix(YawRotation);

	// The movement component clamps the accumulated input vector to length 1,
	// so diagonals do not travel faster than a straight line.
	AddMovementInput(YawMatrix.GetUnitAxis(EAxis::X), MoveAxis.X);
	AddMovementInput(YawMatrix.GetUnitAxis(EAxis::Y), MoveAxis.Y);
}

void AFarmCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxis = Value.Get<FVector2D>();

	// Y already arrives negated from the mapping context (matching the stock
	// Unreal templates), so bInvertLookY flips it back rather than applying it.
	const float PitchSign = bInvertLookY ? -1.0f : 1.0f;

	AddControllerYawInput(LookAxis.X * LookSensitivity);
	AddControllerPitchInput(LookAxis.Y * LookSensitivity * PitchSign);
}

void AFarmCharacter::Interact(const FInputActionValue& Value)
{
	// The interaction component subscribes to this in BeginPlay. Going
	// through the delegate rather than calling the component keeps the
	// character ignorant of what an interactable is.
	UE_LOG(LogFarmPlayer, Verbose, TEXT("Interact pressed."));
	OnInteractPressed.Broadcast();
}

void AFarmCharacter::ToggleMenu(const FInputActionValue& Value)
{
	if (AFarmPlayerController* FarmController = Cast<AFarmPlayerController>(GetController()))
	{
		FarmController->ToggleMenuInputMode();
	}
}

void AFarmCharacter::StartRunning(const FInputActionValue& Value)
{
	if (!bAllowRunning)
	{
		return;
	}

	// Only the speed cap changes; MaxAcceleration still governs the ramp, so
	// the transition is a gradual pick-up rather than a jolt.
	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
}

void AFarmCharacter::StopRunning(const FInputActionValue& Value)
{
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}
