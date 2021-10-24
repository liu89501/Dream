// Fill out your copyright notice in the Description page of Project Settings.


#include "Props/DExperience.h"

#define LOCTEXT_NAMESPACE "PropsInformation"

UDExperience::UDExperience()
{
	PropsInformation.PropsQuality = EPropsQuality::Advanced;
	PropsInformation.PropsName = NSLOCTEXT("PropsInformation", "Experience", "经验");

	ConstructorHelpers::FObjectFinder<UMaterialInstance> IconFinder(TEXT("/Game/Main/Widget/Material/Icon/Other/M_Icon_Exp"));
	PropsInformation.PropsIcon = IconFinder.Object;
}

const FPropsInfo& UDExperience::GetPropsInfo() const
{
	return PropsInformation;
}

ERewardNotifyMode UDExperience::GetRewardNotifyMode() const
{
	return ERewardNotifyMode::Secondary;
}

#undef LOCTEXT_NAMESPACE