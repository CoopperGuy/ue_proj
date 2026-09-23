#include "GAS/DiaAbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"

FGameplayEffectContextHandle UDiaAbilitySystemComponent::MakeEffectContext() const
{
	FGameplayEffectContextHandle Context = Super::MakeEffectContext();
	AActor* Avatar = GetAvatarActor();
	// Controllers/player states can own actor info while this ASC lives on the
	// character. GAS resolves source attribute captures through the instigator.
	if (Context.GetInstigatorAbilitySystemComponent() != this
		&& UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Avatar) == this)
	{
		Context.AddInstigator(Avatar, Avatar);
	}
	return Context;
}
