// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaComponent/DiaOptionManagerComponent.h"
#include "Controller/DiaController.h"
#include "DiaBaseCharacter.h"
#include "AbilitySystemComponent.h"
#include "Engine/World.h"

#include "GAS/DiaAttributeSet.h"

#include "GAS/Effects/DiaGE_StatApply.h"
#include "GAS/DiaGameplayTags.h"
#include "GAS/Effects/DiaGE_OptionGeneric.h"
#include "Logging/ARPGLogChannels.h"


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
    ActiveOptions.Add(NewOption);
}

void UDiaOptionManagerComponent::RemoveOption(const FDiaActualItemOption& Option)
{
    const int32 Index = ActiveOptions.IndexOfByPredicate([&](const FDiaActualItemOption& Existing)
    {
        return Existing.OptionID == Option.OptionID && Existing.TierIndex == Option.TierIndex
            && Existing.GrantedTag == Option.GrantedTag && Existing.Value == Option.Value;
    });
    if (Index != INDEX_NONE) ActiveOptions.RemoveAt(Index);
}

void UDiaOptionManagerComponent::RemoveOption(const FName& OptionID)
{
    const int32 Index = ActiveOptions.IndexOfByPredicate([&](const FDiaActualItemOption& Option)
    {
        return Option.OptionID == OptionID;
    });
    if (Index != INDEX_NONE) ActiveOptions.RemoveAt(Index);
}

FDiaActualItemOption* UDiaOptionManagerComponent::GetOptionByID(const FName& OptionID)
{
    return ActiveOptions.FindByPredicate([&](const FDiaActualItemOption& Option) { return Option.OptionID == OptionID; });
}

FDiaActualItemOption* UDiaOptionManagerComponent::GetOptionByOptionRow(const FDiaActualItemOption& OptionRow)
{
    return GetOptionByID(OptionRow.OptionID);
}

void UDiaOptionManagerComponent::ApplyEquipmentStats(const FEquippedItem& Item, EEquipmentSlot SlotType)
{
	//gameplayeffect를 이용해 스탯 적용
	ADiaController* Controller = Cast<ADiaController>(GetOwner());
	if (!IsValid(Controller))
	{
		UE_LOG(LogARPG, Warning, TEXT("ApplyEquipmentStats: Controller가 유효하지 않습니다."));
		return;
	}

	ADiaBaseCharacter* OwnerCharacter = Controller->GetPawn<ADiaBaseCharacter>();
	if (!IsValid(OwnerCharacter))
	{
		UE_LOG(LogARPG, Warning, TEXT("ApplyEquipmentStats: OwnerCharacter가 유효하지 않습니다."));
		return;
	}

	UAbilitySystemComponent* ASC = OwnerCharacter->GetAbilitySystemComponent();
	if (!IsValid(ASC))
	{
		UE_LOG(LogARPG, Warning, TEXT("ApplyEquipmentStats: AbilitySystemComponent가 유효하지 않습니다."));
		return;
	}

    if (const FActiveGameplayEffectHandle* Existing = AppliedStatHandles.Find(Item.ItemInstance.InstanceID))
    {
        ASC->RemoveActiveGameplayEffect(*Existing);
        AppliedStatHandles.Remove(Item.ItemInstance.InstanceID);
    }

	FGameplayEffectSpecHandle SpecHandle
		= ASC->MakeOutgoingSpec(UDiaGE_StatApply::StaticClass(), 1.0f, ASC->MakeEffectContext());
	if (!SpecHandle.IsValid())
	{
		UE_LOG(LogARPG, Error, TEXT("ApplyEquipmentStats: SpecHandle 생성 실패"));
		return;
	}

	FGameplayEffectSpec* Spec = SpecHandle.Data.Get();
	check(Spec)

	int32 StatCount = Item.ItemInstance.ModifiedStats.Num();
	if (StatCount == 0)
	{
		UE_LOG(LogARPG, Warning, TEXT("ApplyEquipmentStats: ModifiedStats가 비어있습니다."));
		return;
	}
	
	TArray<FGameplayTag> StatTags = FDiaGameplayTags::GetAttributeStats();
	for (const FGameplayTag& StatTag : StatTags)
	{
		if (const auto StatInfo = Item.ItemInstance.ModifiedStats.Find(StatTag))
		{
			Spec->SetSetByCallerMagnitude(StatTag, StatInfo->Values);  // 값 설정
			UE_LOG(LogARPG, Warning, TEXT("스탯 적용: %s = %f"), *StatTag.ToString(), StatInfo->Values);
		}
		else
		{
			Spec->SetSetByCallerMagnitude(StatTag, 0.f);  // 값 설정
		}
	}

	FActiveGameplayEffectHandle EffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*Spec);
	AppliedStatHandles.Add(Item.ItemInstance.InstanceID, EffectHandle);
	UE_LOG(LogARPG, Warning, TEXT("스탯 적용 완료: ItemID = %s"), *Item.ItemInstance.InstanceID.ToString());
}

