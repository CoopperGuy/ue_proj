// Fill out your copyright notice in the Description page of Project Settings.


#include "Skill/Active/DiaProjectileSkill.h"
#include "Skill/DiaProjectile.h"
#include "Character/DiaCharacter.h"

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
		return;
	}

	// 발사 위치 계산
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

	// 발사체 생성 파라미터
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = SkillOwner;
	SpawnParams.Instigator = Cast<APawn>(SkillOwner);
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// 발사체 생성
	ADiaProjectile* Projectile = GetWorld()->SpawnActor<ADiaProjectile>(
		ProjectileClass, Location, Rotation, SpawnParams);

	if (IsValid(Projectile) == true)
	{
		Projectile->Initialize(GetDamage(), SkillOwner);
	}
}

void ADiaProjectileSkill::InitializeSkill(AActor* InOwner)
{
	Super::InitializeSkill(InOwner);
}
