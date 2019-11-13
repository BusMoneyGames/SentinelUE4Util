// Fill out your copyright notice in the Description page of Project Settings.

#include "SentinelTestCommandlet.h"
#include "Modules/ModuleManager.h"
#include "AssetRegistryModule.h"
#include "Engine/StaticMesh.h"


int32 USentinelTestCommandlet::Main(const FString& Params)
{

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> AssetData;
	
	AssetRegistryModule.Get().GetAllAssets(AssetData);

	UE_LOG(LogTemp, Display, TEXT("Found number of assets: %f"), AssetData.Num());

	for (auto& data : AssetData)
	{
		UE_LOG(LogTemp, Warning, TEXT("-----  Asset Name ----- : %s"), *data.AssetName.ToString());
		UE_LOG(LogTemp, Warning, TEXT("-----  Asset Class----- : %s"), *data.AssetClass.ToString());

		UObject *asset = data.GetAsset();
		auto myClass = asset->GetClass();

		for (TFieldIterator<UProperty> PropIt(myClass); PropIt; ++PropIt)
		{
			UProperty* Property = *PropIt;
			FName const PropertyName = Property->GetFName();
			auto type = Property->GetClass();

			UBoolProperty* BoolProperty = Cast<UBoolProperty>(Property);
			if (BoolProperty)
			{
				
				bool val = BoolProperty->GetPropertyValue((void*) asset);				
				UE_LOG(LogTemp, Warning, TEXT("%s -- %s"), *PropertyName.ToString(), val ? TEXT("True") : TEXT("False"));
			}
		}

	}

	// UE_LOG(LogTemp, Warning, TEXT("%d number of meshes in the project"), AssetData.Num());

	return 0;
}