#include "SlateUtils.h"

UMaterialInstanceDynamic* SlateUtils::GetDynamicFromBrush(FSlateBrush& Brush, UObject* Outer)
{
	UMaterialInstanceDynamic* Dynamic = nullptr;
	
	UObject* MagazineResource = Brush.GetResourceObject();
	if (MagazineResource && MagazineResource->IsA<UMaterialInterface>())
	{
		if (UMaterialInstanceDynamic* DynamicResource = Cast<UMaterialInstanceDynamic>(MagazineResource))
		{
			Dynamic = DynamicResource;
		}
		else
		{
			Dynamic = UMaterialInstanceDynamic::Create(Cast<UMaterialInterface>(MagazineResource), Outer);
			Brush.SetResourceObject(Dynamic);
		}
	}

	return Dynamic;
}