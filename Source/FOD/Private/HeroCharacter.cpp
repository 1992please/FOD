// Fill out your copyright notice in the Description page of Project Settings.

#include "HeroCharacter.h"
//#include "UnrealEngine.h"
#include "FOD.h"
#include "FODGameModeBase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MyCameraComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"


#define print(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::Red,text)
const FName TraceTag("MyTraceTag");

// Sets default values
AHeroCharacter::AHeroCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UMyCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	b2DMode = false;
	bTransitionCameraMode = false;
	CameraSwitchTime = .2f;
	Volume2DTag = "CameraOrthSwitch";
	ShadowMode = EShadowMode::Off;
	FearImmunity = 5;
	MaxHealth = 100;

	BaseTimeForRegenration = 2;
	HealthRegenrationPerSecond = 10;
	HealthTimer = 0;
	HangingOffset = -102;
	BarkourState = EBarkourState::GroundedNormal;
	bCanWallRun = true;
}

// Called when the game starts or when spawned
void AHeroCharacter::BeginPlay()
{
	Super::BeginPlay();
	Health = MaxHealth;
}

void AHeroCharacter::OnConstruction(const FTransform& Transform)
{
	if (GetMesh())
	{
		MeshMats.Empty();
		MeshMats.Reserve(GetMesh()->GetNumMaterials());
		for (int i = 0; i < GetMesh()->GetNumMaterials(); i++)
		{
			MeshMats.Add(GetMesh()->CreateAndSetMaterialInstanceDynamic(i));
		}
	}
}

void AHeroCharacter::NotifyActorBeginOverlap(AActor * OtherActor)
{
	if (OtherActor->ActorHasTag(Volume2DTag))
	{
		SwitchTo2DMode(true);
		if ((bool)ShadowMode)
		{
			FollowCamera->SwitchToOrthogonal(true);
		}
	}
}

void AHeroCharacter::NotifyActorEndOverlap(AActor * OtherActor)
{
	if (OtherActor->ActorHasTag(Volume2DTag))
	{
		SwitchTo2DMode(false);
		if ((bool)ShadowMode)
		{
			FollowCamera->SwitchToOrthogonal(false);
		}
	}
}

// Called every frame
void AHeroCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetCharacterMovement()->IsFalling())
	{
		if (BarkourState == EBarkourState::GroundedNormal)
			SetBarkourState(EBarkourState::InAir);
	}
	else
	{
		if (BarkourState == EBarkourState::InAir)
			SetBarkourState(EBarkourState::GroundedNormal);
	}

	UpdateCameraPosition(DeltaTime);

	UpdateFearTimer(DeltaTime);

	UpdateHealth(DeltaTime);

	UpdateTracers(DeltaTime);
}

// Called to bind functionality to input
void AHeroCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up gameplay key bindings
	check(InputComponent);
	InputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	InputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	InputComponent->BindAction("ShadowSwitch", IE_Pressed, this, &AHeroCharacter::EnterShadowWorld);
	InputComponent->BindAction("ShadowSwitch", IE_Released, this, &AHeroCharacter::ExitShadowWorld);

	InputComponent->BindAction("FocusedMood", IE_Pressed, this, &AHeroCharacter::FocusedModeOn);
	InputComponent->BindAction("FocusedMood", IE_Released, this, &AHeroCharacter::FocusedModeOff);

	InputComponent->BindAxis("MoveForward", this, &AHeroCharacter::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AHeroCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	InputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	InputComponent->BindAxis("TurnRate", this, &AHeroCharacter::TurnAtRate);
	InputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	InputComponent->BindAxis("LookUpRate", this, &AHeroCharacter::LookUpAtRate);

	// handle touch devices
}

void AHeroCharacter::ActivateShadow_Implementation(EShadowMode bMode)
{
	print("Hero Character activate shadow mode");
	ShadowMode = bMode;
	if (b2DMode)
	{
		FollowCamera->SwitchToOrthogonal(ShadowMode == EShadowMode::On);
	}
}

