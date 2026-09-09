// Copyright (c) 2026. Private personal project.

#pragma once

#include "CoreMinimal.h"
#include "FarmCameraTypes.generated.h"

/** Easing applied while a camera mode blends in. */
UENUM(BlueprintType)
enum class EFarmCameraBlendFunction : uint8
{
	/** Constant rate. Reads as mechanical; only use for very short blends. */
	Linear,

	/** Slow start, fast finish. */
	EaseIn,

	/** Fast start, slow settle. Good for returning to Free Walk. */
	EaseOut,

	/** Slow start and slow settle. The default for this project: it is the
	 *  least noticeable transition and suits a relaxed, cinematic feel. */
	EaseInOut
};

/**
 * The minimal description of "where the camera is looking from".
 *
 * Camera modes produce one of these per frame; the mode stack blends them
 * together. Deliberately POD-like so blending stays cheap and predictable.
 */
USTRUCT(BlueprintType)
struct HAYDAYFARM_API FFarmCameraView
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Farm|Camera")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Farm|Camera")
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadWrite, Category = "Farm|Camera")
	float FieldOfView = 90.0f;

	/** Blends this view toward Other by Weight (0 = keep this, 1 = take Other). */
	void Blend(const FFarmCameraView& Other, float Weight);
};
