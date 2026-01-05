// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/DiaCharacter.h"
#include "System/CharacterManager.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

#include "Controller/DiaController.h"

#include "DiaInstance.h"
#include "Skill/DiaSkillManager.h"
#include "GAS/DiaAttributeSet.h"
#include "GameMode/DungeonGameMode.h"
#include "DiaComponent/DiaSkillManagerComponent.h"

#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

#include "UI/HUDWidget.h"

#include "GAS/DiaGameplayTags.h"

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

    //HACK 임시로 최대 개수 지정

	constexpr int32 MaxSkillMapping = 8;

    SkillActions.Reserve(MaxSkillMapping);

    SkillActions.Init(nullptr, MaxSkillMapping);

    Tags.Add(FName(TEXT("Player")));
}

// Called when the game starts or when spawned
void ADiaCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	UGameInstance* GI = GetGameInstance();
	if (GI)
	{
		UCharacterManager* CharacterManager = GI->GetSubsystem<UCharacterManager>();
		if (CharacterManager)
		{
			FName DefaultCharacterID = CharacterManager->DefaultCharacterID;
            AttributeSet->InitializeCharacterAttributes(DefaultCharacterID, 1);

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

}


void ADiaCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

    SetupInitialSkills();
}

void ADiaCharacter::SetupInitialSkills()
{
    //HACK 우선은 Warrior로 박아놓는다.
    SkillManagerComponent->LoadJobSKillDataFromTable(EJobType::Warrior);

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

void ADiaCharacter::Die(ADiaBaseCharacter* Causer)
{
    Super::Die(Causer);
}

void ADiaCharacter::UpdateHPGauge(float CurHealth, float MaxHelath)
{
  
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

        // Dodge
        EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Triggered, this, &ADiaCharacter::Dodge);
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

    //Dodge 스킬 부여
    bool isSuccsess = SetUpSkillID(DodgeSkillID);
    if(isSuccsess)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dodge 스킬 부여 성공"));
	}
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Dodge 스킬 부여 실패"));
    }
}

void ADiaCharacter::Move(const FInputActionValue& Value)
{
    //Stun시에 움직임 막음.
    UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
    bool bIsStunned = ASC->HasMatchingGameplayTag(FDiaGameplayTags::Get().State_Stunned);
    if (bIsStunned)
    {
        return;
    }

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

void ADiaCharacter::Dodge(const FInputActionValue& Value)
{
    UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
    if (ASC && UDiaGASHelper::TryActivateAbilityBySkillID(ASC, DodgeSkillID))
    {
        return;
    }
}

void ADiaCharacter::ExecuteSkillByIndex(int32 ActionIndex)
{

    if (int32 skillID = SkillManagerComponent->GetMappedSkillID(ActionIndex))
    {        
        // GAS 스킬 먼저 시도 (ID 1000 이상은 GAS 스킬로 간주)
        if (skillID >= 1000)
        {
            //Stun시에 스킬 사용 막음.
            UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
            bool bIsStunned = ASC->HasMatchingGameplayTag(FDiaGameplayTags::Get().State_Stunned);
            if (bIsStunned)
            {
                return;
            }

            if (ASC && UDiaGASHelper::TryActivateAbilityBySkillID(ASC, skillID))
            {
                return;
            }
        }
        
    }
    else
    {
        #if WITH_EDITOR || UE_BUILD_DEVELOPMENT
            UE_LOG(LogTemp, Warning, TEXT("유효하지 않은 스킬 인덱스: %d"), ActionIndex);
        #endif
    }
}

// GAS 스킬 설정 및 퀵슬롯 위젯 등록
bool ADiaCharacter::SetUpSkillID(int32 SkillID)
{
 	bool isSuccess = Super::SetUpSkillID(SkillID);
    if (isSuccess)
    {
        RegisteSkillOnQuickSlotWidget(SkillID, SkillManagerComponent->GetIndexOfSkillID(SkillID));
    }

    return isSuccess;
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

void ADiaCharacter::RegisteCurrentSkillList()
{
	const TArray<USkillObject*>& SkillIDMapping = SkillManagerComponent->GetSkillIDMapping();
    for (int32 i = 0; i < SkillIDMapping.Num(); ++i)
    {
        if (int32 SkillID = SkillManagerComponent->GetMappedSkillID(i))
        {
            RegisteSkillOnQuickSlotWidget(SkillID, i);
        }
	}
}

// 퀵슬롯 위젯에 스킬 등록
void ADiaCharacter::RegisteSkillOnQuickSlotWidget(int32 SkillID, int32 SlotIndex)
{
    if (ADiaController* PlayerController = Cast<ADiaController>(Controller))
    {
        PlayerController->RegisteSkillOnQuickSlotWidget(SkillID, SlotIndex);
    }
}