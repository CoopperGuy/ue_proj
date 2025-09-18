// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/DiaCharacter.h"
#include "System/CharacterManager.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

#include "Controller/DiaController.h"

#include "DiaComponent/DiaCombatComponent.h"
#include "DiaComponent/DiaStatComponent.h"

#include "DiaInstance.h"
#include "Skill/DiaSkillManager.h"

#include "GameMode/DungeonGameMode.h"

#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

#include "UI/HUDWidget.h"

#include "Camera/CameraComponent.h"
#include "GAS/DiaGASHelper.h"
#include "GAS/Abilities/DiaBasicAttackAbility.h"
#include "AbilitySystemComponent.h"
#include "System/GASSkillManager.h"

// Sets default values
ADiaCharacter::ADiaCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    // ī�޶� �� ����
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 800.0f;
    CameraBoom->bUsePawnControlRotation = false;
    CameraBoom->bInheritPitch = false;
    CameraBoom->bInheritRoll = false;
    CameraBoom->bInheritYaw = false;
    CameraBoom->SetRelativeRotation(FRotator(-60.0f, 0.0f, 0.0f));

    // ī�޶� ����
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    bUseControllerRotationYaw = false;
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;

    GetCharacterMovement()->bOrientRotationToMovement = false;
    GetCharacterMovement()->bUseControllerDesiredRotation = true;

    SkillActions.Reserve(MaxSkillMapping);
    SkillIDMapping.Reserve(MaxSkillMapping);

    SkillActions.Init(nullptr, MaxSkillMapping);
    SkillIDMapping.Init(-1, MaxSkillMapping);

    Tags.Add(FName(TEXT("Player")));
}

// Called when the game starts or when spawned
void ADiaCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// 캐릭터 매니저를 통한 플레이어 초기화
	UGameInstance* GI = GetGameInstance();
	if (GI)
	{
		UCharacterManager* CharacterManager = GI->GetSubsystem<UCharacterManager>();
		if (CharacterManager)
		{
			// 기본 캐릭터로 초기화 (나중에 게임 시작 시 선택된 캐릭터로 변경 가능)
			FName DefaultCharacterID = CharacterManager->DefaultCharacterID;
			CharacterManager->InitializePlayerCharacter(this, DefaultCharacterID, 1);
			
			UE_LOG(LogTemp, Log, TEXT("DiaCharacter: 캐릭터 초기화 완료 - %s"), *DefaultCharacterID.ToString());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("DiaCharacter: CharacterManager를 찾을 수 없음"));
		}
	}
	
    // Input Mapping Context 추가
    if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }

    SetupInitialSkills();
}

void ADiaCharacter::SetupInitialSkills()
{
    Super::SetupInitialSkills();
}

// Called every frame
void ADiaCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    UpdateCharacterRotation();
}

float ADiaCharacter::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    float damage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    return damage;
}

void ADiaCharacter::Die()
{
    Super::Die();
}

void ADiaCharacter::UpdateHPGauge(float CurHealth, float MaxHelath)
{
    ADungeonGameMode* DungeonGameMode = Cast<ADungeonGameMode>(GetWorld()->GetAuthGameMode());

    // 로그 추가 (디버깅용)
    if (!DungeonGameMode)
    {
        UE_LOG(LogTemp, Warning, TEXT("UpdateHPGauge: DungeonGameMode is null"));
        return;
    }

    UHUDWidget* HUD = DungeonGameMode->GetHUDWidget();
    if (HUD)
    {
        float HPPersentage = CurHealth / MaxHelath;
        HUD->UpdateOrbPercentage(OrbType::OT_HP, HPPersentage);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("UpdateHPGauge: HUD Widget is null"));
    }
}
// Called to bind functionality to input
void ADiaCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
    {
        // Movement
        EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &ADiaCharacter::Move);

        // Look
        EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ADiaCharacter::Look);

        // 스킬 바인딩
        for (int32 i = 0; i < SkillActions.Num(); ++i)
        {
            if (IsValid(SkillActions[i]))
            {
                EnhancedInputComponent->BindAction(SkillActions[i], ETriggerEvent::Started, this,
                    &ADiaCharacter::ExecuteSkillByIndex, i);
                #if WITH_EDITOR || UE_BUILD_DEVELOPMENT
                    UE_LOG(LogTemp, Log, TEXT("스킬 바인딩 완료 - 인덱스: %d, 액션: %s"), 
                        i, *SkillActions[i]->GetName());
                #endif
            }
        }

        // 인벤토리 토글 바인딩
        if (InventoryAction)
        {
            EnhancedInputComponent->BindAction(InventoryAction, ETriggerEvent::Triggered, this, &ADiaCharacter::ToggleInventory);
        }
        if (CharacterStatusAction)
        {
			EnhancedInputComponent->BindAction(CharacterStatusAction, ETriggerEvent::Triggered, this, &ADiaCharacter::ToggleCharacterStatus);
        }
        if (SkillPanelAction)
        {
			EnhancedInputComponent->BindAction(SkillPanelAction, ETriggerEvent::Triggered, this, &ADiaCharacter::ToggleSkillPanel);
        }
    }
}

