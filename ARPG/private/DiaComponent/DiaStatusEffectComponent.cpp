// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaComponent/DiaStatusEffectComponent.h"

#include "DiaBaseCharacter.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "GAS/DiaAttributeSet.h"
#include "GAS/DiaGameplayTags.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Logging/ARPGLogChannels.h"
#include "TimerManager.h"

UDiaStatusEffectComponent::UDiaStatusEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDiaStatusEffectComponent::InitializeWithAbilitySystem(UAbilitySystemComponent* InAbilitySystemComponent, ADiaBaseCharacter* InOwnerCharacter)
{
	if (!IsValid(InAbilitySystemComponent) || !IsValid(InOwnerCharacter))
	{
		return;
	}

	UnregisterStateTags();

	AbilitySystemComponent = InAbilitySystemComponent;
	OwnerCharacter = InOwnerCharacter;

	const FDiaGameplayTags& Tags = FDiaGameplayTags::Get();
	RegisterStateTag(Tags.State_Stunned, EGameplayTagEventType::NewOrRemoved);
	RegisterStateTag(Tags.State_Slowed, EGameplayTagEventType::AnyCountChange);
	RegisterStateTag(Tags.State_Freeze, EGameplayTagEventType::AnyCountChange);
	RegisterStateTag(Tags.State_KnockBack, EGameplayTagEventType::NewOrRemoved);
}

void UDiaStatusEffectComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterStateTags();
	Super::EndPlay(EndPlayReason);
}

void UDiaStatusEffectComponent::RegisterStateTag(const FGameplayTag& StateTag, EGameplayTagEventType::Type EventType)
{
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (!IsValid(ASC) || !StateTag.IsValid())
	{
		return;
	}

	FRegisteredStateTag RegisteredTag;
	RegisteredTag.Tag = StateTag;
	RegisteredTag.EventType = EventType;
	RegisteredTag.DelegateHandle = ASC->RegisterGameplayTagEvent(StateTag, EventType)
		.AddUObject(this, &ThisClass::OnStateTagChanged);

	RegisteredStateTags.Add(RegisteredTag);
}

void UDiaStatusEffectComponent::UnregisterStateTags()
{
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (IsValid(ASC))
	{
		for (const FRegisteredStateTag& RegisteredTag : RegisteredStateTags)
		{
			ASC->RegisterGameplayTagEvent(RegisteredTag.Tag, RegisteredTag.EventType).Remove(RegisteredTag.DelegateHandle);
		}
	}

	RegisteredStateTags.Reset();
}

void UDiaStatusEffectComponent::OnStateTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	const FDiaGameplayTags& Tags = FDiaGameplayTags::Get();

	if (CallbackTag == Tags.State_Stunned)
	{
		HandleStunChanged(NewCount);
		return;
	}

	if (CallbackTag == Tags.State_Slowed)
	{
		RefreshMovementSpeed();
	}

	if(CallbackTag == Tags.State_Freeze)
	{
		RefreshMovementSpeed();
	}

	if(CallbackTag == Tags.State_KnockBack)
	{
		HandleKnockBackChanged(NewCount);
		return;
	}
}

void UDiaStatusEffectComponent::HandleStunChanged(int32 NewCount) const
{
	if (ADiaBaseCharacter* Character = OwnerCharacter.Get())
	{
		Character->ApplyStunState(NewCount > 0);
	}
}

void UDiaStatusEffectComponent::RefreshMovementSpeed() const
{
	ADiaBaseCharacter* Character = OwnerCharacter.Get();
	if (!IsValid(Character) || !IsValid(Character->GetAttributeSet()) || !IsValid(Character->GetCharacterMovement()))
	{
		return;
	}

	Character->GetCharacterMovement()->MaxWalkSpeed = FMath::Max(Character->GetAttributeSet()->GetMovementSpeed(), 0.f);
}

void UDiaStatusEffectComponent::HandleFreezeChanged(int32 NewCount) const
{
	ADiaBaseCharacter* Character = OwnerCharacter.Get();
	if (!IsValid(Character) || !IsValid(Character->GetAttributeSet()) || !IsValid(Character->GetCharacterMovement()))
	{
		return;
	}
	if (NewCount > 3)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = 0.f;
	}
	else if (NewCount > 0)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = Character->GetAttributeSet()->GetMovementSpeed() * 0.5f;
	}
	else
	{
		RefreshMovementSpeed();
	}
}