float AHeroCharacter::TakeDamage(float DamageAmount, FDamageEvent const & DamageEvent, AController * EventInstigator, AActor * DamageCauser)
{
	if (Health > 0)
	{
		Health -= DamageAmount;
		bDamaged = true;

		if (Health <= 0)
		{
			Health = 0;
			// Implement Dead Case
		}
		UpdateBurnParameter();
	}
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void AHeroCharacter::SetBarkourState(EBarkourState NewState)
{
	if (NewState == BarkourState)
	{
		return;
	}

	switch (NewState)
	{
		case EBarkourState::GroundedNormal:
		{
			bCanWallRun = true;
			GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
		}
		break;
		case EBarkourState::GroundedActive:
		{

		}
		break;
		case EBarkourState::WallRunning:
		{
			bCanWallRun = false;
			GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
			WallRunningOldAltitude = GetActorLocation().Z;

			WallRunningNewAltitude = GetActorLocation().Z + 300;

			WallRunningFactor = 0;
		}
		break;
		case EBarkourState::Hanging:
		{
			GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
			GetMovementComponent()->StopMovementImmediately();
		}
		break;
		case EBarkourState::ClimbingEdge:
		{
			if (ClimbWallAnim)
			{
				PlayAnimMontage(ClimbWallAnim);
				//bIsHanging = false;
			}
		}
		break;
		case EBarkourState::Rolling:
		{

		}
		break;
		case EBarkourState::InAir:
		{
			GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
		}
		break;
		default:
			break;
	}

	BarkourState = NewState;
	UpdateTracers();
}

bool AHeroCharacter::CheckCanGrab()
{
	FHitResult HitOut;
	if (TraceUpperDownward(HitOut))
	{
		LedgeImpactPoint = HitOut.ImpactPoint;
		if (TraceMiddleForward(HitOut))
		{
			WallTraceImpactPoint = HitOut.ImpactPoint;
			WallNormal = HitOut.ImpactNormal;

			const float DotValue = FVector::DotProduct(GetAxisDirection(), WallNormal);
			if (DotValue < -.7)
			{
				SetBarkourState(EBarkourState::Hanging);
				return true;
			}
		}
	}
	return false;
}

bool AHeroCharacter::CheckCanWallRun()
{
	FHitResult HitOut;
	if (TraceUpperForward(HitOut))
	{
		WallTraceImpactPoint = HitOut.ImpactPoint;
		WallNormal = HitOut.ImpactNormal;

		const float DotValue = FVector::DotProduct(GetAxisDirection(), WallNormal);
		if (DotValue < -.7 && bCanWallRun)
		{
			SetBarkourState(EBarkourState::WallRunning);
			return true;
		}
	}
	return false;
}

void AHeroCharacter::UpdateTracers(float DeltaTime)
{
	switch (BarkourState)
	{
		case EBarkourState::GroundedNormal:
		{
			CheckCanWallRun();
		}
		break;
		case EBarkourState::GroundedActive:
		{

		}
		break;
		case EBarkourState::WallRunning:
		{
			WallRunningFactor += DeltaTime / 1;
			//const float DotValue = FVector::DotProduct(GetAxisDirection(), WallNormal);

			if (WallRunningFactor > 1)
			{
				SetBarkourState(EBarkourState::InAir);
				return;
			}
			FRotator Rotation = (-WallNormal).Rotation();
			FVector Location = WallTraceImpactPoint + WallNormal * GetCapsuleComponent()->GetScaledCapsuleRadius();
			Location.Z = FMath::Lerp<float>(WallRunningOldAltitude, WallRunningNewAltitude, WallRunningFactor);



			SetActorLocationAndRotation(Location, Rotation);
			CheckCanGrab();
		}
		break;
		case EBarkourState::Hanging:
		{
			FRotator Rotation = (-WallNormal).Rotation();
			FVector Location = WallTraceImpactPoint + WallNormal * GetCapsuleComponent()->GetScaledCapsuleRadius();
			Location.Z = LedgeImpactPoint.Z + HangingOffset;
			SetActorLocationAndRotation(Location, Rotation);
		}
		break;
		case EBarkourState::ClimbingEdge:
		{

		}
		break;
		case EBarkourState::Rolling:
		{

		}
		break;
		case EBarkourState::InAir:
		{

			if (CheckCanGrab())
			{
				return;
			}

			CheckCanWallRun();
		}
		break;
		default:
		{

		}
		break;
	}
}

bool AHeroCharacter::TraceLowerForward(FHitResult & HitOut)
{
	return false;
}

bool AHeroCharacter::TraceMiddleForward(FHitResult & HitOut)
{
	FVector Start = GetActorLocation();
	FVector End = Start + GetActorForwardVector() * 150;

	return TraceWall(Start, End, HitOut);
}

bool AHeroCharacter::TraceUpperDownward(FHitResult & HitOut)
{
	FVector Start = GetActorLocation();
	Start.Z += GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 10;
	Start += GetActorForwardVector() * 70;
	FVector End = Start;
	End.Z -= 10;

	return TraceWall(Start, End, HitOut);
}

bool AHeroCharacter::TraceUpperForward(FHitResult & HitOut)
{
	FVector Start = GetActorLocation();

	Start.Z += GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 100;

	FVector End = Start + GetActorForwardVector() * (GetCapsuleComponent()->GetScaledCapsuleRadius() + 5);

	return TraceWall(Start, End, HitOut);
}

bool AHeroCharacter::TraceWall(FVector Start, FVector End, FHitResult & HitOut)
{
	FCollisionQueryParams TraceParams;
	TraceParams.TraceTag = TraceTag;
	TraceParams.bTraceComplex = true;

	return GetWorld()->LineTraceSingleByChannel(HitOut, Start, End, TRACETYPE_HeroTracer, TraceParams);
}

FVector AHeroCharacter::GetAxisDirection()
{
	// find out which way is right
	const FRotator Rotation = FollowCamera->GetComponentRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	// get right vector 
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	return  (ForwardDirection * MovementInput.X + RightDirection * MovementInput.Y).GetSafeNormal2D();
}

void AHeroCharacter::UpdateCameraPosition(float DeltaSeconds)
{
	if (bTransitionCameraMode)
	{
		InterpFactor += DeltaSeconds;
		const float Factor = InterpFactor / CameraSwitchTime;
		if (Factor <= 1)
		{
			if (!b2DMode)
			{
				CameraBoom->SetWorldRotation(UKismetMathLibrary::RLerp(InterpRotation, GetViewRotation(), Factor, true));
				CameraBoom->TargetArmLength = FMath::Lerp(1000.f, 300.f, Factor);

			}
			else
			{
				CameraBoom->SetWorldRotation(UKismetMathLibrary::RLerp(InterpRotation, FRotator::ZeroRotator, Factor, true));
				CameraBoom->TargetArmLength = FMath::Lerp(300.f, 1000.f, Factor);
			}
		}
		else
		{
			bTransitionCameraMode = false;
			if (!b2DMode)
			{
				CameraBoom->bUsePawnControlRotation = true;
			}
			else
			{
				//FollowCamera->ProjectionMode = ECameraProjectionMode::Orthographic;

			}
		}
	}
	else if (b2DMode)
	{
		CameraBoom->SetWorldRotation(FRotator::ZeroRotator);
	}
}

void AHeroCharacter::UpdateFearTimer(float DeltaSeconds)
{
	if ((bool)ShadowMode)
	{

		FearTimer += DeltaSeconds;
		if (FearTimer >= FearImmunity)
		{
			ExitShadowWorld();
		}
	}
	else if (FearTimer > 0)
	{
		FearTimer -= DeltaSeconds;
	}
}

void AHeroCharacter::UpdateHealth(float DeltaSeconds)
{
	if (Health < MaxHealth)
	{
		if (bDamaged)
		{
			HealthTimer = 0;
			bDamaged = false;
		}
		else
		{
			HealthTimer += DeltaSeconds;
			if (HealthTimer >= BaseTimeForRegenration)
			{
				Health += DeltaSeconds * HealthRegenrationPerSecond;
				if (Health > MaxHealth)
				{
					Health = MaxHealth;
				}
				UpdateBurnParameter();
			}
		}
	}
}

void AHeroCharacter::UpdateBurnParameter()
{
	const float BurnFactor = 1 - GetHealthPercentage();

	for (UMaterialInstanceDynamic* Mat : MeshMats)
	{
		Mat->SetScalarParameterValue("BurnFactor", BurnFactor);
	}
}

float AHeroCharacter::GetFearPercentage() const
{
	return FearTimer / FearImmunity;
}

float AHeroCharacter::GetHealthPercentage() const
{
	return Health / MaxHealth;
}

void AHeroCharacter::EnterShadowWorld()
{
	AFODGameModeBase* GameMode = Cast<AFODGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		GameMode->SwitchShadow(true);
	}
}