void ADiaCharacter::GrantInitialGASAbilities()
{
    Super::GrantInitialGASAbilities();
    UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
    if (!ASC)
    {
        UE_LOG(LogTemp, Warning, TEXT("GrantInitialGASAbilities: No ASC"));
        return;
    }

    UGASSkillManager* GasSkillMgr = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGASSkillManager>() : nullptr;
    if (!GasSkillMgr)
    {
        UE_LOG(LogTemp, Warning, TEXT("GrantInitialGASAbilities: No GASSkillManager"));
    }

    if (false)
    {
        int32 ChosenSkillID = 1001;
        TSubclassOf<UGameplayAbility> ChosenAbilityClass = UDiaBasicAttackAbility::StaticClass();

        if (GasSkillMgr)
        {
            TArray<FGASSkillData> AllSkills = GasSkillMgr->GetAllSkillData();
            AllSkills.Sort([](const FGASSkillData& A, const FGASSkillData& B){ return A.SkillID < B.SkillID; });
            for (const FGASSkillData& Data : AllSkills)
            {
                if (Data.AbilityClass)
                {
                    ChosenSkillID = Data.SkillID;
                    ChosenAbilityClass = Data.AbilityClass;
                    break;
                }
            }
        }

        bool bGranted = false;
        if (GasSkillMgr)
        {
            if (const FGASSkillData* FoundData = GasSkillMgr->GetSkillDataPtr(ChosenSkillID))
            {
                bGranted = UDiaGASHelper::GrantAbilityFromSkillData(ASC, *FoundData, ChosenSkillID);
            }
        }
        if (!bGranted)
        {
            FGameplayAbilitySpec Spec(ChosenAbilityClass, 1, ChosenSkillID, this);
            ASC->GiveAbility(Spec);
        }
        if (SkillIDMapping.IsValidIndex(0))
        {
            SkillIDMapping[0] = ChosenSkillID;
        }

        UE_LOG(LogTemp, Log, TEXT("GAS Ability added - SkillID: %d, Class: %s"), ChosenSkillID, *GetNameSafe(ChosenAbilityClass.Get()));
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("GAS Skill Ready! Press '1' to test"));
        }
    }
}

void ADiaCharacter::Move(const FInputActionValue& Value)
{
    const FVector2D MovementVector = Value.Get<FVector2D>();

    if (IsValid(Controller))
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        // ����/����
        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        AddMovementInput(ForwardDirection, MovementVector.Y);

        // ��/��
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        AddMovementInput(RightDirection, MovementVector.X);
    }
}

bool ADiaCharacter::GetMouseWorldLocation(FVector& OutLocation) const
{
    return false;
}

void ADiaCharacter::SetTargetActor(ADiaBaseCharacter* NewTarget)
{
    if (NewTarget->ActorHasTag(FName("Monster")))
    {
        Cast<ADiaController>(GetController())->SetTarget(NewTarget);
    }
}

void ADiaCharacter::PlayDieAnimation()
{
    Super::PlayDieAnimation();
}

void ADiaCharacter::UpdateCharacterRotation()
{
    if (Controller == nullptr)
        return;
    if (!IsValid(Controller))
        return;
    
    APlayerController* PC = Cast<APlayerController>(Controller);
    if (!IsValid(PC))
        return ;

    FVector worldLocation, worldDirection;
    float ScreenX, ScreenY;
    PC->GetMousePosition(ScreenX, ScreenY);



    FVector playerPosition = GetActorLocation();
    if (PC->DeprojectScreenPositionToWorld(ScreenX, ScreenY, worldLocation, worldDirection))
    {
        FHitResult HitResult;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(this);

        const float TraceLength = 10000.0f;
        FVector TraceEnd = worldLocation + (worldDirection * TraceLength);

        if (GetWorld()->LineTraceSingleByChannel(HitResult, worldLocation, TraceEnd, ECC_Visibility, QueryParams))
        {
            FRotator lookRotation = UKismetMathLibrary::FindLookAtRotation(FVector(playerPosition.X, playerPosition.Y, 0.f), FVector(HitResult.Location.X, HitResult.Location.Y, 0.f));
            FRotator currentRotation = GetActorRotation();
            FRotator newRotation = FMath::RInterpTo(currentRotation, lookRotation, GetWorld()->GetDeltaSeconds(), RotationInterpSpeed);
            SetActorRotation(newRotation);
        }
    }

}

