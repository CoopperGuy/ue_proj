// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/CharacterStatus/StatusWidget.h"
#include "Components/TextBlock.h"
#include "DiaComponent/DiaStatComponent.h"

void UStatusWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UStatusWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	if (bUseTickBasedUpdate)
	{
		TickBasedUpdate(InDeltaTime);
	}
}

void UStatusWidget::SetTickBasedUpdate(bool bEnabled, float UpdateInterval)
{
	bUseTickBasedUpdate = bEnabled;
	TickUpdateInterval = UpdateInterval;
	TickUpdateTimer = 0.0f;
	
	UE_LOG(LogTemp, Log, TEXT("StatusWidget: Tick 기반 업데이트 %s (간격: %.2f초)"), 
		bEnabled ? TEXT("활성화") : TEXT("비활성화"), UpdateInterval);
}

void UStatusWidget::TickBasedUpdate(float DeltaTime)
{
	if (!BoundStatComponent.IsValid())
	{
		return;
	}

	TickUpdateTimer += DeltaTime;
	
	if (TickUpdateTimer >= TickUpdateInterval)
	{
		// 스탯 정보 가져와서 UI 업데이트
		const FCharacterData& CharData = BoundStatComponent->GetCharacterData();
		const FLevelData& LevelData = BoundStatComponent->GetLevelData();
		const FCombatStats& CombatData = BoundStatComponent->GetCombatStats();
		
		UpdateHealthMana(CharData.Health, CharData.MaxHealth, CharData.Mana, CharData.MaxMana);
		UpdateExperience(LevelData.CurrentExp, LevelData.MaxExp);
		
		// 전투 스탯도 업데이트 (변경될 수 있음)
		if (AtkText)
		{
			AtkText->SetText(FText::AsNumber(FMath::FloorToInt(CombatData.AttackPower)));
		}
		if (DefText)
		{
			DefText->SetText(FText::AsNumber(FMath::FloorToInt(CombatData.Defense)));
		}
		
		TickUpdateTimer = 0.0f;
	}
}

void UStatusWidget::BindToStatComponent(UDiaStatComponent* StatComponent)
{
	// 기존 바인딩 해제
	UnbindFromStatComponent();
	
	if (!IsValid(StatComponent))
	{
		UE_LOG(LogTemp, Warning, TEXT("StatusWidget: 유효하지 않은 StatComponent"));
		return;
	}
	
	// 새로운 스탯 컴포넌트 설정
	BoundStatComponent = StatComponent;
	
	bool isBoundedBefore = BoundStatComponent->GetOnExpChanged().IsBound();
	// 델리게이트 바인딩
	BoundStatComponent->GetOnHealthChanged().AddDynamic(this, &UStatusWidget::OnHealthChanged);
	BoundStatComponent->GetOnManaChanged().AddDynamic(this, &UStatusWidget::OnManaChanged);
	BoundStatComponent->GetOnExpChanged().AddDynamic(this, &UStatusWidget::OnExpChanged);
	BoundStatComponent->GetOnLevelUp().AddDynamic(this, &UStatusWidget::OnLevelUp);
	
	// 새로운 델리게이트 바인딩 - 기본 스탯 및 전투 스탯
	BoundStatComponent->GetOnBaseStatChanged().AddDynamic(this, &UStatusWidget::OnBaseStatChanged);
	BoundStatComponent->GetOnAttackPowerChanged().AddDynamic(this, &UStatusWidget::OnAttackPowerChanged);
	BoundStatComponent->GetOnDefenseChanged().AddDynamic(this, &UStatusWidget::OnDefenseChanged);
	bool isBoundedAfter = BoundStatComponent->GetOnExpChanged().IsBound();
	UE_LOG(LogTemp, Log, TEXT("Bound Statd %d -> %d"),isBoundedBefore, isBoundedAfter);

	// 초기 상태 업데이트
	const FCharacterData& CharData = BoundStatComponent->GetCharacterData();
	const FLevelData& LevelData = BoundStatComponent->GetLevelData();
	const FCombatStats& CombatData = BoundStatComponent->GetCombatStats();
	
	// 전체 스탯 업데이트 (실제 스탯 값 사용)
	UpdateStatus(
		TEXT("Player"), // 기본값, 필요시 캐릭터에서 가져오도록 수정 가능
		CharData.Health, CharData.MaxHealth,
		CharData.Mana, CharData.MaxMana,
		BoundStatComponent->GetStrength(), // 실제 스탯 값 사용
		BoundStatComponent->GetDexterity(),
		BoundStatComponent->GetIntelligence(),
		CombatData.AttackPower, CombatData.Defense,
		LevelData.CurrentExp, LevelData.MaxExp
	);
	
	UE_LOG(LogTemp, Log, TEXT("StatusWidget: StatComponent 바인딩 완료"));
}