void UDiaOptionManagerComponent::RemoveEqipmentStats(const FEquippedItem& Item, EEquipmentSlot SlotType)
{
	const FGuid& ItemID = Item.ItemInstance.InstanceID;

	//gameplayeffect를 이용해 스탯 제거
	ADiaController* Controller = Cast<ADiaController>(GetOwner());
	if (!IsValid(Controller))
	{
		UE_LOG(LogARPG, Warning, TEXT("ApplyEquipmentStats: Controller가 유효하지 않습니다."));
		return;
	}

	ADiaBaseCharacter* OwnerCharacter = Controller->GetPawn<ADiaBaseCharacter>();
	if (!IsValid(OwnerCharacter))
	{
		UE_LOG(LogARPG, Warning, TEXT("ApplyEquipmentStats: OwnerCharacter가 유효하지 않습니다."));
		return;
	}

	UAbilitySystemComponent* ASC = OwnerCharacter->GetAbilitySystemComponent();
	if (!IsValid(ASC))
	{
		UE_LOG(LogARPG, Warning, TEXT("ApplyEquipmentStats: AbilitySystemComponent가 유효하지 않습니다."));
		return;
	}

	//스탯 제거
    if (FActiveGameplayEffectHandle* StatHandle = AppliedStatHandles.Find(ItemID))
    {
        ASC->RemoveActiveGameplayEffect(*StatHandle);
        AppliedStatHandles.Remove(ItemID);
		UE_LOG(LogARPG, Warning, TEXT("스탯 제거: ItemID = %s"), *ItemID.ToString());
    }

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
		UE_LOG(LogARPG, Warning, TEXT("ApplyEquipmentStats: Controller가 유효하지 않습니다."));
		return;
	}

	ADiaBaseCharacter* OwnerCharacter = Controller->GetPawn<ADiaBaseCharacter>();
	if (!IsValid(OwnerCharacter))
	{
		UE_LOG(LogARPG, Warning, TEXT("ApplyEquipmentStats: OwnerCharacter가 유효하지 않습니다."));
		return;
	}

	UAbilitySystemComponent* ASC = OwnerCharacter->GetAbilitySystemComponent();
	if (!IsValid(ASC))
	{
		UE_LOG(LogARPG, Warning, TEXT("ApplyEquipmentStats: AbilitySystemComponent가 유효하지 않습니다."));
		return;
	}

    if (const FActiveGameplayEffectHandle* Existing = AppliedPrefixHandles.Find(Item.ItemInstance.InstanceID))
        ASC->RemoveActiveGameplayEffect(*Existing);
    if (const FActiveGameplayEffectHandle* Existing = AppliedSuffixHandles.Find(Item.ItemInstance.InstanceID))
        ASC->RemoveActiveGameplayEffect(*Existing);
    AppliedPrefixHandles.Remove(Item.ItemInstance.InstanceID);
    AppliedSuffixHandles.Remove(Item.ItemInstance.InstanceID);

	FActiveGameplayEffectHandle PrefixHandle = MakeGameplayEffectOptions(ASC, Item.ItemInstance.PrefixOptions);
	FActiveGameplayEffectHandle SuffixHandle = MakeGameplayEffectOptions(ASC, Item.ItemInstance.SuffixOptions);
	if(PrefixHandle.IsValid())
	{
		AppliedPrefixHandles.Add(Item.ItemInstance.InstanceID, PrefixHandle);
	}
	if (SuffixHandle.IsValid())
	{
		AppliedSuffixHandles.Add(Item.ItemInstance.InstanceID, SuffixHandle);
	}
}

