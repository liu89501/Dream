// Fill out your copyright notice in the Description page of Project Settings.


#include "Props/Module/DModuleBase.h"

const FPropsInfo& UDModuleBase::GetPropsInfo() const
{
	return PropsInfo;
}

const FEquipmentAttributes& UDModuleBase::GetEquipmentAttributes() const
{
	return ModuleAttributes;
}
