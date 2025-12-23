#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Types/DiaCharacterTable.h"

#include "Types/DiaMonsterTable.h"

#include "DiaAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)


UCLASS()
class ARPG_API UDiaAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UDiaAttributeSet();	

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	void InitializeCharacterAttributes(FName CharacterID, int32 Level = 1);
	void InitializeMonsterAttributes(const FMonsterInfo& MonsterInfo);
	// Health Attributes
	UPROPERTY(BlueprintReadOnly, Category = "Health")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UDiaAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, Category = "Health")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UDiaAttributeSet, MaxHealth)

	// Mana Attributes
	UPROPERTY(BlueprintReadOnly, Category = "Mana")
	FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS(UDiaAttributeSet, Mana)

	UPROPERTY(BlueprintReadOnly, Category = "Mana")
	FGameplayAttributeData MaxMana;
	ATTRIBUTE_ACCESSORS(UDiaAttributeSet, MaxMana)

	UPROPERTY(BlueprintReadOnly, Category = "Exp")
	FGameplayAttributeData Exp;
	ATTRIBUTE_ACCESSORS(UDiaAttributeSet, Exp)

	UPROPERTY(BlueprintReadOnly, Category = "Exp")
	FGameplayAttributeData MaxExp;
	ATTRIBUTE_ACCESSORS(UDiaAttributeSet, MaxExp)

	// Base Stats
	UPROPERTY(BlueprintReadOnly, Category = "BaseStats")
	FGameplayAttributeData Strength;
	ATTRIBUTE_ACCESSORS(UDiaAttributeSet, Strength)

	UPROPERTY(BlueprintReadOnly, Category = "BaseStats")
	FGameplayAttributeData Dexterity;
	ATTRIBUTE_ACCESSORS(UDiaAttributeSet, Dexterity)

	UPROPERTY(BlueprintReadOnly, Category = "BaseStats")
	FGameplayAttributeData Intelligence;
	ATTRIBUTE_ACCESSORS(UDiaAttributeSet, Intelligence)

	// Combat Attributes
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FGameplayAttributeData AttackPower;
	ATTRIBUTE_ACCESSORS(UDiaAttributeSet, AttackPower)
	
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FGameplayAttributeData Defense;
	ATTRIBUTE_ACCESSORS(UDiaAttributeSet, Defense)

	//Item Options
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FGameplayAttributeData DamageIncreaseOption;
	ATTRIBUTE_ACCESSORS(UDiaAttributeSet, DamageIncreaseOption)

	
	// Movement Attributes
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FGameplayAttributeData MovementSpeed;
	ATTRIBUTE_ACCESSORS(UDiaAttributeSet, MovementSpeed)
	
	static bool TranslateAttributeTagToAttribute(const FGameplayTag& AttributeTag, FGameplayAttribute& OutAttribute);
	static TMap<FGameplayTag, FGameplayAttribute> AttributeTagMap;
protected:
	// Helper function to adjust attributes for changes
	void AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty);
	void AdjustAttributeForValChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, float NewValue, const FGameplayAttribute& AffectedAttributeProperty);
	static void MakeAttributeTagMap();
};