void AHeroCharacter::ExitShadowWorld()
{
	AFODGameModeBase* GameMode = Cast<AFODGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		GameMode->SwitchShadow(true);
	}
}

void AHeroCharacter::SwitchTo2DMode(bool bMode)
{
	b2DMode = bMode;
	bTransitionCameraMode = true;
	InterpFactor = 0;

	if (!b2DMode)
	{
		InterpRotation = CameraBoom->GetComponentRotation();
	}
	else
	{
		CameraBoom->bUsePawnControlRotation = false;
		InterpRotation = CameraBoom->GetComponentRotation();
	}
}

void AHeroCharacter::MoveForward(float Value)
{
	MovementInput.X = Value;


	// find out which way is forward
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	// get forward vector
	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	switch (BarkourState)
	{
		case EBarkourState::InAir:
		case EBarkourState::GroundedNormal:
		{
			if ((Controller != NULL) && (Value != 0.0f))
			{
				if (!b2DMode)
				{

					AddMovementInput(Direction, Value);
				}
			}
		}
		break;

		case EBarkourState::Hanging:
			break;
		default:
			break;
	}

}

void AHeroCharacter::MoveRight(float Value)
{
	MovementInput.Y = Value;

	switch (BarkourState)
	{
		case EBarkourState::InAir:
		case EBarkourState::GroundedNormal:
		{
			if ((Controller != NULL) && (Value != 0.0f))
			{
				if (!b2DMode)
				{
					// find out which way is right
					const FRotator Rotation = Controller->GetControlRotation();
					const FRotator YawRotation(0, Rotation.Yaw, 0);

					// get right vector 
					const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
					// add movement in that direction
					AddMovementInput(Direction, Value);
				}
				else
				{
					const FVector Direction(0, 1, 0);
					AddMovementInput(Direction, Value);

				}
			}
		}
		break;
		default:
			break;
	}

}

void AHeroCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AHeroCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AHeroCharacter::Jump()
{
	switch (BarkourState)
	{
		case EBarkourState::GroundedNormal:
		{
			Super::Jump();
		}
		break;
		case EBarkourState::Hanging:
		{

			float DotValue = FVector::DotProduct(GetAxisDirection(), WallNormal);

			if (DotValue > .7)
			{
				UCharacterMovementComponent* const MC = GetCharacterMovement();

				MC->Velocity.Z = MC->JumpZVelocity * 2;
				MC->Velocity.X = MC->JumpZVelocity * WallNormal.X;
				MC->Velocity.Y = MC->JumpZVelocity * WallNormal.Y;
				SetBarkourState(EBarkourState::InAir);
			}
			else if (DotValue < -.7)
			{
				SetBarkourState(EBarkourState::ClimbingEdge);
			}
		}
		break;
		case EBarkourState::WallRunning:
		{
			bCanWallRun = true;
			UCharacterMovementComponent* const MC = GetCharacterMovement();

			MC->Velocity.Z = MC->JumpZVelocity * 1.5;
			MC->Velocity.X = MC->JumpZVelocity * WallNormal.X;
			MC->Velocity.Y = MC->JumpZVelocity * WallNormal.Y;
			SetBarkourState(EBarkourState::InAir);
		}
		break;
		default:
			break;
	}
}

void AHeroCharacter::StopJumping()
{
	Super::StopJumping();
}

void AHeroCharacter::FocusedModeOn()
{
	bFocusedMode = true;
}

void AHeroCharacter::FocusedModeOff()
{
	bFocusedMode = false;
}
