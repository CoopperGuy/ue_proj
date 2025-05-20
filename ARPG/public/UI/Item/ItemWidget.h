// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/ItemBase.h"
#include "ItemWidget.generated.h"

class UImage;
/**
 * 
 */
UCLASS()
class ARPG_API UItemWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	
	void SetItemInfo(const FInventoryItem& ItemData);
protected:
	UPROPERTY(meta = (BindWidget))
	UImage* ItemIcon;

	FInventoryItem ItemInfo;
};
