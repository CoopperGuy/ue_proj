// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/DiaItem.h"
#include "Engine/Texture2D.h"
#include "Math/UnrealMathUtility.h"

#include "UI/Item/ItemName.h"

#include "Item/DiaItemWidgetComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/BoxComponent.h"

#include "System/ItemSubsystem.h"

#include "Controller/DiaController.h"

// Sets default values
ADiaItem::ADiaItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ItemMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	SetRootComponent(ItemMeshComp);

	// 새로 만든 Item 프로필 사용

	//ItemMeshComp->SetCollisionObjectType(ECollisionChannel::ECC_EngineTraceChannel1);
	//ItemMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	//ItemMeshComp->SetCollisionProfileName(TEXT("Item"));
	
	ItemMeshComp->SetNotifyRigidBodyCollision(false);
	ItemMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	ItemMeshComp->OnComponentHit.AddDynamic(this, &ADiaItem::OnHit);

	ItemWidgetComp = CreateDefaultSubobject<UDiaItemWidgetComponent>(TEXT("ItemWidget"));
	ItemWidgetComp->SetupAttachment(ItemMeshComp);
	ItemWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
	ItemWidgetComp->SetDrawSize(FVector2D(150, 30));
	ItemWidgetComp->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
	ItemWidgetComp->SetVisibility(false);
	ItemWidgetComp->SetTwoSided(true);
	// Receive Hardware Input 은 UDiaItemWidgetComponent 생성자에서 bReceiveHardwareInput = true 로 설정.

	//지형과는 충돌 판정만, 캐릭터와는 겹침(오버랩) 판정만 하도록 설정
	CollisionComp = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComp"));
	CollisionComp->SetupAttachment(ItemMeshComp);
	CollisionComp->SetCollisionProfileName(TEXT("Item"));

	//기본 사이즈 설정
	CollisionComp->SetBoxExtent(FVector(30.f, 30.f, 5.f));
	//디버깅 용 테두리 보이게하기
	CollisionComp->bHiddenInGame = false;
}

void ADiaItem::BeginPlay()
{
	Super::BeginPlay();

	LoadItemNameAsync();
}

void ADiaItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	if (bIsRolling)
	{
		CurrentRotSpeed = FMath::FInterpTo(CurrentRotSpeed, 1000.0f, DeltaTime, 100.f);

		FRotator NewRotation = FRotator(0.0f, CurrentRotSpeed, 0.0f);

		SetActorRotation(NewRotation);

		FVector Location = GetActorLocation();
		LandLocation = FMath::VInterpTo(Location, LandLocation, DeltaTime, RollingSpeed);
		SetActorLocation(LandLocation);

		//목표 Location에 거의 도달했을 때 회전 멈추고 위치 고정
		if (FVector::Dist(Location, LandLocation) < 10.f)
		{
			bIsRolling = false;
			SetActorLocation(LandLocation);
			CurrentRotSpeed = 0.f;
		}
	}
}

void ADiaItem::SetItemProperty(const FItemBase& _ItemData)
{

	//CreateInventoryInstanceByItemBase 를 통해 아이템 생성
	UItemSubsystem* ItemSubsystem = GetGameInstance()->GetSubsystem<UItemSubsystem>();
	ItemSubsystem->CreateInventoryInstanceByItemBase(InventoryItem, _ItemData);

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

void ADiaItem::DropItem(const FItemBase& ItemData)
{
	bIsRolling = true;
	RollingItem();
	SetItemName(FText::FromName(InventoryItem.ItemInstance.BaseItem.ItemID));
}

//피직스 안쓰고 그냥 돌리는 거로 변경해야함.
void ADiaItem::RollingItem()
{
	//LineTraceSingleByChannel 등으로 바닥과의 충돌 감지 후 ItemMeshComp에 물리 시뮬레이션 적용하는 방식으로 변경 필요
	//현재는 그냥 아이템이 회전하는 애니메이션만 적용되어 있음

	// 1. 필요한 변수들 준비
	FHitResult HitResult;                             // 충돌 결과 저장용
	FVector Start = GetActorLocation();            // 시작점: 캐릭터 시점(눈) 위치
	FVector DownVector = FVector::DownVector;
	FVector End = Start + (DownVector * 500.f);    // 끝점: 시작점 + (방향 * 거리)

	// 2. 쿼리 파라미터 설정 (보통 자신은 무시하도록 설정)
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this); // 나 자신은 레이에 맞지 않게 제외

	// 3. 레이트레이스 실행
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECC_WorldStatic, // 가시성 채널 사용 (보통 정적인 물체 감지)
		Params
	);

	// 4. 결과 확인 및 디버그 라인 그리기
	DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 2.0f, 0, 1.0f);

	if (bHit)
	{
		AActor* HitActor = HitResult.GetActor();
		if (HitActor)
		{
			UE_LOG(LogTemp, Warning, TEXT("충돌한 액터: %s"), *HitActor->GetName());
			// 부딪힌 위치에 작은 점 표시
			DrawDebugPoint(GetWorld(), HitResult.ImpactPoint, 10.f, FColor::Green, false, 2.f);
			LandLocation = HitResult.ImpactPoint;
		}
	}
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
	UClass* WidgetClass = WidgetAssetPtr.Get();
	if (!WidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("BindItemName: WidgetClass is NULL! 경로 확인 필요"));
		return;
	}

	ItemWidgetComp->SetWidgetClass(WidgetClass);
	ItemWidgetComp->InitWidget();
	ItemWidgetComp->EnsureReceiveHardwareInput();
	ItemWidgetComp->SetVisibility(false);

	UUserWidget* UserWidget = ItemWidgetComp->GetUserWidgetObject();
	if (!UserWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("BindItemName: GetUserWidgetObject() returned NULL!"));
		return;
	}

	UItemName* NameWidget = Cast<UItemName>(UserWidget);
	if (IsValid(NameWidget))
	{
		NameWidget->SetVisibility(ESlateVisibility::Collapsed);
		NameWidget->OnItemNameClicked.AddDynamic(this, &ADiaItem::OnItemNameClicked);

		// 대기 중인 이름 설정이 있으면 적용
		if (bPendingNameSet)
		{
			SetItemName(PendingItemName);
			bPendingNameSet = false;
		}
	}
}

void ADiaItem::SetItemName(const FText& NewName)
{
	UItemName* NameWidget = Cast<UItemName>(ItemWidgetComp->GetUserWidgetObject());
	if (IsValid(NameWidget))
	{
		NameWidget->SetItemName(InventoryItem.ItemInstance.BaseItem.Name);
		NameWidget->SetVisibility(ESlateVisibility::Visible);
		ItemWidgetComp->SetVisibility(true);
		bPendingNameSet = false;
	}
	else
	{
		// 위젯이 아직 준비되지 않음 - 나중에 적용하도록 저장
		bPendingNameSet = true;
		PendingItemName = NewName;
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