// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaComponent/UI/DiaEquipmentComponent.h"
#include "DiaComponent/DiaStatComponent.h"
#include "Controller/DiaController.h"
#include "DiaBaseCharacter.h"
#include "AbilitySystemComponent.h"

#include "GAS/Effects/DiaGE_StatApply.h"
#include "GAS/DiaAttributeSet.h"
#include "GAS/DiaGameplayTags.h"

#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"

UDiaEquipmentComponent::UDiaEquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}


void UDiaEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void UDiaEquipmentComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UDiaEquipmentComponent::EquipItem(const FEquippedItem& Item, EEquipmentSlot Slot)
{
	//아이템 추가
	EquipmentMap.Add(Slot, Item);

	//스텟에 적용
	ApplyEquipmentStats(Item, Slot);
}

void UDiaEquipmentComponent::UnEquipItem()
{
}

void UDiaEquipmentComponent::ApplyEquipmentStats(const FEquippedItem& Item, EEquipmentSlot Slot)
{
	UE_LOG(LogTemp, Log, TEXT("=== ApplyEquipmentStats 시작 ==="));
	UE_LOG(LogTemp, Log, TEXT("Slot: %s, ItemID: %s"), 
		*UEnum::GetValueAsString(Slot),
		Item.ItemInstance.IsValid() ? *Item.ItemInstance.BaseItem.ItemID.ToString() : TEXT("Invalid"));

	//gameplayeffect를 이용해 스탯 적용
	ADiaController* Controller = Cast<ADiaController>(GetOwner());
	if (!IsValid(Controller))
	{
		UE_LOG(LogTemp, Warning, TEXT("ApplyEquipmentStats: Controller가 유효하지 않습니다."));
		return;
	}
	UE_LOG(LogTemp, Verbose, TEXT("Controller 유효성 확인 완료"));

	ADiaBaseCharacter* OwnerCharacter = Controller->GetPawn<ADiaBaseCharacter>();
	if (!IsValid(OwnerCharacter))
	{
		UE_LOG(LogTemp, Warning, TEXT("ApplyEquipmentStats: OwnerCharacter가 유효하지 않습니다."));
		return;
	}
	UE_LOG(LogTemp, Verbose, TEXT("OwnerCharacter 유효성 확인 완료: %s"), *OwnerCharacter->GetName());

	UAbilitySystemComponent* ASC = OwnerCharacter->GetAbilitySystemComponent();
	if (!IsValid(ASC))
	{
		UE_LOG(LogTemp, Warning, TEXT("ApplyEquipmentStats: AbilitySystemComponent가 유효하지 않습니다."));
		return;
	}
	UE_LOG(LogTemp, Verbose, TEXT("ASC 유효성 확인 완료"));

	FGameplayEffectSpecHandle SpecHandle
		= ASC->MakeOutgoingSpec(UDiaGE_StatApply::StaticClass(), 1.0f, ASC->MakeEffectContext());
	if (!SpecHandle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("ApplyEquipmentStats: SpecHandle 생성 실패"));
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("SpecHandle 생성 성공"));

	FGameplayEffectSpec* Spec = SpecHandle.Data.Get();
	check(Spec)

	//미리 정의된 Stats을 적용.
	int32 StatCount = Item.ItemInstance.ModifiedStats.Num();
	UE_LOG(LogTemp, Log, TEXT("적용할 스탯 개수: %d"), StatCount);

	if (StatCount == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("ApplyEquipmentStats: ModifiedStats가 비어있습니다."));
		return;
	}


	TArray<FGameplayTag> StatTags = FDiaGameplayTags::GetAttributeStats();

	int32 AppliedCount = 0;
	for (const FGameplayTag& StatTag : StatTags)
	{
		if (const auto StatInfo = Item.ItemInstance.ModifiedStats.Find(StatTag))
		{
			Spec->SetSetByCallerMagnitude(StatTag, StatInfo->Values);  // 값 설정
			AppliedCount++;

			UE_LOG(LogTemp, Log, TEXT("스탯 적용 중 - Tag: %s, Value: %.2f"),
				*StatTag.ToString(), StatInfo->Values);
		}
		else
		{
			Spec->SetSetByCallerMagnitude(StatTag, 0.f);  // 값 설정
		}
	}

	FActiveGameplayEffectHandle EffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*Spec);

	UE_LOG(LogTemp, Log, TEXT("=== ApplyEquipmentStats 완료 - 총 %d/%d 개 스탯 적용됨 ==="), AppliedCount, StatCount);
}
