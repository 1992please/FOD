// Fill out your copyright notice in the Description page of Project Settings.

#include "ShadowInterface.h"

#define print(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::Red,text)

// Add default functionality here for any IShadowInterface functions that are not pure virtual.

void IShadowInterface::ActivateShadow_Implementation(EShadowMode bMode)
{
	print("IShadowInterface ActivateShadow_Implementation");
}
