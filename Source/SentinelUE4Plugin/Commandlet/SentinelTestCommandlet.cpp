// Fill out your copyright notice in the Description page of Project Settings.

#include "SentinelTestCommandlet.h"
#include "Modules/ModuleManager.h"
#include "AssetRegistryModule.h"
#include "Engine/StaticMesh.h"


int32 USentinelTestCommandlet::Main(const FString& Params) 
{


	const FString& RelProjectDir = FPaths::ProjectDir();
	FString projectRootFolder = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*RelProjectDir);

	UE_LOG(LogTemp, Display, TEXT("Found number of assets: %s"), *projectRootFolder);

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> AssetData;
	
	AssetRegistryModule.Get().GetAllAssets(AssetData);

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	UE_LOG(LogTemp, Display, TEXT("Found number of assets: %f"), AssetData.Num());

	for (auto& data : AssetData)
	{
		//UE_LOG(LogTemp, Warning, TEXT("-----  Asset Name ----- : %s"), *data.AssetName.ToString());
		//UE_LOG(LogTemp, Warning, TEXT("-----  Asset Class----- : %s"), *data.AssetClass.ToString());

		UObject *asset = data.GetAsset();
		auto myClass = asset->GetClass();
		FString out = projectRootFolder + "Data/" + *data.AssetName.ToString() + ".json";
		for (TFieldIterator<UProperty> PropIt(myClass); PropIt; ++PropIt)
		{
			UProperty* Property = *PropIt;
			FName const PropertyName = Property->GetFName();
			auto type = Property->GetClass();

			UBoolProperty* BoolProperty = Cast<UBoolProperty>(Property);
			if (BoolProperty)
			{
				
				bool val = BoolProperty->GetPropertyValue((void*) asset);
				localState->SetBoolField(*PropertyName.ToString(), val);
				// UE_LOG(LogTemp, Warning, TEXT("%s -- %s"), *PropertyName.ToString(), val ? TEXT("True") : TEXT("False"));

			}
		}
		FString json_string;
		TSharedRef< TJsonWriter<> > json_writer = TJsonWriterFactory<>::Create(&json_string);
		FJsonSerializer::Serialize(localState, json_writer);

		FFileHelper::SaveStringToFile(json_string, *out);


	}

	return 0;
}