FActiveGameplayEffectHandle UDiaOptionManagerComponent::MakeGameplayEffectOptions(UAbilitySystemComponent* ASC, const TArray<FDiaActualItemOption>& ItemOptions)
{
	FGameplayEffectSpecHandle SpecHandle
		= ASC->MakeOutgoingSpec(UDiaGE_OptionGeneric::StaticClass(), 1.0f, ASC->MakeEffectContext());
	if (!SpecHandle.IsValid())
	{
		UE_LOG(LogARPG, Error, TEXT("ApplyEquipmentStats: SpecHandle 생성 실패"));
		return FActiveGameplayEffectHandle();
	}

	if (ItemOptions.Num() == 0)
	{
		UE_LOG(LogARPG, Warning, TEXT("MakeGameplayEffectOptions: ItemOptions가 비어있습니다."));
		return FActiveGameplayEffectHandle();
	}

	FGameplayEffectSpec* Spec = SpecHandle.Data.Get();
	check(Spec)
	//UE_LOG(LogARPG, Warning, TEXT("옵션 개수: %d"), ItemOptions.Num());
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
        // Old saves can still contain tags from the prototype option table.
        FGameplayTag GrantedTag = OptionRow.GrantedTag;
        if (GrantedTag.ToString() == TEXT("Item.Option.MultiplyAdditive.DamageIncreaseOption"))
            GrantedTag = FDiaGameplayTags::Get().ItemOptionStatDamage;
        if (GrantedTag.ToString() == TEXT("Item.Option.Additive.CriticalChance"))
            GrantedTag = FDiaGameplayTags::Get().ItemOptionStatCriticalChance;
		float OptionValue = OptionRow.Value;
		if (OptionRow.GrantedTag == FDiaGameplayTags::Get().ItemOptionStatSpeed
			&& OptionRow.ScalingType == EItemOptionScalingType::IOST_Percent)
		{
			OptionValue = 1.0f + OptionRow.Value * 0.01f;
		}

        if (GrantedTag == FDiaGameplayTags::Get().ItemOptionStatSpeed)
        {
            float& Combined = OptionValueMap.FindOrAdd(GrantedTag, 1.0f);
            Combined += OptionValue - 1.0f;
        }
        else
        {
            OptionValueMap.FindOrAdd(GrantedTag) += OptionValue;
        }
		//UE_LOG(LogARPG, Warning, TEXT("옵션 준비: %s = %f"), *OptionRow.GrantedTag.ToString(), OptionRow.Value);
	}

	//int32 i = 0;
	// 모든 옵션 태그에 대해 SetByCaller 설정
	for (const FGameplayTag& OptionTag : AllOptionTags)
	{
		if (const float* FoundValue = OptionValueMap.Find(OptionTag))
		{
			Spec->SetSetByCallerMagnitude(OptionTag, *FoundValue);
			//UE_LOG(LogARPG, Warning, TEXT("옵션 적용: %s = %f, 인덱스 : %d"), *OptionTag.ToString(), *FoundValue, i);
		}
		else
		{
			const float DefaultValue = OptionTag == FDiaGameplayTags::Get().ItemOptionStatSpeed ? 1.0f : 0.0f;
			Spec->SetSetByCallerMagnitude(OptionTag, DefaultValue);
			//UE_LOG(LogARPG, Warning, TEXT("옵션 적용 안됨(0으로 설정): %s, 인덱스 : %d"), *OptionTag.ToString(), i);
		}
		//++i;
	}	
}

float UDiaOptionManagerComponent::GetTotalOptionMagnitudeByTag(const FGameplayTag& OptionTag) const
{
	//현재는 단순히 해당 태그를 가진 옵션들의 값을 모두 더하는 형태로 구현
	float TotalMagnitude = 0.0f;
	for (const FDiaActualItemOption& Option : ActiveOptions)
	{
		if (Option.GrantedTag == OptionTag)
		{
			TotalMagnitude += Option.Value;
		}
	}

	return TotalMagnitude;
}

bool UDiaOptionManagerComponent::TryActivateEquipmentProc(UAbilitySystemComponent* ASC, const FGameplayTag& Tag, float Chance, float Cooldown)
{
    if (!IsValid(ASC) || !ASC->GetAvatarActor() || !ASC->GetAvatarActor()->HasAuthority() || !GetWorld()) return false;
    if (ProcOwner.Get() != ASC)
    {
        ProcOwner = ASC;
        ProcCooldowns.ReadyAt.Reset();
    }
    return ProcCooldowns.TryActivate(Tag, GetWorld()->GetTimeSeconds(), Chance, FMath::FRand(), Cooldown);
}
