// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/DiaItem.h"
#include "Engine/Texture2D.h"
#include "Math/UnrealMathUtility.h"

#include "UI/Item/ItemName.h"

#include "Components/WidgetComponent.h"
#include "Components/StaticMeshComponent.h"

#include "Controller/DiaController.h"

// Sets default values
ADiaItem::ADiaItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ItemMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	SetRootComponent(ItemMeshComp);

	// 새로 만든 Item 프로필 사용

	ItemMeshComp->SetCollisionObjectType(ECollisionChannel::ECC_EngineTraceChannel1);
	ItemMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ItemMeshComp->SetCollisionProfileName(TEXT("Item"));
	
	ItemMeshComp->SetNotifyRigidBodyCollision(true);
	ItemMeshComp->OnComponentHit.AddDynamic(this, &ADiaItem::OnHit);

	ItemWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("ItemWidget"));
	ItemWidgetComp->SetupAttachment(ItemMeshComp);
	ItemWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
	ItemWidgetComp->SetDrawSize(FVector2D(150, 30));
	ItemWidgetComp->SetRelativeLocation(FVector(0.f, 0.f, 100.f)); 
	ItemWidgetComp->SetVisibility(false);
	ItemWidgetComp->SetTwoSided(true);

}

void ADiaItem::BeginPlay()
{
	Super::BeginPlay();

	LoadItemNameAsync();
}

void ADiaItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ADiaItem::SetItemProperty(const FItemBase& _ItemData)
{
	InventoryItem = FInventorySlot::FromBase(_ItemData);

	//아이템 스태틱 매시 로딩
	//현재 아이템 스태틱 매시 첫 로딩 이후 아이템 메시가 보이지 않는 현상 존재
	//두번째 생성 시에 보임
	//수정 필요하다.
	UStaticMesh* ItemAsset = _ItemData.ItemMesh.LoadSynchronous();
	if (ItemAsset)
	{
		// 메시 설정
		ItemMeshComp->SetStaticMesh(ItemAsset);

		// 중요: 메시 설정 후 강제 업데이트
		ItemMeshComp->RecreateRenderState_Concurrent();
		UE_LOG(LogTemp, Warning, TEXT("아이템 [%s] 메시 로드 성공: %s"), *GetName(), *_ItemData.ItemMesh.ToString());

	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("아이템 [%s] 메시 로드 실패: %s"), *GetName(), *_ItemData.ItemMesh.ToString());
	}
	
	//아이템 아이콘 및 기타 등등 변경
	if (_ItemData.IconPath.IsValid())
	{
		UObject* LoadedObj = _ItemData.IconPath.TryLoad();
		UTexture2D* Texture = Cast<UTexture2D>(LoadedObj);
		if (IsValid(Texture))
		{
			ItemIcon = Texture;
			UE_LOG(LogTemp, Warning,
				TEXT("아이템 [%s] 아이콘 로드 성공: %s"), *GetName(), *_ItemData.IconPath.ToString());
		}
		else
		{
			UE_LOG(LogTemp, Warning, 
				TEXT("아이템 [%s] 아이콘 로드 실패: %s"), *GetName(), *_ItemData.IconPath.ToString());
		}
	}
}

void ADiaItem::DropItem()
{
	RollingItem();
	SetItemName(FText::FromName(InventoryItem.ItemInstance.BaseItem.ItemID));
}

void ADiaItem::RollingItem()
{
	ItemMeshComp->SetSimulatePhysics(true);
	ItemMeshComp->SetEnableGravity(true);

	FVector Impulse = FVector(
		FMath::FRandRange(-100.f, 100.f), 
		FMath::FRandRange(-100.f, 100.f), 
		RollingSpeed);
	ItemMeshComp->AddImpulse(Impulse, NAME_None, true);

	FVector Angular = FVector(
		FMath::FRandRange(200.f, 400.f), 
		FMath::FRandRange(200.f, 400.f), 
		FMath::FRandRange(200.f, 400.f));

	ItemMeshComp->SetPhysicsAngularVelocityInDegrees(Angular, true);

}


