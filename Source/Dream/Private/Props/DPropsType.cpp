// ReSharper disable CppSomeObjectMembersMightNotBeInitialized
#include "DPropsType.h"
#include "Kismet/KismetMathLibrary.h"

#define RANDOM_ITEM(Item) if (AttrAssign.Item.Num() > 0) { Attributes.Item = AttrAssign.Item[UKismetMathLibrary::RandomInteger(AttrAssign.Item.Num())]; }

const FPropsInfo FEmptyStruct::EmptyPropsInfo = FPropsInfo();
const FEquipmentAttributes FEmptyStruct::EmptyAttributes = FEquipmentAttributes();
const FSoftClassPath FEmptyStruct::EmptySoftClassPath = FSoftClassPath();

FEquipmentAttributes FEquipmentAttributesAssign::AssignAttributes(const FEquipmentAttributesAssign& AttrAssign)
{
	FEquipmentAttributes Attributes;
	RANDOM_ITEM(AttackPower);
	RANDOM_ITEM(Defense);
	RANDOM_ITEM(Penetration);
	RANDOM_ITEM(CriticalDamage);
	RANDOM_ITEM(CriticalRate);
	RANDOM_ITEM(DamageReduction);
	RANDOM_ITEM(HealthSteal);

	if (AttrAssign.Perks.Num() > 0)
	{
		Attributes.Perks = AttrAssign.Perks[UKismetMathLibrary::RandomInteger(AttrAssign.Perks.Num())].Perks;
	}
	return Attributes;
}

FEquipmentAttributes& FEquipmentAttributes::operator+=(const FEquipmentAttributes& RHS)
{
	AttackPower += RHS.AttackPower;
	Defense += RHS.Defense;
	Penetration += RHS.Penetration;
	CriticalDamage += RHS.CriticalDamage;
	CriticalRate += RHS.CriticalRate;
	DamageReduction += RHS.DamageReduction;
	HealthSteal += RHS.HealthSteal;
	Perks += RHS.Perks;
	return *this;
}

