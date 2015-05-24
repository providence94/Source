// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "Unsealed4x4.h"
#include "Unsealed4x4GameMode.h"
#include "Unsealed4x4Pawn.h"
#include "Unsealed4x4Hud.h"

AUnsealed4x4GameMode::AUnsealed4x4GameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("Class'/Game/Offroad4x4Content/Vehicle/Unsealed.Unsealed_C'"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;
	HUDClass = AUnsealed4x4Hud::StaticClass();
	static ConstructorHelpers::FClassFinder<APlayerController> PlayerController(TEXT("Class'/Game/Offroad4x4Content/Vehicle/UnsealedPlayerController.UnsealedPlayerController_C'"));
	PlayerControllerClass = PlayerController.Class;
	
}
