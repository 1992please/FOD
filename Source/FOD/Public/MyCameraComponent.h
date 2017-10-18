// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "MyCameraComponent.generated.h"

/**
 * 
 */
UCLASS()
class FOD_API UMyCameraComponent : public UCameraComponent
{
	GENERATED_BODY()
public:
	UMyCameraComponent();

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction);

	void SwitchToOrthogonal(bool bMode);

	void UpdateSwitching(float Factor);

	UPROPERTY(EditDefaultsOnly, Category = Camera)
		float SwitchTime;

private:
	bool bSwitching;

	bool bOrtho;

	float Timer;
	
	
};
