// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "Unsealed4x4.h"
#include "Unsealed4x4Pawn.h"
#include "Unsealed4x4WheelFront.h"
#include "Unsealed4x4WheelRear.h"
#include "Unsealed4x4Hud.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/TextRenderComponent.h"
#include "Sound/SoundCue.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Vehicles/WheeledVehicleMovementComponent4W.h"
#include "Engine/SkeletalMesh.h"

const FName AUnsealed4x4Pawn::LookUpBinding("LookUp");
const FName AUnsealed4x4Pawn::LookRightBinding("LookRight");
const FName AUnsealed4x4Pawn::EngineAudioRPM("RPM");

#define LOCTEXT_NAMESPACE "VehiclePawn"

AUnsealed4x4Pawn::AUnsealed4x4Pawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Car mesh 
	//static ConstructorHelpers::FObjectFinder<USkeletalMesh> CarMesh(TEXT("/Game/VehicleAdv/Vehicle/Vehicle_SkelMesh.Vehicle_SkelMesh")); //default one
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> CarMesh(TEXT("/Game/Offroad4x4Content/Vehicle/Amorok_01_buggy.Amorok_01_buggy")); // dummy by tim
	GetMesh()->SetSkeletalMesh(CarMesh.Object);
	
	//static ConstructorHelpers::FClassFinder<UObject> AnimBPClass(TEXT("/Game/VehicleAdv/Vehicle/VehicleAnimationBlueprint"));
	static ConstructorHelpers::FClassFinder<UObject> AnimBPClass(TEXT("/Game/Offroad4x4Content/Vehicle/Amorok_Anim"));
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	GetMesh()->SetAnimInstanceClass(AnimBPClass.Class);

	// Setup friction materials
	static ConstructorHelpers::FObjectFinder<UPhysicalMaterial> SlipperyMat(TEXT("/Game/VehicleAdv/PhysicsMaterials/Slippery.Slippery"));
	SlipperyMaterial = SlipperyMat.Object;
		
	static ConstructorHelpers::FObjectFinder<UPhysicalMaterial> NonSlipperyMat(TEXT("/Game/VehicleAdv/PhysicsMaterials/NonSlippery.NonSlippery"));
	NonSlipperyMaterial = NonSlipperyMat.Object;

	UWheeledVehicleMovementComponent4W* Vehicle4W = CastChecked<UWheeledVehicleMovementComponent4W>(GetVehicleMovement());

	check(Vehicle4W->WheelSetups.Num() == 4);

	// Wheels/Tyres
	// Setup the wheels
	Vehicle4W->WheelSetups[0].WheelClass = UUnsealed4x4WheelFront::StaticClass();
	Vehicle4W->WheelSetups[0].BoneName = FName("F_L_wheelJNT");
	Vehicle4W->WheelSetups[0].AdditionalOffset = FVector(0.f, -8.f, 0.f);

	Vehicle4W->WheelSetups[1].WheelClass = UUnsealed4x4WheelFront::StaticClass();
	Vehicle4W->WheelSetups[1].BoneName = FName("F_R_wheelJNT");
	Vehicle4W->WheelSetups[1].AdditionalOffset = FVector(0.f, 8.f, 0.f);

	Vehicle4W->WheelSetups[2].WheelClass = UUnsealed4x4WheelRear::StaticClass();
	Vehicle4W->WheelSetups[2].BoneName = FName("B_L_wheelJNT");
	Vehicle4W->WheelSetups[2].AdditionalOffset = FVector(0.f, -8.f, 0.f);

	Vehicle4W->WheelSetups[3].WheelClass = UUnsealed4x4WheelRear::StaticClass();
	Vehicle4W->WheelSetups[3].BoneName = FName("B_R_wheelJNT");
	Vehicle4W->WheelSetups[3].AdditionalOffset = FVector(0.f, 8.f, 0.f);

	// Adjust the tire loading
	Vehicle4W->MinNormalizedTireLoad = 0.0f;
	Vehicle4W->MinNormalizedTireLoadFiltered = 0.2f;
	Vehicle4W->MaxNormalizedTireLoad = 2.0f;
	Vehicle4W->MaxNormalizedTireLoadFiltered = 2.0f;

	// Engine 
	// Torque setup
	Vehicle4W->MaxEngineRPM = 5700.0f;
	Vehicle4W->EngineSetup.TorqueCurve.GetRichCurve()->Reset();
	Vehicle4W->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(0.0f, 400.0f);
	Vehicle4W->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(1890.0f, 500.0f);
	Vehicle4W->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(5730.0f, 400.0f);
 
	// Adjust the steering 
	Vehicle4W->SteeringCurve.GetRichCurve()->Reset();
	Vehicle4W->SteeringCurve.GetRichCurve()->AddKey(0.0f, 1.0f);
	Vehicle4W->SteeringCurve.GetRichCurve()->AddKey(40.0f, 0.7f);
	Vehicle4W->SteeringCurve.GetRichCurve()->AddKey(120.0f, 0.6f);
			
 	// Transmission	
	// We want 4wd
	Vehicle4W->DifferentialSetup.DifferentialType = EVehicleDifferential4W::LimitedSlip_RearDrive;
	
	// Drive the front wheels a little more than the rear
	Vehicle4W->DifferentialSetup.FrontRearSplit = 0.65;

	// Automatic gearbox
	Vehicle4W->TransmissionSetup.bUseGearAutoBox = true;
	
	// Physics settings
	// Adjust the center of mass - the buggy is quite low
	UPrimitiveComponent* UpdatedPrimitive = Cast<UPrimitiveComponent>(Vehicle4W->UpdatedComponent);
	if (UpdatedPrimitive)
	{
		UpdatedPrimitive->BodyInstance.COMNudge = FVector(8.0f, 0.0f, 0.0f);
	}

	// Set the inertia scale. This controls how the mass of the vehicle is distributed.
	Vehicle4W->InertiaTensorScale = FVector(1.0f, 1.333f, 1.2f);

	// Create a spring arm component for our chase camera
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	SpringArm->SetRelativeLocation(FVector(0.0f, 0.000f, 0.0f));
	SpringArm->AttachTo(RootComponent);
	SpringArm->TargetArmLength = 200.0f;
	SpringArm->bEnableCameraLag = true;
	SpringArm->bEnableCameraRotationLag = true;
	SpringArm->bInheritPitch = true;
	SpringArm->bInheritYaw = true;
	SpringArm->bInheritRoll = true;

	// Create the chase camera component 
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("ChaseCamera"));
	Camera->AttachTo(SpringArm, USpringArmComponent::SocketName);
	Camera->SetRelativeRotation(FRotator(-20.0f, 0.0f, 0.0f)); 
	Camera->SetRelativeLocation(FVector(-500.0f, 0.000000f, 300.0f));
	Camera->bUsePawnControlRotation = false;
	Camera->FieldOfView = 90.f;

	// Create In-Car camera component 
	InternalCameraOrigin = FVector(-34.0f, 0.0f, 50.0f);
	InternalCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("InternalCamera"));
	//InternalCamera->AttachTo(SpringArm, USpringArmComponent::SocketName);
	InternalCamera->bUsePawnControlRotation = false;
	InternalCamera->FieldOfView = 90.f;
	InternalCamera->SetRelativeLocation(InternalCameraOrigin);
	InternalCamera->AttachTo(GetMesh());

	// In car HUD
	// Create text render component for in car speed display
	InCarSpeed = CreateDefaultSubobject<UTextRenderComponent>(TEXT("IncarSpeed"));
	InCarSpeed->SetRelativeScale3D(FVector(0.1f, 0.1f, 0.1f));
	InCarSpeed->SetRelativeLocation(FVector(35.0f, -6.0f, 20.0f));
	InCarSpeed->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
	InCarSpeed->AttachTo(GetMesh());

	// Create text render component for in car gear display
	InCarGear = CreateDefaultSubobject<UTextRenderComponent>(TEXT("IncarGear"));
	InCarGear->SetRelativeScale3D(FVector(0.1f, 0.1f, 0.1f));
	InCarGear->SetRelativeLocation(FVector(35.0f, 5.0f, 20.0f));
	InCarGear->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
	InCarGear->AttachTo(GetMesh());
	
	// Setup the audio component and allocate it a sound cue
	static ConstructorHelpers::FObjectFinder<USoundCue> SoundCue(TEXT("/Game/VehicleAdv/Sound/Engine_Loop_Cue.Engine_Loop_Cue"));
	EngineSoundComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("EngineSound"));
	EngineSoundComponent->SetSound(SoundCue.Object);
	EngineSoundComponent->AttachTo(GetMesh());

	// Colors for the in-car gear display. One for normal one for reverse
	GearDisplayReverseColor = FColor(255, 0, 0, 255);
	GearDisplayColor = FColor(255, 255, 255, 255);

	bIsLowFriction = false;
	bInReverseGear = false;
}

