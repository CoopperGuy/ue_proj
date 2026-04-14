// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Types/ItemBase.h"
#include "Item/DiaItemWidgetComponent.h"
#include "DiaItem.generated.h"

class UStaticMeshComponent;
class UTexture2D;
class ADiaController;
class UBoxComponent;
class UProjectileMovementComponent;
UCLASS()
class ARPG_API ADiaItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADiaItem();

	void SetItemProperty(const FItemBase& _ItemData);
	void DropItem(const FItemBase& ItemData);

	void RollingItem();
	
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, 
		AActor* OtherActor, UPrimitiveComponent* OtherComp, 
		FVector NormalImpulse, const FHitResult& Hit);
	UFUNCTION()
	void OnItemNameClicked();

	UFUNCTION()
	void OnItemLanded(const FHitResult& Hit);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void LoadItemNameAsync();

	void BindItemName(TSoftClassPtr<UUserWidget>& WidgetAssetPtr);
	void SetItemName(const FText& NewName);
	const FVector ComputeDropTarget() const;
private:
	ADiaController* FindBestPlayerForPickup();
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess = "true"))
	FInventorySlot InventoryItem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* ItemMeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess = "true"))
	UTexture2D* ItemIcon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess = "true"))
	UDiaItemWidgetComponent* ItemWidgetComp;

	UPROPERTY()
	UBoxComponent* CollisionComp;

	UPROPERTY()
	UProjectileMovementComponent* ProjectileMovementComp;

	float CurrentRotSpeed;
	bool bIsRolling;
	float CachedBoxHalfHeight;

	FVector LandLocation = FVector::ZeroVector;
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess = "true"))
	float RollingSpeed = 1000.0f;

	// 위젯 준비 전에 이름 설정 요청이 들어온 경우 저장
	bool bPendingNameSet = false;
	FText PendingItemName;
};
