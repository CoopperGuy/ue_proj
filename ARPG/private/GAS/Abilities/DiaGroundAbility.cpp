// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/DiaGroundAbility.h"
#include "DiaBaseCharacter.h"   
#include "Skill/DiaGroundObj.h"
#include "GAS/DiaGASHelper.h"
#include "GAS/DiaGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_SpawnActor.h"
#include "DiaComponent/DiaSkillManagerComponent.h"
#include "Skill/DiaSkillActor.h"
#include "DiaComponent/Skill/DiaSkillVariant.h"
void UDiaGroundAbility::InitializeWithSkillData(const FGASSkillData& InSkillData)
{
    Super::InitializeWithSkillData(InSkillData);

    const FGASGroundData* G = InSkillData.GetExtraPtr<FGASGroundData>();
    if (!G)
    {
        return;
    }

    if (G->SkillActorClass)
    {
        SkillGroundClass = G->SkillActorClass;
    }
    bUseOwnerRotation = G->bUseOwnerRotation;
}

void UDiaGroundAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    if(SkillData.CastTime == 0.f)
    {
        SpawnSkillGround();
    }
}

void UDiaGroundAbility::SpawnSkillGround()
{
    if (!SkillGroundClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("DiaGroundAbility: SkillGroundClass is not set."));
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
        return;
    }
    const FGameplayAbilityActorInfo& ActorInfo = GetActorInfo();
    ADiaBaseCharacter* Character = Cast<ADiaBaseCharacter>(ActorInfo.AvatarActor.Get());
    if (!Character)
    {
        UE_LOG(LogTemp, Warning, TEXT("DiaGroundAbility: Invalid AvatarActor."));
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
        return;
    }

    //태그 가져와서 몬스터일경우, 플레이어일 경우 구분하자.

	bool bIsPlayer = Character->ActorHasTag(FDiaGameplayTags::Get().Actor_Player.GetTagName());
     
	//HACK: 플레이어가 아니면 캐릭터 방향으로 발사하도록 강제
    if (!bIsPlayer)
    {
        bUseOwnerRotation = true;
    }

    FVector SpawnLocation = bIsPlayer ? UDiaGASHelper::GetMouseWorldLocation(ActorInfo) : Character->GetActorLocation();
    FRotator SpawnRotation = bUseOwnerRotation ? Character->GetActorRotation() : FRotator::ZeroRotator;
	FTransform SpawnTransform(SpawnRotation, SpawnLocation);


    FGameplayAbilityTargetDataHandle TargetDataHandle;

    FGameplayAbilityTargetData_LocationInfo LocationData;
    LocationData.SourceLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
    LocationData.SourceLocation.LiteralTransform = FTransform(SpawnLocation);
    LocationData.TargetLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
    LocationData.TargetLocation.LiteralTransform = FTransform(SpawnLocation); 
    TargetDataHandle.Add(new FGameplayAbilityTargetData_LocationInfo(LocationData));

    FDiaSkillVariantContext VariantContext;
    VariantContext.SkillActorClass = SkillGroundClass;
    VariantContext.TargetData = TargetDataHandle;

    UDiaSkillManagerComponent* DiaSkillManagerComp = Character->FindComponentByClass<UDiaSkillManagerComponent>();
    DiaSkillManagerComp->SpawnSkillActorUseVariants(VariantContext, this);

    //SpawnActorTask = UAbilityTask_SpawnActor::SpawnActor
    //(this, TargetDataHandle, SkillGroundClass);

    //if (SpawnActorTask)
    //{
    //    SpawnActorTask->Success.AddDynamic(this, &UDiaGroundAbility::OnSpawned);

    //    AActor* SpawnedActor = nullptr;
    //    SpawnActorTask->BeginSpawningActor(this, TargetDataHandle, SkillGroundClass, SpawnedActor);
    //    if (SpawnedActor)
    //    {
    //        SpawnActorTask->FinishSpawningActor(this, TargetDataHandle, SpawnedActor);
    //    }

    //    // ReadyForActivation은 Begin/Finish 이후에 호출해야 합니다
    //    SpawnActorTask->ReadyForActivation();
    //}

    //ClassName 확인 디버그
    if (SkillGroundClass)
    {
		UE_LOG(LogTemp, Log, TEXT("DiaGroundAbility: SkillGroundClass is set to %s."), *SkillGroundClass->GetName());
    }

	//ADiaGroundObj* SkillGround = GetWorld()->SpawnActorDeferred<ADiaGroundObj>(
	//	SkillGroundClass,
	//	SpawnTransform,
	//	Character,
	//	Character,
	//	ESpawnActorCollisionHandlingMethod::AlwaysSpawn
	//);

	//if (SkillGround)
	//{
	//	const FGameplayAbilityActorInfo& Info = GetActorInfo();
	//	UAbilitySystemComponent* SourceASC = Info.AbilitySystemComponent.Get();
	//	SkillGround->Initialize(SkillData, Character, SourceASC, DamageEffectClass);
	//	SkillGround->FinishSpawning(SpawnTransform);
	//	UE_LOG(LogTemp, Log, TEXT("DiaGroundAbility: Spawned SkillGround at location %s."), *SpawnTransform.GetLocation().ToString());
	//}
	//else
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("DiaGroundAbility: Failed to spawn SkillGround."));
	//}

    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UDiaGroundAbility::ProcessSkillDelayEvents()
{
    Super::ProcessSkillDelayEvents();

    SpawnSkillGround();
}

bool UDiaGroundAbility::ShouldEndAbilityOnMontageCompleted() const
{
	return SkillData.CastTime <= 0.f;
}

void UDiaGroundAbility::OnSpawned(AActor* SpawnedSkillGround)
{
	UE_LOG(LogTemp, Log, TEXT("DiaGroundAbility::OnSpawned - Skill ground spawned successfully: %s"), *SpawnedSkillGround->GetName());
    ADiaGroundObj* SkillGround = Cast<ADiaGroundObj>(SpawnedSkillGround);
    if (SkillGround)
    {
        const FGameplayAbilityActorInfo& ActorInfo = GetActorInfo();
        ADiaBaseCharacter* Character = Cast<ADiaBaseCharacter>(ActorInfo.AvatarActor.Get());
        if (Character)
        {
            TArray<FGameplayEffectSpecHandle> TargetEffectSpecs;
            MakeEffectSpecContextToTarget(TargetEffectSpecs);
            SkillGround->InitTargetEffectHandle(TargetEffectSpecs);

            const FGameplayAbilityActorInfo& Info = GetActorInfo();
            UAbilitySystemComponent* SourceASC = Info.AbilitySystemComponent.Get();
            SkillGround->Initialize(SkillData, Character, SourceASC, nullptr);
        }
    }

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
