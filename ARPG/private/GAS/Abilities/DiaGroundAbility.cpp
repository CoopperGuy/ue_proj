// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/DiaGroundAbility.h"
#include "DiaBaseCharacter.h"   
#include "Skill/DiaGroundObj.h"
#include "GAS/DiaGASHelper.h"
#include "Abilities/Tasks/AbilityTask_SpawnActor.h"
void UDiaGroundAbility::InitializeWithSkillData(const FGASSkillData& InSkillData)
{
    Super::InitializeWithSkillData(InSkillData);
    if (InSkillData.SkillObjectClass)
    {
        SkillGroundClass = InSkillData.SkillObjectClass;
    }
    //// Multi-shot 같은 값은 데이터 구조에 따라 매핑 (예시로 Count/Spread를 Range/Radius로 매핑)
    //if (InSkillData.SkillType == EGASSkillType::RangedAttack || InSkillData.SkillType == EGASSkillType::Magic)
    //{
    //    // 선택적 매핑 규칙: Range -> ProjectileCount(정수 변환), Radius -> SpreadAngle
    //    ProjectileCount = FMath::Clamp(static_cast<int32>(InSkillData.Range / 200.0f), 1, 7);
    //    bFireMultipleProjectiles = ProjectileCount > 1;
    //    SpreadAngle = FMath::Clamp(InSkillData.Radius * 0.2f, 5.0f, 45.0f);
    //}

}

void UDiaGroundAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	SpawnSkillGround();
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

     
    FVector SpawnLocation = UDiaGASHelper::GetMouseWorldLocation(ActorInfo);
    FRotator SpawnRotation = bUseOwnerRotation ? Character->GetActorRotation() : FRotator::ZeroRotator;
	FTransform SpawnTransform(SpawnRotation, SpawnLocation);


    SpawnActorTask = UAbilityTask_SpawnActor::SpawnActor
    (this, FGameplayAbilityTargetDataHandle(), ADiaGroundObj::StaticClass());

    if (SpawnActorTask)
    {
        SpawnActorTask->Success.AddDynamic(this, &UDiaGroundAbility::OnSpawned);
        SpawnActorTask->ReadyForActivation();
    }


  //  ADiaGroundObj* SkillGround = GetWorld()->SpawnActorDeferred<ADiaGroundObj>(
  //      SkillGroundClass,
  //      SpawnTransform,
  //      Character,
  //      Character,
  //      ESpawnActorCollisionHandlingMethod::AlwaysSpawn
  //  );

  //  if (SkillGround)
  //  {
  //      const FGameplayAbilityActorInfo& Info = GetActorInfo();
  //      UAbilitySystemComponent* SourceASC = Info.AbilitySystemComponent.Get();
  //      SkillGround->Initialize(SkillData, Character, SourceASC, DamageEffectClass);
		//SkillGround->FinishSpawning(SpawnTransform);
		//UE_LOG(LogTemp, Log, TEXT("DiaGroundAbility: Spawned SkillGround at location %s."), *SpawnLocation.ToString());
  //  }
  //  else
  //  {
  //      UE_LOG(LogTemp, Warning, TEXT("DiaGroundAbility: Failed to spawn SkillGround."));
  //  }

}

void UDiaGroundAbility::OnSpawned(AActor* SpawnedSkillGround)
{
    ADiaGroundObj* SkillGround = Cast<ADiaGroundObj>(SpawnedSkillGround);
    if (SkillGround)
    {
        const FGameplayAbilityActorInfo& ActorInfo = GetActorInfo();
        ADiaBaseCharacter* Character = Cast<ADiaBaseCharacter>(ActorInfo.AvatarActor.Get());
        if (Character)
        {
            FVector SpawnLocation = UDiaGASHelper::GetMouseWorldLocation(ActorInfo);
            FRotator SpawnRotation = bUseOwnerRotation ? Character->GetActorRotation() : FRotator::ZeroRotator;
            // GAS 초기화: 소스 ASC, 대미지 GE, 기본 대미지 전달
            const FGameplayAbilityActorInfo& Info = GetActorInfo();
            UAbilitySystemComponent* SourceASC = Info.AbilitySystemComponent.Get();
            SkillGround->Initialize(SkillData, Character, SourceASC, DamageEffectClass);
            SkillGround->SetActorLocationAndRotation(SpawnLocation, SpawnRotation);
            SpawnActorTask->FinishSpawningActor(this, FGameplayAbilityTargetDataHandle(), SkillGround);
        }
    }

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}