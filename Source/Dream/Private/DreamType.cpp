// Fill out your copyright notice in the Description page of Project Settings.


#include "DreamType.h"

DEFINE_LOG_CATEGORY(LogDream)

const FName DreamActorTagName::Teammate = TEXT("Teammate");
const FName DreamActorTagName::Enemy = TEXT("Enemy");
const FName DreamActorTagName::Death = TEXT("Death");


uint32 IPTools::IPV4StringToUint32(const FString& IpString)
{
	TArray<FString> IPSegment;
	IpString.ParseIntoArray(IPSegment, TEXT("."));

	uint32 ServerIP = 0;
	ServerIP |= FCString::Atoi(*IPSegment[3]) & 255;
	ServerIP |= (FCString::Atoi(*IPSegment[2]) & 255) << 8;
	ServerIP |= (FCString::Atoi(*IPSegment[1]) & 255) << 16;
	ServerIP |= (FCString::Atoi(*IPSegment[0]) & 255) << 24;
    
	return ServerIP;
}

FString IPTools::IPV4Uint32ToString(uint32 IP)
{
	FString ServerAddr;
	ServerAddr.AppendInt(((IP >> 24) & 255));
	ServerAddr.AppendChar(TEXT('.'));
	ServerAddr.AppendInt(((IP >> 16) & 255));
	ServerAddr.AppendChar(TEXT('.'));
	ServerAddr.AppendInt(((IP >> 8) & 255));
	ServerAddr.AppendChar(TEXT('.'));
	ServerAddr.AppendInt((IP & 255));
	return ServerAddr;
}
