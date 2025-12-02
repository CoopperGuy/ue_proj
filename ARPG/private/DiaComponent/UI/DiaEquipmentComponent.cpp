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

void UDiaEquipmentComponent::UnEquipItem(EEquipmentSlot Slot)
{
	//아이템 제거
	if (FEquippedItem* EquippedItem = EquipmentMap.Find(Slot))
	{
		//스텟에서 제거
		ApplyEquipmentStats(*EquippedItem, Slot, -1);
		EquipmentMap.Remove(Slot);
		OnItemUnEquipped.Broadcast(Slot);
		UE_LOG(LogTemp, Log, TEXT("UnEquipItem: Slot %s에서 아이템 제거됨."), *UEnum::GetValueAsString(Slot));
	}
}

//이걸 DiaOptionManagerComponent로 옮길까?
void UDiaEquipmentComponent::ApplyEquipmentStats(const FEquippedItem& Item, EEquipmentSlot Slot, int32 State)
{
	//gameplayeffect를 이용해 스탯 적용
	ADiaController* Controller = Cast<ADiaController>(GetOwner());
	if (!IsValid(Controller))
	{
		UE_LOG(LogTemp, Warning, TEXT("ApplyEquipmentStats: Controller가 유효하지 않습니다."));
		return;
	}

	ADiaBaseCharacter* OwnerCharacter = Controller->GetPawn<ADiaBaseCharacter>();
	if (!IsValid(OwnerCharacter))
	{
		UE_LOG(LogTemp, Warning, TEXT("ApplyEquipmentStats: OwnerCharacter가 유효하지 않습니다."));
		return;
	}

	UAbilitySystemComponent* ASC = OwnerCharacter->GetAbilitySystemComponent();
	if (!IsValid(ASC))
	{
		UE_LOG(LogTemp, Warning, TEXT("ApplyEquipmentStats: AbilitySystemComponent가 유효하지 않습니다."));
		return;
	}

	FGameplayEffectSpecHandle SpecHandle
		= ASC->MakeOutgoingSpec(UDiaGE_StatApply::StaticClass(), 1.0f, ASC->MakeEffectContext());
	if (!SpecHandle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("ApplyEquipmentStats: SpecHandle 생성 실패"));
		return;
	}

	FGameplayEffectSpec* Spec = SpecHandle.Data.Get();
	check(Spec)

	int32 StatCount = Item.ItemInstance.ModifiedStats.Num();

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
			Spec->SetSetByCallerMagnitude(StatTag, StatInfo->Values * State);  // 값 설정
			AppliedCount++;
		}
		else
		{
			Spec->SetSetByCallerMagnitude(StatTag, 0.f);  // 값 설정
		}
	}

	FActiveGameplayEffectHandle EffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*Spec);
}
