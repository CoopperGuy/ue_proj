#include "UI/SkillQuickSlotWidget.h"
#include "DiaComponent/DiaSkillManagerComponent.h"
#include "DiaComponent/Skill/SkillObject.h"
#include "Blueprint/UserWidget.h"
#include "UI/Skill/SkillQuickSlot.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Controller/DiaController.h"
#include "GameFramework/Pawn.h"
#include "System/GASSkillManager.h"
#include "Components/ProgressBar.h"

void USkillQuickSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 필수 위젯 유효성 확인(필요 시 초기화 로직 추가)
	// 현재는 BindWidget으로 바인딩만 확인합니다.
	if (!IsValid(ExpBar))
		return;
	if(!IsValid(SkillQuickSlot1) || !IsValid(SkillQuickSlot2) || !IsValid(SkillQuickSlot3) || !IsValid(SkillQuickSlot4) || !IsValid(SkillQuickSlot5))
		return;

	SkillSlots = { SkillQuickSlot1, SkillQuickSlot2, SkillQuickSlot3, SkillQuickSlot4, SkillQuickSlot5 };

	BindPlayerASCReadyEvent();
	TryInitializeFromPlayer();

	ExpBar->SetPercent(0.0f);
}

void USkillQuickSlotWidget::NativeDestruct()
{
	UnbindPlayerASCReadyEvent();
	CachedASC.Reset();

	Super::NativeDestruct();
}

bool USkillQuickSlotWidget::TryInitializeFromPlayer()
{
	APawn* Pawn = GetOwningPlayerPawn();
	if (!IsValid(Pawn))
	{
		CachedASC.Reset();
		return false;
	}

	UAbilitySystemComponent* AbilitySystem = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pawn);
	if (!IsValid(AbilitySystem))
	{
		CachedASC.Reset();
		return false;
	}

	CachedASC = AbilitySystem;
	return true;
}

void USkillQuickSlotWidget::BindPlayerASCReadyEvent()
{
	ADiaController* PlayerController = Cast<ADiaController>(GetOwningPlayer());
	if (!IsValid(PlayerController))
	{
		return;
	}

	if (BoundPlayerController.Get() == PlayerController && PlayerASCReadyDelegateHandle.IsValid())
	{
		return;
	}

	UnbindPlayerASCReadyEvent();
	BoundPlayerController = PlayerController;
	PlayerASCReadyDelegateHandle = PlayerController->GetOnPlayerASCReady()
		.AddUObject(this, &ThisClass::HandlePlayerASCReady);
}

void USkillQuickSlotWidget::UnbindPlayerASCReadyEvent()
{
	if (ADiaController* PlayerController = BoundPlayerController.Get();
		PlayerController && PlayerASCReadyDelegateHandle.IsValid())
	{
		PlayerController->GetOnPlayerASCReady().Remove(PlayerASCReadyDelegateHandle);
	}

	PlayerASCReadyDelegateHandle.Reset();
	BoundPlayerController.Reset();
}

void USkillQuickSlotWidget::HandlePlayerASCReady(
	ADiaBaseCharacter* /*PlayerCharacter*/,
	UAbilitySystemComponent* AbilitySystem)
{
	CachedASC = AbilitySystem;
}

void USkillQuickSlotWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 모든 스킬 슬롯의 쿨타임 업데이트
	UAbilitySystemComponent* ASC = CachedASC.Get();
	for (USkillQuickSlot* QuickSlot : SkillSlots)
	{
		if (QuickSlot)
		{
			QuickSlot->UpdateCoolTime(ASC);
		}
	}
}

void USkillQuickSlotWidget::UpdateSkillSlot(int32 SlotIndex, int32 SkillID)
{
    if (!SkillSlots.IsValidIndex(SlotIndex) || !SkillSlots[SlotIndex]) return;
    USkillQuickSlot* TargetSlot = SkillSlots[SlotIndex];
    UGASSkillManager* Data = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGASSkillManager>() : nullptr;
    const FGASSkillData* SkillData = SkillID > 0 && Data ? Data->GetSkillDataPtr(SkillID) : nullptr;
    if (!SkillData)
    {
        TargetSlot->ClearSkillQuickSlot();
        return;
    }

    const APawn* Pawn = GetOwningPlayerPawn();
    const UDiaSkillManagerComponent* Skills = Pawn ? Pawn->FindComponentByClass<UDiaSkillManagerComponent>() : nullptr;
    const USkillObject* Skill = Skills ? Skills->GetSkillObjectBySkillID(SkillID) : nullptr;
    // An unavailable icon must not discard the skill identity or its tooltip.
    TargetSlot->SetSkillQuickSlot(SkillID, SkillData->Icon.LoadSynchronous(), Skill ? Skill->GetSkillLevel() : 1);
}

void USkillQuickSlotWidget::StartCoolDownAnimation(int32 SlotIndex, UAbilitySystemComponent* ASC)
{
	if (SkillSlots.IsValidIndex(SlotIndex) && SkillSlots[SlotIndex])
	{
		SkillSlots[SlotIndex]->UpdateCoolTime(ASC);
	}
}

void USkillQuickSlotWidget::EndCoolDownAnimation(int32 SlotIndex)
{
	if (SkillSlots.IsValidIndex(SlotIndex) && SkillSlots[SlotIndex])
	{
		SkillSlots[SlotIndex]->UpdateCoolTime(nullptr);
	}
}

void USkillQuickSlotWidget::SetExpBarPercent(float Percent)
{
	if (ExpBar)
	{
		ExpBar->SetPercent(Percent);
	}
}