void AUnsealed4x4Pawn::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	// set up gameplay key bindings
	check(InputComponent);

	InputComponent->BindAxis("MoveForward", this, &AUnsealed4x4Pawn::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AUnsealed4x4Pawn::MoveRight);
	InputComponent->BindAxis(LookUpBinding);
	InputComponent->BindAxis(LookRightBinding);

	InputComponent->BindAction("Handbrake", IE_Pressed, this, &AUnsealed4x4Pawn::OnHandbrakePressed);
	InputComponent->BindAction("Handbrake", IE_Released, this, &AUnsealed4x4Pawn::OnHandbrakeReleased);
	InputComponent->BindAction("SwitchCamera", IE_Pressed, this, &AUnsealed4x4Pawn::OnToggleCamera);

	InputComponent->BindAction("ResetVR", IE_Pressed, this, &AUnsealed4x4Pawn::OnResetVR); 
}

void AUnsealed4x4Pawn::MoveForward(float Val)
{
	GetVehicleMovementComponent()->SetThrottleInput(Val);

}

void AUnsealed4x4Pawn::MoveRight(float Val)
{
	GetVehicleMovementComponent()->SetSteeringInput(Val);
}

void AUnsealed4x4Pawn::OnHandbrakePressed()
{
	GetVehicleMovementComponent()->SetHandbrakeInput(true);
}

