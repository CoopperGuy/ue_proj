// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DiaPortal.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
UCLASS()
class ARPG_API ADiaPortal : public AActor
{
	GENERATED_BODY()
	
public:	
	ADiaPortal();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION()
	void OnPortalCursorOver();

	UFUNCTION()
	void OnPortalCursorOut();

	UFUNCTION()
    void OnPortalClicked(AActor* TouchedActor, FKey ButtonPressed);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	UBoxComponent* PortalCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	UStaticMeshComponent* PortalMesh;

	bool bCursorHoveringPortal = false;
};
