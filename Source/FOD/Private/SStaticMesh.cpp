// Fill out your copyright notice in the Description page of Project Settings.

#include "SStaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
// Sets default values
ASStaticMesh::ASStaticMesh()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	NormalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Normal Mesh"));
	NormalMesh->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	NormalMesh->Mobility = EComponentMobility::Static;
	NormalMesh->bGenerateOverlapEvents = false;
	NormalMesh->bUseDefaultCollision = true;

	RootComponent = NormalMesh;

	ShadowMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Shadow Mesh"));
	ShadowMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	ShadowMesh->Mobility = EComponentMobility::Static;
	ShadowMesh->bGenerateOverlapEvents = false;
	ShadowMesh->SetVisibility(false);
	ShadowMesh->bUseDefaultCollision = true;
	ShadowMesh->SetupAttachment(RootComponent);
}


void ASStaticMesh::ActivateShadow_Implementation(EShadowMode Mode)
{
	NormalMesh->SetVisibility(!(bool)Mode);
	ShadowMesh->SetVisibility((bool)Mode);
}