void AUnsealed4x4Pawn::OnHandbrakeReleased()
{
	GetVehicleMovementComponent()->SetHandbrakeInput(false);
}

void AUnsealed4x4Pawn::OnToggleCamera()
{
	EnableIncarView(!bInCarCameraActive);
}

void AUnsealed4x4Pawn::EnableIncarView(const bool bState)
{
	if (bState != bInCarCameraActive)
	{
		bInCarCameraActive = bState;
		
		if (bState == true)
		{
			OnResetVR();
			Camera->Deactivate();
			InternalCamera->Activate();
			
			APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
			if ( (PlayerController != nullptr) && (PlayerController->PlayerCameraManager != nullptr ) )
			{
				PlayerController->PlayerCameraManager->bFollowHmdOrientation = true;
			}
		}
		else
		{
			InternalCamera->Deactivate();
			Camera->Activate();
		}
		
		InCarSpeed->SetVisibility(bInCarCameraActive);
		InCarGear->SetVisibility(bInCarCameraActive);
	}
}


void AUnsealed4x4Pawn::Tick(float Delta)
{
	// Setup the flag to say we are in reverse gear
	bInReverseGear = GetVehicleMovement()->GetCurrentGear() < 0;
	
	/*if (isAutoAccel && GetVehicleMovement()->IsMovingOnGround())
	{
		AutoAcceleration();
	}*/
	// Update phsyics material
	UpdatePhysicsMaterial();
	
	
	// Update the strings used in the hud (incar and onscreen)
	UpdateHUDStrings();

	// Set the string in the incar hud
	SetupInCarHUD();
	

	// Pass the engine RPM to the sound component
	float RPMToAudioScale = 2500.0f / GetVehicleMovement()->GetEngineMaxRotationSpeed();
	EngineSoundComponent->SetFloatParameter(EngineAudioRPM, GetVehicleMovement()->GetEngineRotationSpeed()*RPMToAudioScale);
}

void AUnsealed4x4Pawn::BeginPlay()
{
	bool bWantInCar = false;
#ifdef HMD_INTGERATION
	// Enable in car view if HMD is attached
	bWantInCar = GEngine->HMDDevice.IsValid()
#endif // HMD_INTGERATION
	
	EnableIncarView(bWantInCar);
	// Start an engine sound playing
	EngineSoundComponent->Play();
}

void AUnsealed4x4Pawn::OnResetVR()
{
#ifdef HMD_INTGERATION
	if (GEngine->HMDDevice.IsValid())
	{
		GEngine->HMDDevice->ResetOrientationAndPosition();
		InternalCamera->SetRelativeLocation(InternalCameraOrigin);
		GetController()->SetControlRotation(FRotator());
	}
#endif // HMD_INTGERATION
}

void AUnsealed4x4Pawn::UpdateHUDStrings()
{
	float KPH = FMath::Abs(GetVehicleMovement()->GetForwardSpeed()) * 0.036f;
	int32 KPH_int = FMath::FloorToInt(KPH);

	// Using FText because this is display text that should be localizable
	SpeedDisplayString = FText::Format(LOCTEXT("SpeedFormat", "{0} km/h"), FText::AsNumber(KPH_int));
	
	if (bInReverseGear == true)
	{
		GearDisplayString = FText(LOCTEXT("ReverseGear", "R"));
	}
	else
	{
		int32 Gear = GetVehicleMovement()->GetCurrentGear();
		GearDisplayString = (Gear == 0) ? LOCTEXT("N", "N") : FText::AsNumber(Gear);
	}	
}

void AUnsealed4x4Pawn::SetupInCarHUD()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if ((PlayerController != nullptr) && (InCarSpeed != nullptr) && (InCarGear != nullptr))
	{
		// Setup the text render component strings
		InCarSpeed->SetText(SpeedDisplayString.ToString());
		InCarGear->SetText(GearDisplayString.ToString());
		
		if (bInReverseGear == false)
		{
			InCarGear->SetTextRenderColor(GearDisplayColor);
		}
		else
		{
			InCarGear->SetTextRenderColor(GearDisplayReverseColor);
		}
	}
}

void AUnsealed4x4Pawn::UpdatePhysicsMaterial()
{
	if (GetActorUpVector().Z < 0)
	{
		if (bIsLowFriction == true)
		{
			GetMesh()->SetPhysMaterialOverride(NonSlipperyMaterial);
			bIsLowFriction = false;
		}
		else
		{
			GetMesh()->SetPhysMaterialOverride(SlipperyMaterial);
			bIsLowFriction = true;
		}
	}
}

#undef LOCTEXT_NAMESPACE
