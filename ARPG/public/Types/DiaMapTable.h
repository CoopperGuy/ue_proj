// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DiaMapTable.generated.h"

USTRUCT(BlueprintType)
struct FMapInfoTable : public FTableRowBase
{
	GENERATED_BODY()
	
	// 맵 ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Info")
	FName MapID;
	
	// 맵 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Info")
	FText DisplayName;
	
	// 맵 설명
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Info")
	FText Description;
	
	// 최소 레벨
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Info")
	int32 MinLevel = 1;
	
	// 최대 레벨
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Info")
	int32 MaxLevel = 100;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Info")
	float MpaRadius = 1000.f;

	// 맵 타입 (필드, 던전, 마을 등)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Info")
	FName MapType;


};

