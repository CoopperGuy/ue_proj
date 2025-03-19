// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaComponent/DiaStatusEffectComponent.h"
#include "Skill/Effect/DiaStatusEffect.h"
#include "DiaBaseCharacter.h"


// Sets default values for this component's properties
UDiaStatusEffectComponent::UDiaStatusEffectComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UDiaStatusEffectComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UDiaStatusEffectComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 활성화된 모든 상태 효과 틱 처리
	for (int32 i = ActiveStatusEffects.Num() - 1; i >= 0; i--)
	{
		UDiaStatusEffect* StatusEffect = ActiveStatusEffects[i];
		
		if (IsValid(StatusEffect) && StatusEffect->IsActive())
		{
			StatusEffect->Tick(DeltaTime);
		}
		else
		{
			// 유효하지 않거나 비활성화된 효과 제거
			ActiveStatusEffects.RemoveAt(i);
		}
	}
}

UDiaStatusEffect* UDiaStatusEffectComponent::AddStatusEffect(TSubclassOf<UDiaStatusEffect> StatusEffectClass, float Duration, float Strength, AActor* Instigator)
{
	if (!StatusEffectClass)
	{
		return nullptr;
	}
	
	// 소유자 가져오기
	ADiaBaseCharacter* OwnerCharacter = Cast<ADiaBaseCharacter>(GetOwner());
	if (!IsValid(OwnerCharacter))
	{
		return nullptr;
	}
	
	// 상태 효과 객체 생성
	UDiaStatusEffect* NewStatusEffect = NewObject<UDiaStatusEffect>(this, StatusEffectClass);
	if (!IsValid(NewStatusEffect))
	{
		return nullptr;
	}
	
	// 상태 효과 초기화
	NewStatusEffect->Initialize(OwnerCharacter, Duration, Strength);
	
	// 이미 같은 ID의 효과가 있는지 확인
	UDiaStatusEffect* ExistingEffect = GetStatusEffectByID(NewStatusEffect->GetStatusEffectID());
	if (ExistingEffect)
	{
		// 중첩 가능한 경우 기존 효과의 스택 증가
		if (ExistingEffect->CanStack())
		{
			ExistingEffect->Apply();
			return ExistingEffect;
		}
		else
		{
			// 중첩 불가능한 경우 기존 효과 갱신
			RemoveStatusEffect(ExistingEffect);
		}
	}
	
	// 새 효과 추가 및 적용
	ActiveStatusEffects.Add(NewStatusEffect);
	NewStatusEffect->Apply();
	
	// 이벤트 발생
	OnStatusEffectAdded.Broadcast(NewStatusEffect);
	
	return NewStatusEffect;
}

UDiaStatusEffect* UDiaStatusEffectComponent::AddStatusEffectByID(const FString& StatusEffectID, float Duration, float Strength, AActor* Instigator)
{
	TSubclassOf<UDiaStatusEffect> StatusEffectClass = StatusEffectClassMap.FindRef(StatusEffectID);
	if (!StatusEffectClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Status effect class not found for ID: %s"), *StatusEffectID);
		return nullptr;
	}
	
	return AddStatusEffect(StatusEffectClass, Duration, Strength, Instigator);
}

bool UDiaStatusEffectComponent::RemoveStatusEffect(UDiaStatusEffect* StatusEffect)
{
	if (!IsValid(StatusEffect))
	{
		return false;
	}
	
	int32 Index = ActiveStatusEffects.Find(StatusEffect);
	if (Index == INDEX_NONE)
	{
		return false;
	}
	
	// 효과 제거
	StatusEffect->Remove();
	
	// 목록에서 제거
	ActiveStatusEffects.RemoveAt(Index);
	
	// 이벤트 발생
	OnStatusEffectRemoved.Broadcast(StatusEffect);
	
	return true;
}

bool UDiaStatusEffectComponent::RemoveStatusEffectByID(const FString& StatusEffectID)
{
	UDiaStatusEffect* StatusEffect = GetStatusEffectByID(StatusEffectID);
	if (!StatusEffect)
	{
		return false;
	}
	
	return RemoveStatusEffect(StatusEffect);
}

int32 UDiaStatusEffectComponent::RemoveStatusEffectsByTag(const FName& Tag)
{
	TArray<UDiaStatusEffect*> EffectsWithTag = GetStatusEffectsByTag(Tag);
	int32 RemovedCount = 0;
	
	for (UDiaStatusEffect* StatusEffect : EffectsWithTag)
	{
		if (RemoveStatusEffect(StatusEffect))
		{
			RemovedCount++;
		}
	}
	
	return RemovedCount;
}

void UDiaStatusEffectComponent::RemoveAllStatusEffects()
{
	for (int32 i = ActiveStatusEffects.Num() - 1; i >= 0; i--)
	{
		UDiaStatusEffect* StatusEffect = ActiveStatusEffects[i];
		if (IsValid(StatusEffect))
		{
			StatusEffect->Remove();
			OnStatusEffectRemoved.Broadcast(StatusEffect);
		}
	}
	
	ActiveStatusEffects.Empty();
}

bool UDiaStatusEffectComponent::HasStatusEffect(TSubclassOf<UDiaStatusEffect> StatusEffectClass) const
{
	for (UDiaStatusEffect* StatusEffect : ActiveStatusEffects)
	{
		if (IsValid(StatusEffect) && StatusEffect->IsA(StatusEffectClass) && StatusEffect->IsActive())
		{
			return true;
		}
	}
	
	return false;
}

bool UDiaStatusEffectComponent::HasStatusEffectByID(const FString& StatusEffectID) const
{
	return GetStatusEffectByID(StatusEffectID) != nullptr;
}

bool UDiaStatusEffectComponent::HasStatusEffectWithTag(const FName& Tag) const
{
	for (UDiaStatusEffect* StatusEffect : ActiveStatusEffects)
	{
		if (IsValid(StatusEffect) && StatusEffect->HasTag(Tag) && StatusEffect->IsActive())
		{
			return true;
		}
	}
	
	return false;
}

UDiaStatusEffect* UDiaStatusEffectComponent::GetStatusEffectByID(const FString& StatusEffectID) const
{
	for (UDiaStatusEffect* StatusEffect : ActiveStatusEffects)
	{
		if (IsValid(StatusEffect) && StatusEffect->GetStatusEffectID() == StatusEffectID && StatusEffect->IsActive())
		{
			return StatusEffect;
		}
	}
	
	return nullptr;
}

TArray<UDiaStatusEffect*> UDiaStatusEffectComponent::GetAllStatusEffects() const
{
	return ActiveStatusEffects;
}

TArray<UDiaStatusEffect*> UDiaStatusEffectComponent::GetStatusEffectsByTag(const FName& Tag) const
{
	TArray<UDiaStatusEffect*> Result;
	
	for (UDiaStatusEffect* StatusEffect : ActiveStatusEffects)
	{
		if (IsValid(StatusEffect) && StatusEffect->HasTag(Tag) && StatusEffect->IsActive())
		{
			Result.Add(StatusEffect);
		}
	}
	
	return Result;
}

