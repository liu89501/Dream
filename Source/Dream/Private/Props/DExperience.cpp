// Fill out your copyright notice in the Description page of Project Settings.


#include "Props/DExperience.h"

const FPropsInfo& UDExperience::GetPropsInfo() const
{
	return PropsInformation;
}

ERewardNotifyMode UDExperience::GetRewardNotifyMode() const
{
	return ERewardNotifyMode::Secondary;
}
