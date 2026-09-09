// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include <GameplayEffectTypes.h>
#include "Types/DiaRewardData.h"

#include "HUDWidget.generated.h"


enum class OrbType
{
	OT_HP,
	OT_MP,
	OT_MAX
};

enum class BarType
{
	BT_HP,
	BT_MAX
};

class UOrb;
class UMonsterHP;
class UDiaPrimaryLayout;
class ADiaBaseCharacter;
class ADiaController;
class UAbilitySystemComponent;
class ADiaMonster;
class USkillQuickSlotWidget;
class UDiaDamagePopup;
class URewardChoicePanel;
/**
 * 기존에는 이 클래스에서 스탯, 스킬창, 인벤토리 등등 다양한 위젯을 처리했는데 이걸 이제 다른곳으로 옮겨서 관리하도록 변경할거임
 * PrimaryLayout에서 관리하도록 변경
 * 이유는 다양한데, 우선 입력 처리와 관련된 부분의 문제
 * 또, HUDWidget은 HUD에만 관련된 위젯을 관리하도록 변경할거임
 */
UCLASS()
class ARPG_API UHUDWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	void UpdateOrbPercentage(OrbType _Type, float _Percentage);
	void UpdateMonsterPercentage(BarType _Type, float _Percentage, bool IsBoss = false);
	void UpdateExpPercentage(float _Percentage);

	void CreateCautionWidget();

	void RegisteSkillOnQuickSlotWidget(int32 SkillID, int32 SlotIndex);

	void ShowClearAlret(FGuid RoomGuid);
	void ShowDamagePopup(float DamageAmount, const FVector2D& ScreenPos, bool bIsCritical);

	void OpenRewardChoicePanel(const FText& Title, const FText& Subtitle, const TArray<FRewardData>& RewardOptions);

protected:
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;

	void BindCharacterAttributeChangeDelegates();
	void UnBindCharacterAttributeChangeDelegates();
protected:
	UPROPERTY(meta = (BindWidget))
	UOrb* HpWidget;

	UPROPERTY(meta = (BindWidget))
	UOrb* MpWidget;

	UPROPERTY(meta = (BindWidget))
	UMonsterHP* MonsterHPWidget;

	UPROPERTY(meta = (BindWidget))
	UMonsterHP* BossMonsterHPWidget;

	UPROPERTY(meta = (BindWidget))
	USkillQuickSlotWidget* SkillQuickSlotWidget;

	UPROPERTY(meta = (BindWidget))
	URewardChoicePanel* RewardChoicePanel;

	UPROPERTY()
	UDiaPrimaryLayout* DiaPrimaryLayout;

	UPROPERTY()
	ADiaBaseCharacter* TargetMonster;

	FDelegateHandle HealthChangedDelegateHandle;
	FDelegateHandle ManaChangedDelegateHandle;
	FDelegateHandle ExpChangedDelegateHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CommonWidget")
	TSubclassOf<UCommonActivatableWidget> ClearAlertWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage")
	TSubclassOf<UDiaDamagePopup> DamagePopupWidgetClass;

	UPROPERTY(Transient)
	TWeakObjectPtr<ADiaController> CachedOwningController;

	UPROPERTY(Transient)
	TWeakObjectPtr<UAbilitySystemComponent> CachedAbilitySystem;

	FDelegateHandle TargetChangedDelegateHandle;

	void HandleHealthChanged(const FOnAttributeChangeData& Data);
	void HandleManaChanged(const FOnAttributeChangeData& Data);
	void HandlExpChanged(const FOnAttributeChangeData& Data);

	UFUNCTION()
	void UpdateTagetMonster(ADiaBaseCharacter* NewTarget);

	UFUNCTION()
	void OnRewardCardSelected(FRewardData RewardData);

public:
	void SetMonsterHPVisibility(ESlateVisibility _Visibility);
};
