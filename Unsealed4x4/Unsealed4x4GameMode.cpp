// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "Unsealed4x4.h"
#include "Unsealed4x4GameMode.h"
#include "Unsealed4x4Pawn.h"
#include "Unsealed4x4Hud.h"

AUnsealed4x4GameMode::AUnsealed4x4GameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DefaultPawnClass = AUnsealed4x4Pawn::StaticClass();
	HUDClass = AUnsealed4x4Hud::StaticClass();
}
