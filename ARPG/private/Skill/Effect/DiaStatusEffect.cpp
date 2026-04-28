// Fill out your copyright notice in the Description page of Project Settings.


#include "Skill/Effect/DiaStatusEffect.h"
#include "DiaBaseCharacter.h"

UDiaStatusEffect::UDiaStatusEffect()
{
    Duration = 5.0f;
    Strength = 1.0f;
    bCanStack = false;
    MaxStacks = 1;
    CurrentStacks = 0;
    TickInterval = 1.0f;
    bIsActive = false;
    RemainingTime = 0.0f;
    TimeSinceLastTick = 0.0f;
    EffectType = EStatusEffectType::Other;
}

void UDiaStatusEffect::Initialize(ADiaBaseCharacter* InOwner, float InDuration, float InStrength)
{
    OwnerCharacter = InOwner;

    if (InDuration > 0.0f)
    {
        Duration = InDuration;
    }

    if (InStrength > 0.0f)
    {
        Strength = InStrength;
    }

    RemainingTime = Duration;
}

void UDiaStatusEffect::Apply()
{
    if (!IsValid(OwnerCharacter))
    {
        return;
    }

    bIsActive = true;
    RemainingTime = Duration;
    CurrentStacks = FMath::Clamp(CurrentStacks + 1, 1, MaxStacks);

    // ȿ�� ���� ���� (�ڽ� Ŭ�������� ����)

    UE_LOG(LogTemp, Log, TEXT("Status Effect Applied: %s on %s (Stacks: %d)"),
        *DisplayName.ToString(), *OwnerCharacter->GetName(), CurrentStacks);
}

void UDiaStatusEffect::Remove()
{
    if (!bIsActive)
    {
        return;
    }

    bIsActive = false;

    // ȿ�� ���� ���� (�ڽ� Ŭ�������� ����)

    UE_LOG(LogTemp, Log, TEXT("Status Effect Removed: %s from %s"),
        *DisplayName.ToString(), *OwnerCharacter->GetName());
}

void UDiaStatusEffect::Tick(float DeltaTime)
{
    if (!bIsActive)
    {
        return;
    }

    // ���� �ð� ����
    if (Duration > 0.0f)
    {
        RemainingTime -= DeltaTime;

        // ���� �ð� ����
        if (RemainingTime <= 0.0f)
        {
            Remove();
            return;
        }
    }

    // �ֱ��� ȿ�� ó��
    TimeSinceLastTick += DeltaTime;
    if (TimeSinceLastTick >= TickInterval)
    {
        OnEffectTick();
        TimeSinceLastTick = 0.0f;
    }
}

bool UDiaStatusEffect::HasTag(const FName& Tag) const
{
    return Tags.Contains(Tag);
}

void UDiaStatusEffect::OnEffectTick()
{
    // �ڽ� Ŭ�������� ����
}

bool UDiaStatusEffect::AddStack()
{
    if (!bCanStack || CurrentStacks >= MaxStacks)
    {
        return false;
    }

    CurrentStacks++;

    // ������ �߰��� �� ���ӽð� ����
    if (Duration > 0)
    {
        RemainingTime = Duration;
    }

    return true;
}

bool UDiaStatusEffect::RemoveStack()
{
    if (CurrentStacks <= 0)
    {
        return false;
    }

    CurrentStacks--;

    // ������ 0�� �Ǹ� ȿ�� ����
    if (CurrentStacks <= 0)
    {
        Remove();
    }

    return true;
}

void UDiaStatusEffect::SetCurrentStacks(int32 NewStacks)
{
    if (!bCanStack)
    {
        CurrentStacks = 1;
        return;
    }

    CurrentStacks = FMath::Clamp(NewStacks, 0, MaxStacks);

    // ������ 0�� �Ǹ� ȿ�� ����
    if (CurrentStacks <= 0)
    {
        Remove();
    }
}



