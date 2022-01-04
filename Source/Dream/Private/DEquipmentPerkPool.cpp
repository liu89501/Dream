// Fill out your copyright notice in the Description page of Project Settings.


#include "DEquipmentPerkPool.h"
#include "PlayerDataInterfaceType.h"

#define RandomlySelected(Assign, Source) \
	TArray<float> Weights_##Source; \
	Weights_##Source.SetNumUninitialized(Source.Num()); \
	for (int32 N = 0; N < Weights_##Source.Num(); N++) \
	{ \
		Weights_##Source[N] = Source[N].Probability; \
	} \
	\
	int32 Index_##Source = FRandomProbability::RandomProbability(Weights_##Source); \
	if (Index_##Source != INDEX_NONE) \
	{ \
		Assign = Source[Index_##Source].Value; \
	}

#define RandomlySelectedGroup(Target, Source, Type, Var, ItemVar) \
	for (const Type& List_##Type : Source) \
	{ \
		TArray<float> Weight_##Source; \
		Weight_##Source.SetNumUninitialized(List_##Type.Var.Num()); \
		for (int32 N = 0; N < Weight_##Source.Num(); N++) \
		{ \
			Weight_##Source[N] = List_##Type.Var[N].Probability; \
		} \
		int32 Index = FRandomProbability::RandomProbability(Weight_##Source); \
		if (Index != INDEX_NONE) \
		{ \
			Target.Add(List_##Type.Var[Index].ItemVar); \
		} \
	}

void UDPerkPool::GeneratePerks(TArray<int32>& PerkGuids)
{
	RandomlySelectedGroup(PerkGuids, PerkGroup, FPerkList, Perks, Guid.ItemGuid);
}

void UDEquipmentAttributesPool::GenerateAttributes(FEquipmentAttributes& Attributes)
{
	RandomlySelected(Attributes.AttackPower, AttackPower);
	RandomlySelected(Attributes.MaxHealth, MaxHealth);
	RandomlySelected(Attributes.CriticalDamage, CriticalDamage);
	RandomlySelected(Attributes.CriticalRate, CriticalRate);
	RandomlySelected(Attributes.HealthSteal, HealthSteal);
	RandomlySelected(Attributes.Defense, Defense);
	RandomlySelected(Attributes.DamageReduction, DamageReduction);
	RandomlySelected(Attributes.Penetration, Penetration);

	if (PerkPool != nullptr)
	{
		PerkPool->GeneratePerks(Attributes.Perks);
	}
}
