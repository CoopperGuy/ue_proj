// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "DiaStatusEffectComponent.generated.h"

class ADiaBaseCharacter;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPG_API UDiaStatusEffectComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UDiaStatusEffectComponent();

	void InitializeWithAbilitySystem(UAbilitySystemComponent* InAbilitySystemComponent, ADiaBaseCharacter* InOwnerCharacter);

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	struct FRegisteredStateTag
	{
		FGameplayTag Tag;
		EGameplayTagEventType::Type EventType = EGameplayTagEventType::NewOrRemoved;
		FDelegateHandle DelegateHandle;
	};

	void RegisterStateTag(const FGameplayTag& StateTag, EGameplayTagEventType::Type EventType);
	void UnregisterStateTags();
	void OnStateTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

	void HandleStunChanged(int32 NewCount) const;
	void RefreshMovementSpeed() const;
	void HandleFreezeChanged(int32 NewCount) const;
	void HandleKnockBackChanged(int32 NewCount);

	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	TWeakObjectPtr<ADiaBaseCharacter> OwnerCharacter;
	TArray<FRegisteredStateTag> RegisteredStateTags;
	FTimerHandle KnockBackAIRestartTimerHandle;
	float KnockBackAIRestartDelay = 0.25f;
};
