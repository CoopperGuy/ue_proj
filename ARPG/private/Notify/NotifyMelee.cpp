// Fill out your copyright notice in the Description page of Project Settings.


#include "Notify/NotifyMelee.h"


UNotifyMelee::UNotifyMelee()
{
}

void UNotifyMelee::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (!MeshComp || !MeshComp->GetOwner())
    {
        return;
    }

    // 소유자가 플레이어인지 확인
    APawn* Owner = Cast<APawn>(MeshComp->GetOwner());
    if (!Owner)
    {
        return;
    }
    
    
}   

