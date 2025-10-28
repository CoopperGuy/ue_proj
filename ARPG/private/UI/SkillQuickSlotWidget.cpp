#include "UI/SkillQuickSlotWidget.h"
#include "Blueprint/UserWidget.h"
#include "UI/Skill/SkillQuickSlot.h"
#include "AbilitySystemComponent.h"
#include "System/GASSkillManager.h"


void USkillQuickSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 필수 위젯 유효성 확인(필요 시 초기화 로직 추가)
	// 현재는 BindWidget으로 바인딩만 확인합니다.
	SkillSlots = { SkillQuickSlot1, SkillQuickSlot2, SkillQuickSlot3, SkillQuickSlot4, SkillQuickSlot5 };
}

void USkillQuickSlotWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!CachedASC.IsValid())
	{
		if (APlayerController* PC = GetOwningPlayer())
		{
			if (APawn* Pawn = PC->GetPawn())
			{
				CachedASC = Cast<UAbilitySystemComponent>(Pawn->GetComponentByClass(UAbilitySystemComponent::StaticClass()));
			}
		}
	}

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
	if (SkillSlots.IsValidIndex(SlotIndex))
	{
		USkillQuickSlot* TargetSlot = SkillSlots[SlotIndex];
		if (TargetSlot)
		{
			if (SkillID > 0)
			{
				UGASSkillManager* GasSkillMgr = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGASSkillManager>() : nullptr;
				if (!GasSkillMgr)
				{
					TargetSlot->ClearSkillQuickSlot();
					return;
				}

				const FGASSkillData* FoundData = GasSkillMgr->GetSkillDataPtr(SkillID);
				if (FoundData)
				{
					if (!FoundData->Icon.IsNull())
					{
						UTexture2D* SkillIcon = FoundData->Icon.LoadSynchronous();
						if (SkillIcon)
						{
							int32 SkillLevel = 1;
							TargetSlot->SetSkillQuickSlot(SkillID, SkillIcon, SkillLevel);
						}
						else
						{
							TargetSlot->ClearSkillQuickSlot();
						}
					}
					else
					{
						TargetSlot->ClearSkillQuickSlot();
					}
				}
				else
				{
					TargetSlot->ClearSkillQuickSlot();
				}
			}
			else
			{
				TargetSlot->ClearSkillQuickSlot();
			}
		}
	}
}

void USkillQuickSlotWidget::StartCoolDownAnimation(int32 SlotIndex, UAbilitySystemComponent* ASC)
{
	SkillSlots[SlotIndex]->UpdateCoolTime(ASC);
}

void USkillQuickSlotWidget::EndCoolDownAnimation(int32 SlotIndex)
{
	SkillSlots[SlotIndex]->UpdateCoolTime(nullptr);
}
