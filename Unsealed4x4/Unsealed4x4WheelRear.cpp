// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "Unsealed4x4.h"
#include "Unsealed4x4WheelRear.h"

UUnsealed4x4WheelRear::UUnsealed4x4WheelRear(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ShapeRadius = 40.f;
	ShapeWidth = 34.0f;
	bAffectedByHandbrake = true;
	SteerAngle = 40.f;
	DampingRate = 0.25f;
	Mass = 20;
}
