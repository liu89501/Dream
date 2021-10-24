// ReSharper disable CppSomeObjectMembersMightNotBeInitialized
#include "DPropsType.h"
#include "DBaseAttributesAsset.h"
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
	RANDOM_ITEM(MaxHealth);

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
	MaxHealth += RHS.MaxHealth;
	Perks += RHS.Perks;
	return *this;
}

FEquipmentAttributes& FEquipmentAttributes::operator+=(const FEquipmentAttributes&& RHS)
{
	AttackPower += RHS.AttackPower;
	Defense += RHS.Defense;
	Penetration += RHS.Penetration;
	CriticalDamage += RHS.CriticalDamage;
	CriticalRate += RHS.CriticalRate;
	DamageReduction += RHS.DamageReduction;
	HealthSteal += RHS.HealthSteal;
	MaxHealth += RHS.MaxHealth;
	return *this;
}

FEquipmentAttributes& FEquipmentAttributes::operator+=(const FBaseAttributes& RHS)
{
	AttackPower += RHS.AttackPower;
	Defense += RHS.Defense;
	CriticalDamage += RHS.CriticalDamage;
	CriticalRate += RHS.CriticalRate;
	HealthSteal += RHS.HealthSteal;
	MaxHealth += RHS.MaxHealth;
	return *this;
}

FEquipmentAttributes FEquipmentAttributes::operator-(const FEquipmentAttributes& RHS) const
{
	FEquipmentAttributes Temp;
	Temp.AttackPower = AttackPower - RHS.AttackPower;
	Temp.Defense = Defense - RHS.Defense;
	Temp.Penetration = Penetration - RHS.Penetration;
	Temp.CriticalDamage = CriticalDamage - RHS.CriticalDamage;
	Temp.CriticalRate = CriticalRate - RHS.CriticalRate;
	Temp.DamageReduction = DamageReduction - RHS.DamageReduction;
	Temp.HealthSteal = HealthSteal - RHS.HealthSteal;
	Temp.MaxHealth = MaxHealth - RHS.MaxHealth;
	return Temp;
}

FEquipmentAttributes& FEquipmentAttributes::operator-=(const FEquipmentAttributes& RHS)
{
	AttackPower -= RHS.AttackPower;
	Defense -= RHS.Defense;
	Penetration -= RHS.Penetration;
	CriticalDamage -= RHS.CriticalDamage;
	CriticalRate -= RHS.CriticalRate;
	DamageReduction -= RHS.DamageReduction;
	HealthSteal -= RHS.HealthSteal;
	MaxHealth -= RHS.MaxHealth;
	return *this;
}

FEquipmentAttributes::FEquipmentAttributes(FEquipmentAttributes&& Other) noexcept
{
	AttackPower = Other.AttackPower;
	Defense = Other.Defense;
	Penetration = Other.Penetration;
	CriticalDamage = Other.CriticalDamage;
	CriticalRate = Other.CriticalRate;
	DamageReduction = Other.DamageReduction;
	HealthSteal = Other.HealthSteal;
	MaxHealth = Other.MaxHealth;
	Perks = MoveTemp(Other.Perks);
}

FEquipmentAttributes::FEquipmentAttributes(const FEquipmentAttributes& Other)
{
	AttackPower = Other.AttackPower;
	Defense = Other.Defense;
	Penetration = Other.Penetration;
	CriticalDamage = Other.CriticalDamage;
	CriticalRate = Other.CriticalRate;
	DamageReduction = Other.DamageReduction;
	HealthSteal = Other.HealthSteal;
	MaxHealth = Other.MaxHealth;
	Perks = Other.Perks;
}

FEquipmentAttributes& FEquipmentAttributes::operator=(const FEquipmentAttributes& Other)
{
	if (this == &Other)
	{
		return *this;
	}
	AttackPower = Other.AttackPower;
	Defense = Other.Defense;
	Penetration = Other.Penetration;
	CriticalDamage = Other.CriticalDamage;
	CriticalRate = Other.CriticalRate;
	DamageReduction = Other.DamageReduction;
	HealthSteal = Other.HealthSteal;
	MaxHealth = Other.MaxHealth;
	Perks = Other.Perks;
	return *this;
}

FEquipmentAttributes& FEquipmentAttributes::operator=(FEquipmentAttributes&& Other) noexcept
{
	if (this == &Other)
	{
		return *this;
	}
		
	AttackPower = Other.AttackPower;
	MaxHealth = Other.MaxHealth;
	CriticalDamage = Other.CriticalDamage;
	CriticalRate = Other.CriticalRate;
	HealthSteal = Other.HealthSteal;
	Defense = Other.Defense;
	DamageReduction = Other.DamageReduction;
	Penetration = Other.Penetration;
	Perks = MoveTemp(Other.Perks);
	return *this;
}
