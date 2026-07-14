// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/DiaInteractable.h"
#include "DiaPortal.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
UCLASS()
class ARPG_API ADiaPortal : public AActor, public IDiaInteractable
{
	GENERATED_BODY()
	
public:	
	ADiaPortal();

	virtual bool IsInteractable() const override { return true; }
	virtual void OnInteract(APlayerController* InteractingController) override;
protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnPortalCursorOver();

	UFUNCTION()
	void OnPortalCursorOut();
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	UBoxComponent* PortalCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	UStaticMeshComponent* PortalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	FName DestinationLevel;

	bool bCursorHoveringPortal = false;
public:
	const FName& GetDestinationLevel() const { return DestinationLevel; }
};
