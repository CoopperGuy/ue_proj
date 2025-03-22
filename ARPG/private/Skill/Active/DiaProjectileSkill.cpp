// Fill out your copyright notice in the Description page of Project Settings.


#include "Skill/Active/DiaProjectileSkill.h"
#include "Skill/DiaProjectile.h"
#include "Character/DiaCharacter.h"
#include "Kismet/GameplayStatics.h"

ADiaProjectileSkill::ADiaProjectileSkill()
{
	ProjectileOffset = FVector(0.0f, 0.0f, 0.0f);
}

void ADiaProjectileSkill::ExecuteSkill()
{
	Super::ExecuteSkill();
}

void ADiaProjectileSkill::EndSkill()
{
	Super::EndSkill();
}

void ADiaProjectileSkill::OnSkillStart()
{
	Super::OnSkillStart();
	
	if (IsValid(ProjectileClass))
	{
		SpawnProjectile();
	}
	
	//Start Skill CoolDown Check
	EndSkill();
}

void ADiaProjectileSkill::OnSkillEnd()
{
	Super::OnSkillEnd();
}

void ADiaProjectileSkill::SpawnProjectile()
{
	if (!IsValid(SkillOwner) || !IsValid(ProjectileClass))
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnProjectile Failed: Invalid SkillOwner or ProjectileClass"));
		return;
	}

	// 발사 위치 및 방향 계산
	FVector Location = SkillOwner->GetActorLocation() + 
					  SkillOwner->GetActorRotation().RotateVector(ProjectileOffset);
	
	// 발사 방향 계산
	FRotator Rotation;
	
	if (bUseOwnerRotation)
	{
		// 캐릭터 방향으로 발사
		Rotation = SkillOwner->GetActorRotation();
	}
	else
	{
		// 마우스 커서 방향으로 발사 (플레이어 캐릭터인 경우)
		ADiaCharacter* DiaChar = Cast<ADiaCharacter>(SkillOwner);
		if (IsValid(DiaChar))
		{
			FVector TargetLocation;
			if (DiaChar->GetMouseWorldLocation(TargetLocation))
			{
				FVector Direction = (TargetLocation - Location).GetSafeNormal();
				Rotation = Direction.Rotation();
			}
			else
			{
				Rotation = SkillOwner->GetActorRotation();
			}
		}
		else
		{
			Rotation = SkillOwner->GetActorRotation();
		}
	}

	// 블루프린트 방식으로 우회하여 스폰
	UClass* ProjectileClassObj = ProjectileClass.Get();
	if (ProjectileClassObj)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = SkillOwner;
		SpawnParams.Instigator = Cast<APawn>(SkillOwner);
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// GameplayStatics를 사용하여 스폰
		AActor* NewActor = UGameplayStatics::BeginDeferredActorSpawnFromClass(
			this, ProjectileClassObj, FTransform(Rotation, Location), ESpawnActorCollisionHandlingMethod::AlwaysSpawn, SkillOwner);
		
		if (NewActor)
		{
			ADiaProjectile* Projectile = Cast<ADiaProjectile>(NewActor);
			if (Projectile)
			{
				UGameplayStatics::FinishSpawningActor(NewActor, FTransform(Rotation, Location));
				Projectile->Initialize(GetDamage(), SkillOwner);
				UE_LOG(LogTemp, Log, TEXT("Projectile spawned and initialized via GameplayStatics"));
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Failed to cast spawned actor to ADiaProjectile"));
				NewActor->Destroy();
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to begin deferred actor spawn"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to get UClass from TSubclassOf"));
	}
}

void ADiaProjectileSkill::InitializeSkill(AActor* InOwner)
{
	Super::InitializeSkill(InOwner);
}
