// ReSharper disable CppExpressionWithoutSideEffects
#include "PDI/PlayerDataInterface.h"

bool IsLocal()
{
	FString PDI;
	GConfig->GetString(TEXT("MySettings"), TEXT("PDI"), PDI, GEngineIni);
	return PDI.Equals(PDI_LOCAL, ESearchCase::IgnoreCase);
}

bool FPlayerDataInterface::IsLocalInterface()
{
	static bool IsLocalPDI = IsLocal();
	return IsLocalPDI;
}
