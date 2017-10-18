// Fill out your copyright notice in the Description page of Project Settings.

#include "MyCameraComponent.h"

UMyCameraComponent::UMyCameraComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bSwitching = false;
	bOrtho = false;
	Timer = 0;
	SwitchTime = .1f;
}

void UMyCameraComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bSwitching)
	{
		Timer += DeltaTime;
		UpdateSwitching(Timer / SwitchTime);
	}
}

void UMyCameraComponent::SwitchToOrthogonal(bool bOrthoMode)
{
	bSwitching = true;
	bOrtho = bOrthoMode;
	Timer = 0;
	if (!bOrthoMode)
	{
		ProjectionMode = ECameraProjectionMode::Perspective;
	}
}

void UMyCameraComponent::UpdateSwitching(float Factor)
{
	if (Factor <= .5)
	{
		Factor *= 2;
		FieldOfView = FMath::Lerp(90, 120, Factor);
	}
	else if (Factor <= 1)
	{
		Factor = (Factor - .5f) * 2;
		FieldOfView = FMath::Lerp(120, 90, Factor);
	}
	else
	{
		if (bOrtho)
		{
			ProjectionMode = ECameraProjectionMode::Orthographic;
			OrthoNearClipPlane = 610.09021f;
			OrthoWidth = 2908.819336;
		}
		//reset variables
		bSwitching = false;
	}
}



