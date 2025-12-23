// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "StatusSet.generated.h"

class UTextBlock;

UCLASS()
class ARPG_API UStatusItemObject : public UObject
{
	GENERATED_BODY()
public:
	FString StatusName;
	FString StatusValue;
	FString StatusMaxValue;

	bool bIsPercent = false;
	bool bShowMaxValue = false;
};
/**
 * 
 */
UCLASS()
class ARPG_API UStatusSet : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
	
public:
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	void UpdateStatusItemObject(UStatusItemObject* InStatusItemObjectPtr);
	void SetStatusName(const FText& NewName);
	void SetStatusValue(const FText& NewValue, const FText& NewMaxValue = FText());
protected:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* StatusName;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* StatusValue;

	UPROPERTY()
	TWeakObjectPtr<UStatusItemObject> StatusItemObjectPtr;
};
