// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FODGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class FOD_API AFODGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
public:
	AFODGameModeBase();

	void SwitchShadow(bool bShadowMode);

	UFUNCTION(BlueprintPure, Category = ShadowGamePlay)
		bool IsShadowWorldActive() const;

private:
	bool bShadowWorld;
	
	
};
