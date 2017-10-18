// Fill out your copyright notice in the Description page of Project Settings.

#include "FODGameModeBase.h"
#include "ShadowInterface.h"
#include "EngineUtils.h"

AFODGameModeBase::AFODGameModeBase()
{
	bShadowWorld = false;
}

void AFODGameModeBase::SwitchShadow(bool bShadowMode)
{
	if (bShadowMode == bShadowWorld)
		return;

	bShadowWorld = !bShadowWorld;
	UWorld* World = GetWorld();
	// We do nothing if not class provided, rather than giving ALL actors!
	if (World)
	{
		for (FActorIterator It(World); It; ++It)
		{
			AActor* Actor = *It;
			IShadowInterface* ShadowObject = Cast<IShadowInterface>(Actor);
			if (ShadowObject && !Actor->IsPendingKill())
			{
				ShadowObject->ActivateShadow((EShadowMode)bShadowWorld);
			}
		}
	}
}

bool AFODGameModeBase::IsShadowWorldActive() const
{
	return bShadowWorld;
}

