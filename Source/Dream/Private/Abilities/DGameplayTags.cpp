#include "DGameplayTags.h"

MyCustomizeTags::MyCustomizeTags()
{
	Condition_Immediately = FGameplayTag::RequestGameplayTag(TEXT("Condition.Immediately"));
	Condition_Reloading = FGameplayTag::RequestGameplayTag(TEXT("Condition.Reloading"));
	Condition_Injured = FGameplayTag::RequestGameplayTag(TEXT("Condition.Injured"));
	Condition_Firing = FGameplayTag::RequestGameplayTag(TEXT("Condition.Firing"));
	Condition_KilledEnemy = FGameplayTag::RequestGameplayTag(TEXT("Condition.KilledEnemy"));
	Condition_HitEnemy = FGameplayTag::RequestGameplayTag(TEXT("Condition.HitEnemy"));

	GE_Buff_Weapon = FGameplayTag::RequestGameplayTag(TEXT("GE.Buff.Weapon"));
	GE_Buff_Module = FGameplayTag::RequestGameplayTag(TEXT("GE.Buff.Module"));
	GE_Buff_Talent = FGameplayTag::RequestGameplayTag(TEXT("GE.Buff.Talent"));
	GE_Buff_All = FGameplayTag::RequestGameplayTag(TEXT("GE.Buff"));

	HealthStealSetByCallerTag = FGameplayTag::RequestGameplayTag(TEXT("GE.SetByCaller.HealthSteal"));
}
