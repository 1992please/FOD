// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "ShadowInterface.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HeroCharacter.generated.h"

UENUM(BlueprintType)
enum class EBarkourState : uint8
{
	GroundedNormal,
	GroundedActive,
	WallRunning,
	Hanging,
	ClimbingEdge,
	Rolling,
	InAir
};

UCLASS()
class FOD_API AHeroCharacter : public ACharacter, public IShadowInterface
{
	GENERATED_BODY()

public:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void ActivateShadow_Implementation(EShadowMode bMode) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	AHeroCharacter();

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UMyCameraComponent* FollowCamera;

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UMyCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;

protected:
	virtual void BeginPlay() override;

//-----------------------------------Player States---------------------------//
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Fear")
	float FearImmunity;

	UPROPERTY(EditDefaultsOnly, Category = Character)
	float MaxHealth;

	float Health;
	void UpdateBurnParameter();

	float FearTimer;

	float BaseTimeForRegenration;

	float HealthRegenrationPerSecond;

	float HealthTimer;

private:
	bool bDamaged;

	void UpdateCameraPosition(float DeltaSeconds);

	void UpdateFearTimer(float DeltaSeconds);

	void UpdateHealth(float DeltaSeconds);


public:

	UFUNCTION(BlueprintPure, Category = Fear)
	float GetFearPercentage() const;

	UFUNCTION(BlueprintPure, Category = Fear)
	float GetHealthPercentage() const;

	// ----------------------Shadow Switch-------------------------------//
protected:
	EShadowMode ShadowMode;

	void EnterShadowWorld();

	void ExitShadowWorld();
	UPROPERTY(Category = Shadow, EditAnywhere, BlueprintReadOnly)
	class UMaterialInstance* ShadowMaterial;

	UPROPERTY(Category = Shadow, EditAnywhere, BlueprintReadOnly)
	UMaterialInstance* NormalMaterial;

	UPROPERTY()
	TArray<UMaterialInstanceDynamic*> MeshMats;
	// ----------------------2D/3D Switch Section---------------------- //
protected:
	
	UPROPERTY(VisibleAnywhere, Category = Tags)
		FName Volume2DTag;

	UPROPERTY(EditDefaultsOnly, Category = "2DMode")
		float CameraSwitchTime;

	bool bTransitionCameraMode;

	bool b2DMode;



	float InterpFactor;

	FRotator InterpRotation;

	void SwitchTo2DMode(bool bMode);

	// -------------------------------------------------------------------//
	//-----------------------------------------------------------------------//
	// -----------------------MOVEMENT SYSTEM-----------------------------//
public:
	// to be called from the anim blueprint
	UFUNCTION(BlueprintCallable, Category = "Movement System")
	void SetBarkourState(EBarkourState NewState);

	UFUNCTION(BlueprintPure, Category = "Movement System")
	EBarkourState GetBarkourState() const { return BarkourState; }

	UFUNCTION(BlueprintPure, Category = "Movement System")
	bool GetFocusedPressed() const { return bFocusedPressed; }

	UFUNCTION(BlueprintPure, Category = "Movement System")
	bool GetStealthMode() const { return bStealthMode; }

	UFUNCTION(BlueprintPure, Category = "Movement System")
	bool GetJogPressed() const { return bJogPressed; }
protected:
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(EditDefaultsOnly, Category = "Movement System")
	float HangingOffset;

	UPROPERTY(EditDefaultsOnly, Category = "Movement System")
	float JogSpeed;

	UPROPERTY(EditDefaultsOnly, Category = "Movement System")
	float RunSpeed;

	UPROPERTY(EditDefaultsOnly, Category = "Movement System")
	float SneakSpeed;

	UPROPERTY(EditDefaultsOnly, Category = Barkour)
	UAnimMontage* ClimbWallAnim;

	void MoveForward(float Value);

	void MoveRight(float Value);

	void TurnAtRate(float Rate);

	void LookUpAtRate(float Rate);

	virtual void Jump() override;

	virtual void StopJumping() override;

	void FocusedModeOn();

	void FocusedModeOff();

	void JogOn();

	void JogOff();

	void StealthSwitch();

private:
	float WallRunningOldAltitude;
	float WallRunningNewAltitude;
	float WallRunningFactor;

	bool bFocusedPressed;
	bool bJogPressed;
	bool bStealthMode;


	EBarkourState BarkourState;

	FVector WallTraceImpactPoint;

	FVector WallNormal;

	FVector LedgeImpactPoint;

	bool CheckCanGrab();

	bool CheckCanWallRun();

	void UpdateTracers(float DeltaTime = .1f);

	bool TraceLowerForward(FHitResult& HitOut);

	bool TraceMiddleForward(FHitResult& HitOut);

	bool TraceUpperDownward(FHitResult& HitOut);

	bool TraceUpperForward(FHitResult& HitOut);

	bool TraceWall(FVector Start, FVector End, FHitResult& HitOut);

	FVector2D MovementInput;

	FVector GetAxisDirection();

	bool bCanWallRun;
	//--------------------------------------------------------------------//
	//-------------------------------------------------------------------//




};
