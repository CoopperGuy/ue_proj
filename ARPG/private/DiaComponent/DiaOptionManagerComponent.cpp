// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaComponent/DiaOptionManagerComponent.h"
#include "Controller/DiaController.h"
#include "DiaBaseCharacter.h"
#include "AbilitySystemComponent.h"

#include "GAS/DiaAttributeSet.h"

#include "GAS/Effects/DiaGE_StatApply.h"
#include "GAS/DiaGameplayTags.h"
#include "GAS/Effects/DiaGE_OptionGeneric.h"


UDiaOptionManagerComponent::UDiaOptionManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UDiaOptionManagerComponent::BeginPlay()
{
	Super::BeginPlay();	
}

void UDiaOptionManagerComponent::AddOption(const FDiaActualItemOption& NewOption)
{
	ActiveOptions.Emplace(NewOption.OptionID, NewOption);
}

void UDiaOptionManagerComponent::RemoveOption(const FDiaActualItemOption& OptionRow)
{
	RemoveOption(OptionRow.OptionID);
}

void UDiaOptionManagerComponent::RemoveOption(const FName& OptionID)
{
	ActiveOptions.Remove(OptionID);
}

FDiaActualItemOption* UDiaOptionManagerComponent::GetOptionByID(const FName& OptionID)
{
	if(FDiaActualItemOption* FoundOption = ActiveOptions.Find(OptionID))
	{
		return FoundOption;
	}
	return nullptr;
}

FDiaActualItemOption* UDiaOptionManagerComponent::GetOptionByOptionRow(const FDiaActualItemOption& OptionRow)
{
	return GetOptionByID(OptionRow.OptionID);
}

void UDiaOptionManagerComponent::ApplyEquipmentStats(const FEquippedItem& Item, EEquipmentSlot Slot)
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
	for (const FGameplayTag& StatTag : StatTags)
	{
		if (const auto StatInfo = Item.ItemInstance.ModifiedStats.Find(StatTag))
		{
			Spec->SetSetByCallerMagnitude(StatTag, StatInfo->Values);  // 값 설정
			UE_LOG(LogTemp, Warning, TEXT("스탯 적용: %s = %f"), *StatTag.ToString(), StatInfo->Values);
		}
		else
		{
			Spec->SetSetByCallerMagnitude(StatTag, 0.f);  // 값 설정
		}
	}

	FActiveGameplayEffectHandle EffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*Spec);
	AppliedStatHandles.FindOrAdd(Item.ItemInstance.InstanceID, EffectHandle);
	UE_LOG(LogTemp, Warning, TEXT("스탯 적용 완료: ItemID = %s"), *Item.ItemInstance.InstanceID.ToString());
}

void UDiaOptionManagerComponent::RemoveEqipmentStats(const FEquippedItem& Item, EEquipmentSlot Slot)
{
	const FGuid& ItemID = Item.ItemInstance.InstanceID;
	if (!AppliedStatHandles.Contains(ItemID))
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveEqipmentStats: 해당 아이템의 스탯이 적용되어 있지 않습니다. ItemID = %s"), *ItemID.ToString());
		return;
	}

	FActiveGameplayEffectHandle& EffectHandle = AppliedStatHandles[ItemID];

	//gameplayeffect를 이용해 스탯 제거
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

	ASC->RemoveActiveGameplayEffect(EffectHandle);
	AppliedStatHandles.Remove(ItemID);
	UE_LOG(LogTemp, Warning, TEXT("스탯 제거: ItemID = %s"), *ItemID.ToString());
	//옵션도 동시에 제거
	if (FActiveGameplayEffectHandle* PrefixHandle = AppliedPrefixHandles.Find(Item.ItemInstance.InstanceID))
	{
		ASC->RemoveActiveGameplayEffect(*PrefixHandle);
		AppliedPrefixHandles.Remove(Item.ItemInstance.InstanceID);
	}
	if (FActiveGameplayEffectHandle* SuffixHandle = AppliedSuffixHandles.Find(Item.ItemInstance.InstanceID))
	{
		ASC->RemoveActiveGameplayEffect(*SuffixHandle);
		AppliedSuffixHandles.Remove(Item.ItemInstance.InstanceID);
	}
}

