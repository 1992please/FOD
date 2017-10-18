// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ShadowInterface.h"
#include "CoreMinimal.h"
#include "Engine/PointLight.h"
#include "SPointLight.generated.h"

class AHeroCharacter;
class USphereComponent;
/**
 * 
 */
UCLASS()
class FOD_API ASPointLight : public APointLight, public IShadowInterface
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = DamageCurve)
	UCurveFloat* DamageCurve;

	UPROPERTY(EditAnywhere, Category = DamageCurve)
	float DamageTimerInterval;

	UPROPERTY(EditAnywhere, Category = DamageCurve)
	float DamagePerSec;
private:
	ASPointLight();

	AHeroCharacter* DamagedPlayer;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USphereComponent* Sphere;

	// override Actor
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;
	virtual void PostInitializeComponents() override;
	virtual void Tick(float DeltaSeconds) override;

	//End of override
	virtual void ActivateShadow_Implementation(EShadowMode bMode) override;

private:
	float DamageTimer;
	bool bCanDamage;
	void UpdateDealDamageToPlayer(float DeltaSeconds);
protected:
	//overriden to avoid Erros
#if WITH_EDITOR
	//~ Begin AActor Interface.
	virtual void EditorApplyScale(const FVector& DeltaScale, const FVector* PivotLocation, bool bAltDown, bool bShiftDown, bool bCtrlDown) override;
	//~ End AActor Interface.
#endif

	//~ Begin UObject Interface.
	virtual void PostLoad() override;
#if WITH_EDITOR
	virtual void LoadedFromAnotherClass(const FName& OldClassName) override;
#endif
	//~ End UObject Interface.
	
};
