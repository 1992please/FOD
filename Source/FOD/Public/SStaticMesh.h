// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "ShadowInterface.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SStaticMesh.generated.h"

UCLASS()
class FOD_API ASStaticMesh : public AActor, public IShadowInterface
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	ASStaticMesh();

	virtual void ActivateShadow_Implementation(EShadowMode bMode) override;
protected:
	UPROPERTY(VisibleAnywhere, Category = Mesh)
	UStaticMeshComponent* NormalMesh;
	UPROPERTY(VisibleAnywhere, Category = Mesh)
	UStaticMeshComponent* ShadowMesh;

	
	
};
