// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "Unsealed4x4.h"
#include "Unsealed4x4WheelRear.h"

UUnsealed4x4WheelRear::UUnsealed4x4WheelRear(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ShapeRadius = 18.0f;
	ShapeWidth = 15.0f;
	bAffectedByHandbrake = true;
	SteerAngle = 0.f;
}
