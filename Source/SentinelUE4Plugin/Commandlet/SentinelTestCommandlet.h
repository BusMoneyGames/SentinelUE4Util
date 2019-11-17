// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "SharedPointer.h"
#include "Json.h"
#include "FileHelpers.h"
#include "Serialization/ArchiveStackTrace.h"

#include "SentinelTestCommandlet.generated.h"

/**
 * 
 */
UCLASS()
class SENTINELUE4PLUGIN_API USentinelTestCommandlet : public UCommandlet
{
	GENERATED_BODY()

	public:
		TSharedRef<FJsonObject> localState = MakeShareable(new FJsonObject);

		virtual int32 Main(const FString& Params) override;
		void IterateThroughPackages();
		void IterateThroughObjects();
		void SentinelPackageReport(FLinkerLoad* InLinker);


};
