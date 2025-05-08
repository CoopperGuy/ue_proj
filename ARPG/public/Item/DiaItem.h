// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Types/ItemBase.h"
#include "DiaItem.generated.h"

class UStaticMeshComponent;
class UTexture2D;
UCLASS()
class ARPG_API ADiaItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADiaItem();

	void SetItemProperty(const FItemBase& _ItemData);
	void DropItem();

	//테스트용 블루프린트 함수 설정
	UFUNCTION(BlueprintPure, Category = "Item")
	void RollingItem();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess = "true"))
	FItemBase ItemData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* ItemMeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess = "true"))
	UTexture2D* ItemIcon;
};
