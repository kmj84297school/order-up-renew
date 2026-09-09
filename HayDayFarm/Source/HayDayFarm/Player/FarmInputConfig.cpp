// Copyright (c) 2026. Private personal project.

#include "Player/FarmInputConfig.h"
#include "Core/FarmLog.h"
#include "EnhancedActionKeyMapping.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "InputTriggers.h"

namespace FarmInputConfigInternal
{
	/** Creates an input action of the given value type. */
	static UInputAction* MakeAction(UObject* Outer, const TCHAR* ActionName, EInputActionValueType ValueType)
	{
		UInputAction* Action = NewObject<UInputAction>(Outer, FName(ActionName));
		Action->ValueType = ValueType;
		return Action;
	}

	/** Maps a key that should produce +1 on the action's X axis. A digital key
	 *  already writes 1.0 to X, so no modifier is needed. */
	static void MapForward(UInputMappingContext* Context, UInputAction* Action, const FKey& Key)
	{
		Context->MapKey(Action, Key);
	}

	/** Maps a key producing -1 on X (backwards). */
	static void MapBackward(UObject* Outer, UInputMappingContext* Context, UInputAction* Action, const FKey& Key)
	{
		FEnhancedActionKeyMapping& Mapping = Context->MapKey(Action, Key);
		Mapping.Modifiers.Add(NewObject<UInputModifierNegate>(Outer));
	}

	/** Maps a key producing +1 on Y (right). A digital key writes to X, so the
	 *  swizzle moves it to Y. */
	static void MapRight(UObject* Outer, UInputMappingContext* Context, UInputAction* Action, const FKey& Key)
	{
		FEnhancedActionKeyMapping& Mapping = Context->MapKey(Action, Key);

		UInputModifierSwizzleAxis* Swizzle = NewObject<UInputModifierSwizzleAxis>(Outer);
		Swizzle->Order = EInputAxisSwizzle::YXZ;
		Mapping.Modifiers.Add(Swizzle);
	}

	/** Maps a key producing -1 on Y (left). Order matters: swizzle X into Y
	 *  first, then negate, otherwise the negate lands on the unused X axis. */
	static void MapLeft(UObject* Outer, UInputMappingContext* Context, UInputAction* Action, const FKey& Key)
	{
		FEnhancedActionKeyMapping& Mapping = Context->MapKey(Action, Key);

		UInputModifierSwizzleAxis* Swizzle = NewObject<UInputModifierSwizzleAxis>(Outer);
		Swizzle->Order = EInputAxisSwizzle::YXZ;
		Mapping.Modifiers.Add(Swizzle);
		Mapping.Modifiers.Add(NewObject<UInputModifierNegate>(Outer));
	}

	/** Maps a boolean action that should fire exactly once per press. */
	static void MapPress(UObject* Outer, UInputMappingContext* Context, UInputAction* Action, const FKey& Key)
	{
		FEnhancedActionKeyMapping& Mapping = Context->MapKey(Action, Key);
		Mapping.Triggers.Add(NewObject<UInputTriggerPressed>(Outer));
	}
}

bool UFarmInputConfig::IsComplete() const
{
	return MoveAction && LookAction && InteractAction && RunAction && MenuAction && MappingContext;
}

FString UFarmInputConfig::DescribeMissingFields() const
{
	TArray<FString> Missing;

	if (!MoveAction)     { Missing.Add(TEXT("MoveAction")); }
	if (!LookAction)     { Missing.Add(TEXT("LookAction")); }
	if (!InteractAction) { Missing.Add(TEXT("InteractAction")); }
	if (!RunAction)      { Missing.Add(TEXT("RunAction")); }
	if (!MenuAction)     { Missing.Add(TEXT("MenuAction")); }
	if (!MappingContext) { Missing.Add(TEXT("MappingContext")); }

	return Missing.Num() > 0 ? FString::Join(Missing, TEXT(", ")) : TEXT("none");
}

UFarmInputConfig* UFarmInputConfig::CreateRuntimeConfig(UObject* Outer)
{
	using namespace FarmInputConfigInternal;

	check(Outer);

	UFarmInputConfig* Config = NewObject<UFarmInputConfig>(Outer, TEXT("FarmRuntimeInputConfig"));

	Config->MoveAction     = MakeAction(Config, TEXT("IA_Move"),     EInputActionValueType::Axis2D);
	Config->LookAction     = MakeAction(Config, TEXT("IA_Look"),     EInputActionValueType::Axis2D);
	Config->InteractAction = MakeAction(Config, TEXT("IA_Interact"), EInputActionValueType::Boolean);
	Config->RunAction      = MakeAction(Config, TEXT("IA_Run"),      EInputActionValueType::Boolean);
	Config->MenuAction     = MakeAction(Config, TEXT("IA_Menu"),     EInputActionValueType::Boolean);

	UInputMappingContext* Context = NewObject<UInputMappingContext>(Config, TEXT("IMC_FarmDefault"));

	MapForward(Context, Config->MoveAction, EKeys::W);
	MapForward(Context, Config->MoveAction, EKeys::Up);
	MapBackward(Config, Context, Config->MoveAction, EKeys::S);
	MapBackward(Config, Context, Config->MoveAction, EKeys::Down);
	MapRight(Config, Context, Config->MoveAction, EKeys::D);
	MapRight(Config, Context, Config->MoveAction, EKeys::Right);
	MapLeft(Config, Context, Config->MoveAction, EKeys::A);
	MapLeft(Config, Context, Config->MoveAction, EKeys::Left);

	{
		// Mouse2D delivers X = horizontal delta, Y = vertical delta with up
		// positive. Negating Y here matches the convention used by the stock
		// Unreal templates, so a hand-authored IMC can be swapped in later
		// without also having to flip the sign in AFarmCharacter::Look().
		FEnhancedActionKeyMapping& LookMapping = Context->MapKey(Config->LookAction, EKeys::Mouse2D);

		UInputModifierNegate* NegateY = NewObject<UInputModifierNegate>(Config);
		NegateY->bX = false;
		NegateY->bY = true;
		NegateY->bZ = false;
		LookMapping.Modifiers.Add(NegateY);
	}

	MapPress(Config, Context, Config->InteractAction, EKeys::E);
	MapPress(Config, Context, Config->MenuAction, EKeys::Escape);

	// Run is a plain held boolean -- no Pressed trigger, so Started and
	// Completed both fire and the character can return to walking speed.
	Context->MapKey(Config->RunAction, EKeys::LeftShift);

	Config->MappingContext = Context;

	UE_LOG(LogFarmPlayer, Log, TEXT("Built runtime input config (WASD + mouse + E + Esc, no content assets required)."));

	return Config;
}
