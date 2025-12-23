// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "StatusWidget.generated.h"

class UTextBlock;
class UStatusSet;
class UListView;
class UStatusItemObject;
/**
 * 캐릭터 상태 UI 위젯 - GAS AttributeSet 기반
 */
UCLASS()
class ARPG_API UStatusWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 위젯 초기화 및 AttributeSet 바인딩
	virtual void NativeConstruct() override;
				
	void OnUpdateStats(const FOnAttributeChangeData& Data);

protected:
	// ASC 캐싱
	UPROPERTY()
	UAbilitySystemComponent* CachedASC = nullptr;

	UPROPERTY(meta = (BindWidget))
	UListView* StatusList;

	//UPROPERTY(EditAnywhere, Category = "UI")
	//TSubclassOf<UStatusSet> StatusSetClass;

	UPROPERTY()
	TMap<FString, UStatusItemObject*> StatusSetMap;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tag")
	TSet<FGameplayTag> HasMaxStatusTags;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tag")
	TSet<FGameplayTag> IsVisibleStatusTags;
	//나중에는 표현할 Tag만 모아서 할 수도있겠다.
};
