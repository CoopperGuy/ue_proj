// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SkillSlotWidget.generated.h"

class UImage;
class UTextBlock;
//사용시 블루프린트 생성 후 해당 블루프린트를 스킬 슬롯에 (skillpanelwidget) 등록하여 사용
/**
 * 
 */
UCLASS()
class ARPG_API USkillSlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void NativeConstruct() override;

	void SetSkillInfo(int32 ID, UTexture2D* Icon, const FText& Name, int32 Level);

	void SetSkillID(int32 ID) { SkillID = ID; }

	void SetSkillIcon(UTexture2D* Icon);
	void SetSkillName(const FText& Name);
	void SetSkillLevel(int32 Level);
protected:

private:
	UPROPERTY(meta = (BindWidget))
	UImage* SkillIconImage;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* SkillNameText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* SkillLevelText;

	int32 SkillID;
};