void ADiaCharacter::Look(const FInputActionValue& Value)
{
    //UpdateCharacterRotation();
}

void ADiaCharacter::ExecuteSkillByIndex(int32 ActionIndex)
{
    if (SkillIDMapping.IsValidIndex(ActionIndex) && SkillIDMapping[ActionIndex] != -1)
    {
        int32 skillID = SkillIDMapping[ActionIndex];
        
        #if WITH_EDITOR || UE_BUILD_DEVELOPMENT
            UE_LOG(LogTemp, Log, TEXT("스킬 실행 요청 - 인덱스: %d, 스킬ID: %d"), ActionIndex, skillID);
        #endif
        
        // GAS 스킬 먼저 시도 (ID 1000 이상은 GAS 스킬로 간주)
        if (skillID >= 1000)
        {
            UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
            if (ASC && UDiaGASHelper::TryActivateAbilityBySkillID(ASC, skillID))
            {
                UE_LOG(LogTemp, Log, TEXT("GAS 스킬 실행 성공 - ID: %d"), skillID);
                return;
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("GAS 스킬 실행 실패 - ID: %d"), skillID);
            }
        }
        
        //// 기존 스킬 시스템으로 폴백
        //if (IsValid(CombatComponent))
        //{
        //    #if WITH_EDITOR || UE_BUILD_DEVELOPMENT
        //        // 스킬 실행 로그
        //        const FSkillData* skillData = GetWorld()->GetGameInstance<UDiaInstance>()->GetSkillManager()->GetSkillData(skillID);
        //        if (skillData)
        //        {
        //            UE_LOG(LogTemp, Log, TEXT("기존 스킬 시스템 실행 - 인덱스: %d, 스킬ID: %d, 스킬이름: %s"), 
        //                ActionIndex, skillID, *skillData->SkillName.ToString());
        //        }
        //    #endif
        //    
        //    CombatComponent->ExecuteSkill(skillID);
        //}
        //else
        //{
        //    UE_LOG(LogTemp, Warning, TEXT("CombatComponent가 유효하지 않습니다."));
        //}
    }
    else
    {
        #if WITH_EDITOR || UE_BUILD_DEVELOPMENT
            UE_LOG(LogTemp, Warning, TEXT("유효하지 않은 스킬 인덱스: %d"), ActionIndex);
        #endif
    }
}

void ADiaCharacter::ToggleInventory()
{
    if (ADiaController* PlayerController = Cast<ADiaController>(Controller))
    {
        ESlateVisibility eVisibility =  PlayerController->GetInventoryVisibility();
		//보이는 상태면 false로 안보이게 끔, 아니면 true로 보이게끔
#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
        UE_LOG(LogTemp, Warning, TEXT("인벤토리 토글"));
#endif
        PlayerController->ToggleInventoryVisibility(eVisibility == ESlateVisibility::Visible ? false : true);
    }
}

void ADiaCharacter::ToggleCharacterStatus()
{
    if (ADiaController* PlayerController = Cast<ADiaController>(Controller))
    {
        ESlateVisibility eVisibility = PlayerController->GetWidgetVisibility("CharacterStatus");
        //보이는 상태면 false로 안보이게 끔, 아니면 true로 보이게끔
#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
        UE_LOG(LogTemp, Warning, TEXT("캐릭터 위젯 토글"));
#endif
        PlayerController->ToggleChracterStatusVisibility(eVisibility == ESlateVisibility::Visible ? false : true);
    }
}

void ADiaCharacter::ToggleSkillPanel()
{
    if (ADiaController* PlayerController = Cast<ADiaController>(Controller))
    {
        ESlateVisibility eVisibility = PlayerController->GetWidgetVisibility("SkillPanelWidget");
        PlayerController->ToggleSkillPanelVisibility(eVisibility == ESlateVisibility::Visible ? false : true);
    }
}