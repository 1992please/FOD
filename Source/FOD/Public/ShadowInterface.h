// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShadowInterface.generated.h"

UENUM(BlueprintType)
enum class EShadowMode : uint8
{
	Off,
	On
};

// This class does not need to be modified.
UINTERFACE(Blueprintable)
class UShadowInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class FOD_API IShadowInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/** React to a trigger volume activating this object. Return true if the reaction succeeds. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Shadow")
	void ActivateShadow(EShadowMode bMode);
	virtual void ActivateShadow_Implementation(EShadowMode bMode) = 0;

};
