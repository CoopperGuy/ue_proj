// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaComponent/DiaOptionManagerComponent.h"


UDiaOptionManagerComponent::UDiaOptionManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UDiaOptionManagerComponent::BeginPlay()
{
	Super::BeginPlay();	
}

void UDiaOptionManagerComponent::AddOption(const FDiaItemOptionRow& NewOption)
{
	ActiveOptions.Emplace(NewOption.OptionID, NewOption);
}

void UDiaOptionManagerComponent::RemoveOption(const FDiaItemOptionRow& OptionRow)
{
	RemoveOption(OptionRow.OptionID);
}

void UDiaOptionManagerComponent::RemoveOption(const FName& OptionID)
{
	ActiveOptions.Remove(OptionID);
}

FDiaItemOptionRow* UDiaOptionManagerComponent::GetOptionByID(const FName& OptionID)
{
	if(FDiaItemOptionRow* FoundOption = ActiveOptions.Find(OptionID))
	{
		return FoundOption;
	}
	return nullptr;
}

FDiaItemOptionRow* UDiaOptionManagerComponent::GetOptionByOptionRow(const FDiaItemOptionRow& OptionRow)
{
	return GetOptionByID(OptionRow.OptionID);
}