void UDiaStatusEffectComponent::HandleKnockBackChanged(int32 NewCount)
{
	ADiaBaseCharacter* Character = OwnerCharacter.Get();
	if (!IsValid(Character))
	{
		ARPG_LOG(LogARPG_Combat, Warning, TEXT("HandleKnockBackChanged skipped: OwnerCharacter is invalid. NewCount=%d, ASC=%s"),
			NewCount,
			*GetNameSafe(AbilitySystemComponent.Get()));
		return;
	}

	UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement();
	if (!IsValid(MovementComponent))
	{
		ARPG_LOG(LogARPG_Combat, Warning, TEXT("HandleKnockBackChanged skipped: MovementComponent is invalid. Character=%s, NewCount=%d"),
			*GetNameSafe(Character),
			NewCount);
		return;
	}

	const FVector ForwardVector = Character->GetActorForwardVector();
	const FVector Impulse = ForwardVector * -1000.f;
	const FVector VelocityBefore = MovementComponent->Velocity;

	ARPG_LOG(LogARPG_Combat, Warning, TEXT("HandleKnockBackChanged entered. Character=%s, NewCount=%d, Location=%s, Forward=%s, VelocityBefore=%s, MovementMode=%d"),
		*GetNameSafe(Character),
		NewCount,
		*Character->GetActorLocation().ToString(),
		*ForwardVector.ToString(),
		*VelocityBefore.ToString(),
		static_cast<int32>(MovementComponent->MovementMode));

	if (NewCount > 0)
	{
		if (AAIController* AIController = Cast<AAIController>(Character->GetController()))
		{
			AIController->StopMovement();
			if (AIController->BrainComponent)
			{
				AIController->BrainComponent->StopLogic(TEXT("Knockback"));
			}
			ARPG_LOG(LogARPG_Combat, Warning, TEXT("HandleKnockBackChanged stopped AI movement. Character=%s, Controller=%s"),
				*GetNameSafe(Character),
				*GetNameSafe(AIController));
		}

		if (UWorld* World = Character->GetWorld())
		{
			World->GetTimerManager().ClearTimer(KnockBackAIRestartTimerHandle);
		}

		if (MovementComponent->MovementMode == MOVE_None)
		{
			MovementComponent->SetMovementMode(MOVE_Walking);
			ARPG_LOG(LogARPG_Combat, Warning, TEXT("HandleKnockBackChanged restored movement mode before knockback. Character=%s, NewMovementMode=%d"),
				*GetNameSafe(Character),
				static_cast<int32>(MovementComponent->MovementMode));
		}

		MovementComponent->StopMovementImmediately();
		MovementComponent->AddImpulse(Impulse, true);
		MovementComponent->Velocity = Impulse;
		MovementComponent->UpdateComponentVelocity();
		Character->LaunchCharacter(Impulse, true, false);
	}
	else
	{
		if (UWorld* World = Character->GetWorld())
		{
			TWeakObjectPtr<ADiaBaseCharacter> CharacterPtr = Character;
			World->GetTimerManager().ClearTimer(KnockBackAIRestartTimerHandle);
			World->GetTimerManager().SetTimer(
				KnockBackAIRestartTimerHandle,
				FTimerDelegate::CreateLambda([CharacterPtr]()
				{
					ADiaBaseCharacter* RestartCharacter = CharacterPtr.Get();
					if (!IsValid(RestartCharacter))
					{
						return;
					}

					AAIController* AIController = Cast<AAIController>(RestartCharacter->GetController());
					if (!IsValid(AIController))
					{
						return;
					}

					if (AIController->BrainComponent)
					{
						AIController->BrainComponent->RestartLogic();
					}

					ARPG_LOG(LogARPG_Combat, Warning, TEXT("HandleKnockBackChanged restarted AI after knockback. Character=%s, Controller=%s, Velocity=%s"),
						*GetNameSafe(RestartCharacter),
						*GetNameSafe(AIController),
						*RestartCharacter->GetCharacterMovement()->Velocity.ToString());
				}),
				KnockBackAIRestartDelay,
				false);
		}
	}
}

