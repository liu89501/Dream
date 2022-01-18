#include "DGameplayTags.h"

MyCustomizeTags::MyCustomizeTags()
{
	Condition_Immediately = FGameplayTag::RequestGameplayTag(TEXT("Condition.Immediately"));
	Condition_Reloading = FGameplayTag::RequestGameplayTag(TEXT("Condition.Reloading"));
	Condition_Injured = FGameplayTag::RequestGameplayTag(TEXT("Condition.Injured"));
	Condition_Firing_Client = FGameplayTag::RequestGameplayTag(TEXT("Condition.Firing_Client"));
	Condition_KilledEnemy = FGameplayTag::RequestGameplayTag(TEXT("Condition.KilledEnemy"));
	Condition_HitEnemy = FGameplayTag::RequestGameplayTag(TEXT("Condition.HitEnemy"));
	Condition_HitEnemy_Client = FGameplayTag::RequestGameplayTag(TEXT("Condition.HitEnemy_Client"));
	Condition_Crit = FGameplayTag::RequestGameplayTag(TEXT("Condition.Crit"));

	GE_Buff_Weapon = FGameplayTag::RequestGameplayTag(TEXT("GE.Buff.Weapon"));
	GE_Buff_Module = FGameplayTag::RequestGameplayTag(TEXT("GE.Buff.Module"));
	GE_Buff_Talent = FGameplayTag::RequestGameplayTag(TEXT("GE.Buff.Talent"));
	GE_Buff_All = FGameplayTag::RequestGameplayTag(TEXT("GE.Buff"));

	SetByCaller_HealthSteal = FGameplayTag::RequestGameplayTag(TEXT("GE.SetByCaller.HealthSteal"));
	
	Exec_Temporary_PercentageDmgInc = FGameplayTag::RequestGameplayTag(TEXT("Exec.Temporary.PercentageDmgInc"));
	Exec_Temporary_FixedDamage = FGameplayTag::RequestGameplayTag(TEXT("Exec.Temporary.FixedDamage"));
}
