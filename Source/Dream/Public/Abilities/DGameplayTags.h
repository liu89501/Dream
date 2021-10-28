#pragma once

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

    // 生命偷取 SetByCallerTag
    FGameplayTag HealthStealSetByCallerTag;
};

static const MyCustomizeTags& CustomizeTags()
{
    static MyCustomizeTags Tags;
    return Tags;
}