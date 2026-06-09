#include "Logging/ARPGLogChannels.h"

#include "GameFramework/Actor.h"

DEFINE_LOG_CATEGORY(LogARPG_Inventory);
DEFINE_LOG_CATEGORY(LogARPG_GAS);
DEFINE_LOG_CATEGORY(LogARPG_Map);
DEFINE_LOG_CATEGORY(LogARPG_Room);
DEFINE_LOG_CATEGORY(LogARPG_Spawn);
DEFINE_LOG_CATEGORY(LogARPG_UI);
DEFINE_LOG_CATEGORY(LogARPGAttribute);

DEFINE_LOG_CATEGORY(LogARPG);
DEFINE_LOG_CATEGORY(LogARPG_Skill);
DEFINE_LOG_CATEGORY(LogARPG_Combat);
DEFINE_LOG_CATEGORY(LogARPG_AI);

namespace ARPGLog
{
	FString DescribeActor(const AActor* Actor)
	{
		if (!IsValid(Actor))
		{
			return TEXT("None");
		}

		FString Role = TEXT("Actor");
		if (Actor->ActorHasTag(FName(TEXT("Actor.Player"))))
		{
			Role = TEXT("Player");
		}
		else if (Actor->ActorHasTag(FName(TEXT("Actor.Monster"))))
		{
			Role = TEXT("Monster");
		}
		else if (Actor->ActorHasTag(FName(TEXT("SkillActor.Ground"))))
		{
			Role = TEXT("SkillActor.Ground");
		}
		else if (Actor->ActorHasTag(FName(TEXT("SkillActor.Projectile"))))
		{
			Role = TEXT("SkillActor.Projectile");
		}
		else if (Actor->ActorHasTag(FName(TEXT("SkillActor.Default"))))
		{
			Role = TEXT("SkillActor.Default");
		}

		return FString::Printf(
			TEXT("%s:%s Class=%s"),
			*Role,
			*GetNameSafe(Actor),
			*GetNameSafe(Actor->GetClass()));
	}
}
