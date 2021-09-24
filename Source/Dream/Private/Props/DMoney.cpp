// Fill out your copyright notice in the Description page of Project Settings.


#include "Props/DMoney.h"

const FPropsInfo& UDMoney::GetPropsInfo() const
{
	return PropsInformation;
}

ERewardNotifyMode UDMoney::GetRewardNotifyMode() const
{
	return ERewardNotifyMode::Secondary;
}
