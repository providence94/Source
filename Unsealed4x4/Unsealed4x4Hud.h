// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/HUD.h"
#include "Unsealed4x4Hud.generated.h"


UCLASS(config = Game)
class AUnsealed4x4Hud : public AHUD
{
	GENERATED_BODY()

public:
	AUnsealed4x4Hud(const FObjectInitializer& ObjectInitializer);

	/** Font used to render the vehicle info */
	UPROPERTY()
	UFont* HUDFont;

	// Begin AHUD interface
	virtual void DrawHUD() override;
	// End AHUD interface

};
