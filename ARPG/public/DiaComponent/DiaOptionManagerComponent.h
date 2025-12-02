// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Types/DiaItemOptionRow.h"
#include "DiaOptionManagerComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ARPG_API UDiaOptionManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UDiaOptionManagerComponent();

protected:
	virtual void BeginPlay() override;

public:	
	//const reference로 변수를 받고, 이걸 그대로 저장해도 되나?
	void AddOption(const FDiaItemOptionRow& NewOption);
	void RemoveOption(const FDiaItemOptionRow& OptionRow);
	void RemoveOption(const FName& OptionID);

	//받은 gameplay tag를 이용해 필요한 옵션을 꺼내오는 기능
	FDiaItemOptionRow* GetOptionByID(const FName& OptionID);
	FDiaItemOptionRow* GetOptionByOptionRow(const FDiaItemOptionRow& OptionRow);

	//모든 옵션을 반환
	const TMap<FName, FDiaItemOptionRow>& GetAllOptions() const { return ActiveOptions; }


private:
	//옵션을 통합적으로 포괄한 형태의 struct가 필요할 수 있을듯.
	//우선은, 옵션 데이터 테이블의 row struct를 그대로 사용
	//현재의 데이터들은 전부 깊은 복사가 되기 때문에 사용. 만약 포인터 같은것들이 들어가면 문제가 발생할지도
	//Key OptionID, Value OptionRow
	UPROPERTY()
	TMap<FName, FDiaItemOptionRow> ActiveOptions;
	
};
