#include "UI/SkillQuickSlotWidget.h"
#include "Blueprint/UserWidget.h"
#include "UI/Skill/SkillQuickSlot.h"
#include "System/GASSkillManager.h"


void USkillQuickSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 필수 위젯 유효성 확인(필요 시 초기화 로직 추가)
	// 현재는 BindWidget으로 바인딩만 확인합니다.
	SkillSlots = { SkillQuickSlot1, SkillQuickSlot2, SkillQuickSlot3, SkillQuickSlot4, SkillQuickSlot5 };
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

				TSubclassOf<UGameplayAbility> AbilityClass = nullptr;
				const FGASSkillData* FoundData = GasSkillMgr->GetSkillDataPtr(SkillID);
				if (FoundData)
				{
					AbilityClass = FoundData->AbilityClass ? FoundData->AbilityClass : nullptr;

					//HACK 레벨 다시 불러와야함
					if (FoundData->Icon)
					{
						UTexture2D* SkillIcon = FoundData->Icon.LoadSynchronous();
						int32 SkillLevel = 1;
						TargetSlot->SetSkillQuickSlot(SkillID, SkillIcon, SkillLevel);
					}
				}
				TargetSlot->ClearSkillQuickSlot();
			}
			else
			{
				TargetSlot->ClearSkillQuickSlot();
			}
		}
	}
}
