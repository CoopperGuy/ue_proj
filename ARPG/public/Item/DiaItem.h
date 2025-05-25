// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Types/ItemBase.h"
#include "DiaItem.generated.h"

class UStaticMeshComponent;
class UTexture2D;
class UWidgetComponent;
class ADiaController;
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
	UFUNCTION(BlueprintCallable, Category = "Item")
	void RollingItem();
	
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, 
		AActor* OtherActor, UPrimitiveComponent* OtherComp, 
		FVector NormalImpulse, const FHitResult& Hit);
	UFUNCTION()
	void OnItemNameClicked();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void LoadItemNameAsync();

	void BindItemName(TSoftClassPtr<UUserWidget>& WidgetAssetPtr);
	void SetItemName(const FText& NewName);
private:
	ADiaController* FindBestPlayerForPickup();
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess = "true"))
	FInventoryItem InventoryItem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* ItemMeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess = "true"))
	UTexture2D* ItemIcon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* ItemWidgetComp;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess = "true"))
	float RollingSpeed = 1000.0f;
};
