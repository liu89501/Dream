// ReSharper disable CppSomeObjectMembersMightNotBeInitialized
#include "DPropsType.h"
#include "DBaseAttributesAsset.h"

const FPropsInfo FEmptyStruct::EmptyPropsInfo;
const FEquipmentAttributes FEmptyStruct::EmptyAttributes;
const FSoftClassPath FEmptyStruct::EmptySoftClassPath;
const FItemDef FEmptyStruct::EmptyItemDef;

UClass* FItemDef::GetItemClass() const
{
	UClass* ItemUClass = ItemClass.ResolveClass();

	if (ItemUClass == nullptr)
	{
		ItemUClass = ItemClass.TryLoadClass<UObject>();
	}

	return ItemUClass;
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

FEquipmentAttributes& FEquipmentAttributes::operator+=(FEquipmentAttributes&& RHS)
{
	AttackPower += RHS.AttackPower;
	Defense += RHS.Defense;
	Penetration += RHS.Penetration;
	CriticalDamage += RHS.CriticalDamage;
	CriticalRate += RHS.CriticalRate;
	DamageReduction += RHS.DamageReduction;
	HealthSteal += RHS.HealthSteal;
	MaxHealth += RHS.MaxHealth;
	Perks += MoveTemp(RHS.Perks);
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

FEquipmentAttributes& FEquipmentAttributes::MergeAndSkipPerks(const FEquipmentAttributes& Other)
{
	AttackPower += Other.AttackPower;
	Defense += Other.Defense;
	Penetration += Other.Penetration;
	CriticalDamage += Other.CriticalDamage;
	CriticalRate += Other.CriticalRate;
	DamageReduction += Other.DamageReduction;
	HealthSteal += Other.HealthSteal;
	MaxHealth += Other.MaxHealth;
	return *this;
}

bool FEquipmentAttributes::IsValidNumericalValue(float ErrorTolerance) const
{
	return AttackPower > ErrorTolerance ||
		Defense > ErrorTolerance ||
		Penetration > ErrorTolerance ||
		CriticalDamage > ErrorTolerance ||
		CriticalRate > ErrorTolerance ||
		DamageReduction > ErrorTolerance ||
		HealthSteal > ErrorTolerance ||
		MaxHealth > ErrorTolerance;
}

bool FAttributeHandle::operator==(const FAttributeHandle& Other) const
{
	return AttributeProperty == Other.AttributeProperty;
}

bool FAttributeHandle::operator!=(const FAttributeHandle& Other) const
{
	return !(*this == Other);
}

bool FEquipmentAttributes::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	Ar << AttackPower;
	Ar << MaxHealth;

	if (Ar.IsLoading())
	{
		ReadFixedCompressedFloat<10, 16>(CriticalDamage, Ar);
		ReadFixedCompressedFloat<1, 16>(CriticalRate, Ar);
		ReadFixedCompressedFloat<1, 16>(HealthSteal, Ar);
	}
	else
	{
		bOutSuccess &= WriteFixedCompressedFloat<10, 16>(CriticalDamage, Ar);
		bOutSuccess &= WriteFixedCompressedFloat<1, 16>(CriticalRate, Ar);
		bOutSuccess &= WriteFixedCompressedFloat<1, 16>(HealthSteal, Ar);
	}

	Ar << Defense;

	SafeNetSerializeTArray_Default<8>(Ar, Perks);

	return true;
}

FArchive& operator<<(FArchive& Ar, FEquipmentAttributes& Attr)
{
	Ar << Attr.AttackPower;
	Ar << Attr.MaxHealth;
	Ar << Attr.CriticalDamage;
	Ar << Attr.CriticalRate;
	Ar << Attr.HealthSteal;
	Ar << Attr.Defense;
	Ar << Attr.DamageReduction;
	Ar << Attr.Penetration;
	Ar << Attr.Perks;
	return Ar;
}

uint32 GetTypeHash(const FAttributeHandle& Other)
{
	return PointerHash(Other.AttributeProperty.Get());
}
