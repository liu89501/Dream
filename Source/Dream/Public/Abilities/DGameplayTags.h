﻿#pragma once

#include "GameplayTags.h"

struct MyCustomizeTags
{
    MyCustomizeTags();
    
    /** buff 相关 */
    FGameplayTag GE_Buff_Weapon;
    FGameplayTag GE_Buff_Module;
    FGameplayTag GE_Buff_Talent;
    FGameplayTag GE_Buff_All;

    /** perk 触发相关 */
    FGameplayTag Condition_Immediately;
    FGameplayTag Condition_Reloading;
    FGameplayTag Condition_Injured;
    FGameplayTag Condition_Firing;
    FGameplayTag Condition_KilledEnemy;
    FGameplayTag Condition_HitEnemy;
    FGameplayTag Condition_Crit; // 暴击时

    // 生命偷取 SetByCallerTag
    FGameplayTag SetByCaller_HealthSteal;

    /** GameplayExecution中的临时变量标签 */
    FGameplayTag Exec_Temporary_PercentageDmgInc; // 百分比增伤
    FGameplayTag Exec_Temporary_FixedDamage; // 固定伤害
};

static const MyCustomizeTags& CustomizeTags()
{
    static MyCustomizeTags Tags;
    return Tags;
}