// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/CharacterStatus/StatusSet.h"
#include "Components/TextBlock.h"
void UStatusSet::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	//const UStatusSet* StatusSet = Cast<UStatusSet>(ListItemObject);
	//if (!IsValid(StatusSet)) return;

	UStatusItemObject* Data = Cast<UStatusItemObject>(ListItemObject);
	if (!IsValid(Data)) return;

	UpdateStatusItemObject(Data);

}

void UStatusSet::UpdateStatusItemObject(UStatusItemObject* InStatusItemObjectPtr)
{
	StatusItemObjectPtr = InStatusItemObjectPtr;

	SetStatusName(FText::FromString(StatusItemObjectPtr->StatusName));
	SetStatusValue(FText::FromString(StatusItemObjectPtr->StatusValue),
		FText::FromString(StatusItemObjectPtr->StatusMaxValue));
}

void UStatusSet::SetStatusName(const FText& NewName)
{
	StatusName->SetText(NewName);
}

void UStatusSet::SetStatusValue(const FText& NewValue, const FText& NewMaxValue)
{
	if (StatusItemObjectPtr->bShowMaxValue)
	{
		StatusValue->SetText(FText::Format(
			FText::FromString(TEXT("{0}/{1}")),NewValue, NewMaxValue));
		UE_LOG(LogTemp, Warning, TEXT("SetStatusValue called with MaxValue: %s"), *NewMaxValue.ToString());
	}
	else if(StatusItemObjectPtr->bIsPercent)
	{
		StatusValue->SetText(FText::Format(FText::FromString(TEXT("{0}%")), NewValue));
	}
	else
	{
		StatusValue->SetText(NewValue);
		UE_LOG(LogTemp, Warning, TEXT("SetStatusValue called without Value: %s"), *NewValue.ToString());
	}
}
	