void UDiaOptionManagerComponent::ApplyEquipmentSlotOption(const FEquippedItem& Item)
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

	FActiveGameplayEffectHandle PrefixHandle = MakeGameplayEffectOptions(ASC, Item.ItemInstance.PrefixOptions);
	FActiveGameplayEffectHandle SuffixHandle = MakeGameplayEffectOptions(ASC, Item.ItemInstance.SuffixOptions);
	if(PrefixHandle.IsValid())
	{
		AppliedPrefixHandles.FindOrAdd(Item.ItemInstance.InstanceID, PrefixHandle);
	}
	if (SuffixHandle.IsValid())
	{
		AppliedSuffixHandles.FindOrAdd(Item.ItemInstance.InstanceID, SuffixHandle);
	}
}

FActiveGameplayEffectHandle UDiaOptionManagerComponent::MakeGameplayEffectOptions(UAbilitySystemComponent* ASC, const TArray<FDiaActualItemOption>& ItemOptions)
{
	FGameplayEffectSpecHandle SpecHandle
		= ASC->MakeOutgoingSpec(UDiaGE_OptionGeneric::StaticClass(), 1.0f, ASC->MakeEffectContext());
	if (!SpecHandle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("ApplyEquipmentStats: SpecHandle 생성 실패"));
		return FActiveGameplayEffectHandle();
	}

	FGameplayEffectSpec* Spec = SpecHandle.Data.Get();
	check(Spec)
	//UE_LOG(LogTemp, Warning, TEXT("옵션 개수: %d"), ItemOptions.Num());
	ApplyitemOptionsToSpec(ItemOptions, Spec);
	FActiveGameplayEffectHandle EffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*Spec);

	return EffectHandle;
}

void UDiaOptionManagerComponent::ApplyitemOptionsToSpec(const TArray<FDiaActualItemOption>& ItemOptions, FGameplayEffectSpec* Spec)
{
	TArray<FGameplayTag> AllOptionTags = FDiaGameplayTags::Get().GetItemOptionList();
	
	// 태그 -> 값 맵핑 (인덱스가 아닌 실제 값을 저장)
	TMap<FGameplayTag, float> OptionValueMap;
	for (const auto& OptionRow : ItemOptions)
	{
		OptionValueMap.Add(OptionRow.GrantedTag, OptionRow.Value);
		//UE_LOG(LogTemp, Warning, TEXT("옵션 준비: %s = %f"), *OptionRow.GrantedTag.ToString(), OptionRow.Value);
	}

	//int32 i = 0;
	// 모든 옵션 태그에 대해 SetByCaller 설정
	for (const FGameplayTag& OptionTag : AllOptionTags)
	{
		if (const float* FoundValue = OptionValueMap.Find(OptionTag))
		{
			Spec->SetSetByCallerMagnitude(OptionTag, *FoundValue);
			//UE_LOG(LogTemp, Warning, TEXT("옵션 적용: %s = %f, 인덱스 : %d"), *OptionTag.ToString(), *FoundValue, i);
		}
		else
		{
			Spec->SetSetByCallerMagnitude(OptionTag, 0.f);
			//UE_LOG(LogTemp, Warning, TEXT("옵션 적용 안됨(0으로 설정): %s, 인덱스 : %d"), *OptionTag.ToString(), i);
		}
		//++i;
	}	
}

void UDiaOptionManagerComponent::ApplyEquipmentAllOptions()
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
		= ASC->MakeOutgoingSpec(UDiaGE_OptionGeneric::StaticClass(), 1.0f, ASC->MakeEffectContext());
	if (!SpecHandle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("ApplyEquipmentStats: SpecHandle 생성 실패"));
		return;
	}

	FGameplayEffectSpec* Spec = SpecHandle.Data.Get();
	check(Spec)

	for(const auto& OptionPair : ActiveOptions)
	{
		const FDiaActualItemOption& OptionRow = OptionPair.Value;
		ApplyitemOptionsToSpec(TArray<FDiaActualItemOption>{ OptionRow }, Spec);
	}

	FActiveGameplayEffectHandle EffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*Spec);
}