void UStatusWidget::UnbindFromStatComponent()
{
	if (BoundStatComponent.IsValid())
	{
		// 모든 델리게이트 바인딩 해제
		BoundStatComponent->GetOnHealthChanged().RemoveDynamic(this, &UStatusWidget::OnHealthChanged);
		BoundStatComponent->GetOnManaChanged().RemoveDynamic(this, &UStatusWidget::OnManaChanged);
		BoundStatComponent->GetOnExpChanged().RemoveDynamic(this, &UStatusWidget::OnExpChanged);
		BoundStatComponent->GetOnLevelUp().RemoveDynamic(this, &UStatusWidget::OnLevelUp);
		
		// 새로운 델리게이트 바인딩 해제
		BoundStatComponent->GetOnBaseStatChanged().RemoveDynamic(this, &UStatusWidget::OnBaseStatChanged);
		BoundStatComponent->GetOnAttackPowerChanged().RemoveDynamic(this, &UStatusWidget::OnAttackPowerChanged);
		BoundStatComponent->GetOnDefenseChanged().RemoveDynamic(this, &UStatusWidget::OnDefenseChanged);
		
		BoundStatComponent.Reset();
		
		UE_LOG(LogTemp, Log, TEXT("StatusWidget: StatComponent 바인딩 해제 완료"));
	}
}

void UStatusWidget::OnHealthChanged(float NewHealth, float MaxHealth)
{
	if (HPText)
	{
		HPText->SetText(FText::Format(FText::FromString(TEXT("{0}/{1}")), 
			FText::AsNumber(FMath::FloorToInt(NewHealth)), 
			FText::AsNumber(FMath::FloorToInt(MaxHealth))));
	}
}

void UStatusWidget::OnManaChanged(float NewMana, float MaxMana)
{
	if (MPText)
	{
		MPText->SetText(FText::Format(FText::FromString(TEXT("{0}/{1}")), 
			FText::AsNumber(FMath::FloorToInt(NewMana)), 
			FText::AsNumber(FMath::FloorToInt(MaxMana))));
	}
}

void UStatusWidget::OnExpChanged(float NewExp, float MaxExp)
{
	if (ExpText)
	{
		ExpText->SetText(FText::Format(FText::FromString(TEXT("{0}/{1}")), 
			FText::AsNumber(FMath::FloorToInt(NewExp)), 
			FText::AsNumber(FMath::FloorToInt(MaxExp))));
	}
}

void UStatusWidget::OnLevelUp(int32 NewLevel)
{
	// 레벨업 시 전체 스탯 업데이트
	if (BoundStatComponent.IsValid())
	{
		const FCharacterData& CharData = BoundStatComponent->GetCharacterData();
		const FLevelData& LevelData = BoundStatComponent->GetLevelData();
		const FCombatStats& CombatData = BoundStatComponent->GetCombatStats();
		
		// 전투 스탯 업데이트
		if (AtkText)
		{
			AtkText->SetText(FText::AsNumber(FMath::FloorToInt(CombatData.AttackPower)));
		}
		if (DefText)
		{
			DefText->SetText(FText::AsNumber(FMath::FloorToInt(CombatData.Defense)));
		}
		
		UE_LOG(LogTemp, Log, TEXT("StatusWidget: 레벨업! 새 레벨: %d"), NewLevel);
	}
}

