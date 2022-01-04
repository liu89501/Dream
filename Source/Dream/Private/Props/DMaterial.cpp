// Fill out your copyright notice in the Description page of Project Settings.


#include "Props/DMaterial.h"

const FPropsInfo& UDMaterial::GetPropsInfo() const
{
	return PropsInformation;
}

ERewardNotifyMode UDMaterial::GetRewardNotifyMode() const
{
	return ERewardNotifyMode::Secondary;
}
