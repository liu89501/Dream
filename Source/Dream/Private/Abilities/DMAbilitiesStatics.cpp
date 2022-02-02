// Fill out your copyright notice in the Description page of Project Settings.


#include "DMAbilitiesStatics.h"

#define Defense_Threshold 1500.f
#define DamageResist_Max 0.8f

#define LevelDiffDamageResist_Threshold 100.f

#define Recovery_Threshold 100.f
#define Recovery_Max_Time 5.f
#define Recovery_Min_Time 2.f

float UDMAbilitiesStatics::GetDamageResist(float Defense)
{
	return FMath::Min(DamageResist_Max, Defense / (Defense + Defense_Threshold));
}

float UDMAbilitiesStatics::GetHealthRecoveryTime(float Recovery)
{
	return FMath::Lerp(Recovery_Max_Time, Recovery_Min_Time, Recovery / (Recovery + Recovery_Threshold));
}

float UDMAbilitiesStatics::GetDamageResistFromLevelDiff(int32 LevelDiff)
{
	return LevelDiff / (LevelDiff + LevelDiffDamageResist_Threshold);
}
