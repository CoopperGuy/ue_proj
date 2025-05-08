// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/DiaItem.h"
#include "Engine/Texture2D.h"
#include "Math/UnrealMathUtility.h"
#include "Components/StaticMeshComponent.h"


// Sets default values
ADiaItem::ADiaItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ItemMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	SetRootComponent(ItemMeshComp);
}

void ADiaItem::SetItemProperty(const FItemBase& _ItemData)
{
	ItemData = _ItemData;

	//아이템 스태틱 매시 로딩
	UStaticMesh* ItemAsset = ItemData.ItemMesh.LoadSynchronous();
	if (ItemAsset)
	{
		// 메시 설정
		ItemMeshComp->SetStaticMesh(ItemAsset);

		// 중요: 메시 설정 후 강제 업데이트
		ItemMeshComp->RecreateRenderState_Concurrent();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("아이템 [%s] 메시 로드 실패: %s"), *GetName(), *ItemData.ItemMesh.ToString());
	}
	
	//아이템 아이콘 및 기타 등등 변경
	if (ItemData.IconPath.IsValid())
	{
		UObject* LoadedObj = ItemData.IconPath.TryLoad();
		UTexture2D* Texture = Cast<UTexture2D>(LoadedObj);
		if (IsValid(Texture))
		{
			ItemIcon = Texture;
		}
		else
		{
			UE_LOG(LogTemp, Warning, 
				TEXT("아이템 [%s] 아이콘 로드 실패: %s"), *GetName(), *ItemData.IconPath.ToString());
		}
	}
}

void ADiaItem::DropItem()
{

}

void ADiaItem::RollingItem()
{
	ItemMeshComp->SetSimulatePhysics(true);
	ItemMeshComp->SetEnableGravity(true);

	FVector Impulse = FVector(FMath::FRandRange(-100.f, 100.f), FMath::FRandRange(-100.f, 100.f), 400.0f);
	ItemMeshComp->AddImpulse(Impulse, NAME_None, true);

	FVector Angular = FVector(FMath::FRandRange(200.f, 400.f), FMath::FRandRange(200.f, 400.f), FMath::FRandRange(200.f, 400.f));
	ItemMeshComp->SetPhysicsAngularVelocityInDegrees(Angular, true);

}

// Called when the game starts or when spawned
void ADiaItem::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADiaItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

