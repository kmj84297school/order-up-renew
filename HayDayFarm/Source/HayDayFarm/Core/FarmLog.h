// Copyright (c) 2026. Private personal project.

#pragma once

#include "CoreMinimal.h"
#include "Logging/LogMacros.h"

/**
 * Project-wide log categories.
 *
 * Keeping these split by subsystem means a noisy area (animals, environment)
 * can be silenced from the console without hiding everything else:
 *     Log LogFarmAnimal Off
 */
DECLARE_LOG_CATEGORY_EXTERN(LogFarm, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogFarmPlayer, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogFarmCamera, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogFarmEnvironment, Log, All);