void ADiaItem::LoadItemNameAsync()
{
	// 비동기 방식으로 위젯 로드
	TSoftClassPtr<UUserWidget> WidgetAssetPtr;
	WidgetAssetPtr = TSoftClassPtr<UUserWidget>(FSoftObjectPath(TEXT("/Game/UI/WorldWidget/WBP_ItemName3D.WBP_ItemName3D_C")));

	if (WidgetAssetPtr.IsValid())
	{
		BindItemName(WidgetAssetPtr);
	}
	else
	{
		// 비동기 로드 시작
		WidgetAssetPtr.LoadSynchronous();

		if (WidgetAssetPtr.Get())
		{
			BindItemName(WidgetAssetPtr);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("아이템 위젯 클래스 비동기 로드 실패"));
		}
	}
}

void ADiaItem::BindItemName(TSoftClassPtr<UUserWidget>& WidgetAssetPtr)
{
	ItemWidgetComp->SetWidgetClass(WidgetAssetPtr.Get());
	ItemWidgetComp->SetVisibility(false);

	UItemName* NameWidget = Cast<UItemName>(ItemWidgetComp->GetUserWidgetObject());
	if (IsValid(NameWidget))
	{
		NameWidget->SetVisibility(ESlateVisibility::Collapsed);
		NameWidget->OnItemNameClicked.AddDynamic(this, &ADiaItem::OnItemNameClicked);
	}
	UE_LOG(LogTemp, Warning, TEXT("아이템 위젯 클래스 비동기 로드 성공"));
}

void ADiaItem::SetItemName(const FText& NewName)
{
	UItemName* NameWidget = Cast<UItemName>(ItemWidgetComp->GetUserWidgetObject());
	if (IsValid(NameWidget))
	{
		NameWidget->SetItemName(InventoryItem.ItemInstance.BaseItem.Name);
		NameWidget->SetVisibility(ESlateVisibility::Visible);

		ItemWidgetComp->SetVisibility(true);
	}
}

void ADiaItem::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor)
	{
	}
}

void ADiaItem::OnItemNameClicked()
{
	if (UWidgetComponent* WidgetComp = GetComponentByClass<UWidgetComponent>())
	{
		if (UUserWidget* Widget = WidgetComp->GetUserWidgetObject())
		{
			if (UItemName* NameWidget = Cast<UItemName>(Widget))
			{
				NameWidget->SetItemName(FText::FromName(InventoryItem.ItemInstance.BaseItem.ItemID));
			}
		}
	}

	// 가장 가까운 플레이어에게 아이템 전달
	if (ADiaController* PlayerController = FindBestPlayerForPickup())
	{
		FInventorySlot ItemToAdd = InventoryItem;
		UE_LOG(LogTemp, Log, TEXT("아이템 클릭됨: %s"), *InventoryItem.ItemInstance.BaseItem.ItemID.ToString());

		// DiaItem에서 가장 적절한 플레이어 찾기

		bool bSuccess = PlayerController->ItemAddedToInventory(ItemToAdd);
		if (bSuccess)
		{
			UE_LOG(LogTemp, Log, TEXT("아이템이 %s의 인벤토리에 추가됨"),
				*PlayerController->GetName());

			// 아이템 제거
			if (IsValid(ItemWidgetComp))
			{
				ItemWidgetComp->SetVisibility(false);
			}
			Destroy();
		}

		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);
		SetActorTickEnabled(false);
		ItemWidgetComp->SetVisibility(false);
	}
}

ADiaController* ADiaItem::FindBestPlayerForPickup()
{
	UWorld* World = GetWorld();
	if (!World) return nullptr;
	
	ADiaController* BestController = nullptr;
	float ClosestDistance = MAX_FLT;
	const float MaxPickupDistance = 500.0f; // 픽업 가능 거리
	
	// 모든 플레이어 컨트롤러 검사
	for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		if (ADiaController* DiaController = Cast<ADiaController>(Iterator->Get()))
		{
			APawn* Player = DiaController->GetPawn();
			if (Player)
			{
				float Distance = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
				
				// 픽업 가능 거리 내에서 가장 가까운 플레이어
				if (Distance <= MaxPickupDistance && Distance < ClosestDistance)
				{
					ClosestDistance = Distance;
					BestController = DiaController;
				}
			}
		}
	}
	
	if (!BestController)
	{
		UE_LOG(LogTemp, Warning, TEXT("픽업 가능한 거리에 플레이어가 없습니다 (최대 거리: %.1f)"), MaxPickupDistance);
	}
	
	return BestController;
}