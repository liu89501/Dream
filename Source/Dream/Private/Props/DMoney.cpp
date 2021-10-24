// Fill out your copyright notice in the Description page of Project Settings.


#include "Props/DMoney.h"

#define LOCTEXT_NAMESPACE "PropsInformation"

UDMoney::UDMoney()
{
	PropsInformation.PropsQuality = EPropsQuality::Advanced;
	PropsInformation.PropsName = NSLOCTEXT("PropsInformation", "Money", "交易券");

	ConstructorHelpers::FObjectFinder<UMaterialInstance> IconFinder(TEXT("/Game/Main/Widget/Material/Icon/Other/M_Icon_RMB"));
	PropsInformation.PropsIcon = IconFinder.Object;
}

const FPropsInfo& UDMoney::GetPropsInfo() const
{
	return PropsInformation;
}

ERewardNotifyMode UDMoney::GetRewardNotifyMode() const
{
	return ERewardNotifyMode::Secondary;
}

#undef LOCTEXT_NAMESPACE