void UStatusWidget::UpdateStatus(const FString& UserName, float CurrentHP, float MaxHP, float CurrentMP, float MaxMP, float Strength, float Dexterity, float Intelligence, float AttackPower, float Defense, int32 CurrentExp, int32 MaxExp)
{
	if (UserNameText)
	{
		UserNameText->SetText(FText::FromString(UserName));
	}
	if (HPText)
	{
		HPText->SetText(FText::Format(FText::FromString(TEXT("{0}/{1}")), FText::AsNumber(CurrentHP), FText::AsNumber(MaxHP)));
	}
	if (MPText)
	{
		MPText->SetText(FText::Format(FText::FromString(TEXT("{0}/{1}")), FText::AsNumber(CurrentMP), FText::AsNumber(MaxMP)));
	}
	if (StrText)
	{
		StrText->SetText(FText::Format(FText::FromString(TEXT("{0}")), FText::AsNumber(Strength)));
	}
	if (DexText)
	{
		DexText->SetText(FText::Format(FText::FromString(TEXT("{0}")), FText::AsNumber(Dexterity)));
	}
	if (IntText)
	{
		IntText->SetText(FText::Format(FText::FromString(TEXT("{0}")), FText::AsNumber(Intelligence)));
	}
	if (AtkText)
	{
		AtkText->SetText(FText::Format(FText::FromString(TEXT("{0}")), FText::AsNumber(AttackPower)));
	}
	if (DefText)
	{
		DefText->SetText(FText::Format(FText::FromString(TEXT("{0}")), FText::AsNumber(Defense)));
	}
	if (ExpText)
	{
		ExpText->SetText(FText::Format(FText::FromString(TEXT("{0}/{1}")), FText::AsNumber(CurrentExp), FText::AsNumber(MaxExp)));
	}
}

void UStatusWidget::UpdateHealthMana(float CurrentHP, float MaxHP, float CurrentMP, float MaxMP)
{
	if (HPText)
	{
		HPText->SetText(FText::Format(FText::FromString(TEXT("{0}/{1}")), FText::AsNumber(CurrentHP), FText::AsNumber(MaxHP)));
	}
	if (MPText)
	{
		MPText->SetText(FText::Format(FText::FromString(TEXT("{0}/{1}")), FText::AsNumber(CurrentMP), FText::AsNumber(MaxMP)));
	}
}

void UStatusWidget::UpdateExperience(int32 CurrentExp, int32 MaxExp)
{
	if (ExpText)
	{
		ExpText->SetText(FText::Format(FText::FromString(TEXT("{0}/{1}")), FText::AsNumber(CurrentExp), FText::AsNumber(MaxExp)));
	}
}

//////////////////////////////////////////////////////////////////////////
// 새로운 스탯 델리게이트 콜백 함수들
//////////////////////////////////////////////////////////////////////////

void UStatusWidget::OnBaseStatChanged(EDefaultStat StatType, float NewValue, float OldValue)
{
	// 스탯 타입에 따라 해당 UI 요소 업데이트
	switch (StatType)
	{
	case EDefaultStat::eDS_Str:
		if (StrText)
		{
			StrText->SetText(FText::AsNumber(FMath::FloorToInt(NewValue)));
		}
		break;
		
	case EDefaultStat::eDS_Int:
		if (IntText)
		{
			IntText->SetText(FText::AsNumber(FMath::FloorToInt(NewValue)));
		}
		break;
		
	case EDefaultStat::eDS_Dex:
		if (DexText)
		{
			DexText->SetText(FText::AsNumber(FMath::FloorToInt(NewValue)));
		}
		break;
		
	case EDefaultStat::eDS_Con:
		// Constitution은 현재 UI에 없으므로 로그만 출력
		UE_LOG(LogTemp, Log, TEXT("StatusWidget: Constitution 변경 %.1f -> %.1f"), OldValue, NewValue);
		break;
		
	default:
		break;
	}

	UE_LOG(LogTemp, Log, TEXT("StatusWidget: %s 스탯 UI 업데이트 - %.1f -> %.1f"), 
		*UEnum::GetValueAsString(StatType), OldValue, NewValue);
}

void UStatusWidget::OnAttackPowerChanged(float NewAttackPower, float OldAttackPower)
{
	if (AtkText)
	{
		AtkText->SetText(FText::AsNumber(FMath::FloorToInt(NewAttackPower)));
		
		UE_LOG(LogTemp, Log, TEXT("StatusWidget: 공격력 UI 업데이트 - %.1f -> %.1f"), 
			OldAttackPower, NewAttackPower);
	}
}

void UStatusWidget::OnDefenseChanged(float NewDefense, float OldDefense)
{
	if (DefText)
	{
		DefText->SetText(FText::AsNumber(FMath::FloorToInt(NewDefense)));
		
		UE_LOG(LogTemp, Log, TEXT("StatusWidget: 방어력 UI 업데이트 - %.1f -> %.1f"), 
			OldDefense, NewDefense);
	}
}
