// Fill out your copyright notice in the Description page of Project Settings.

#include "SentinelTestCommandlet.h"
#include "Modules/ModuleManager.h"
#include "AssetRegistryModule.h"
#include "Engine/StaticMesh.h"


int32 USentinelTestCommandlet::Main(const FString& Params)
{
	const TCHAR* Parms = *Params;
	TArray<FString> Tokens, Switches;
	ParseCommandLine(Parms, Tokens, Switches);

	return 0;
};

/**
 * Iterate through all OBJECTS to collect information from each of them
 */
void USentinelTestCommandlet::IterateThroughObjects() {

	const FString& RelProjectDir = FPaths::ProjectDir();
	FString projectRootFolder = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*RelProjectDir);

	UE_LOG(LogTemp, Display, TEXT("Project Root: %s"), *projectRootFolder);

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> AssetData;

	AssetRegistryModule.Get().GetAllAssets(AssetData);

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	UE_LOG(LogTemp, Display, TEXT("Found number of assets: %f"), AssetData.Num());

	for (auto& data : AssetData)
	{
		//UE_LOG(LogTemp, Warning, TEXT("-----  Asset Name ----- : %s"), *data.AssetName.ToString());
		//UE_LOG(LogTemp, Warning, TEXT("-----  Asset Class----- : %s"), *data.AssetClass.ToString());

		UObject* asset = data.GetAsset();
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
				bool val = BoolProperty->GetPropertyValue((void*)asset);
				localState->SetBoolField(*PropertyName.ToString(), val);
				// UE_LOG(LogTemp, Warning, TEXT("%s -- %s"), *PropertyName.ToString(), val ? TEXT("True") : TEXT("False"));
			}

			UStrProperty* StrProperty = Cast<UStrProperty>(Property);
			if (StrProperty)
			{
				FString strValue = StrProperty->GetPropertyValue((void*)asset);
				FArchive ar;

				UE_LOG(LogTemp, Display, TEXT("String value: %s"), *strValue);

				localState->SetStringField(*PropertyName.ToString(), *strValue);
			}
		}

		FString json_string;
		TSharedRef< TJsonWriter<> > json_writer = TJsonWriterFactory<>::Create(&json_string);
		FJsonSerializer::Serialize(localState, json_writer);


		FFileHelper::SaveStringToFile(json_string, *out);

	}
};

/**
 * Iterate through all PACKAGES to collect information from each of them
 */
void USentinelTestCommandlet::IterateThroughPackages() {

	TArray<FString> FilesInPath;
	FEditorFileUtils::FindAllPackageFiles(FilesInPath);

	// Loop through all the packages
	for (int32 FileIndex = 0; FileIndex < FilesInPath.Num(); FileIndex++)
	{
		auto t = FilesInPath[FileIndex];

		FString Filename = FPaths::ConvertRelativePathToFull(FilesInPath[FileIndex]);
		{
			// reset the loaders for the packages we want to load so that we don't find the wrong version of the file
			// (otherwise, attempting to run pkginfo on e.g. Engine.xxx will always return results for Engine.u instead)
			FString PackageName;
			if (FPackageName::TryConvertFilenameToLongPackageName(Filename, PackageName))
			{
				UPackage* ExistingPackage = FindObject<UPackage>(nullptr, *PackageName, true);
				if (ExistingPackage != nullptr)
				{
					ResetLoaders(ExistingPackage);
				}
			}
		}

		FLinkerLoad* Linker = nullptr;
		UPackage* Package = nullptr;
		FArchiveStackTraceReader* Reader = nullptr;

		FString TempPackageName = Filename;
		const TCHAR* ContentFolderString = TEXT("/Content/");
		int32 ContentFolderIndex = TempPackageName.Find(ContentFolderString);

		if (ContentFolderIndex >= 0)
		{
			TempPackageName = Filename.Mid(ContentFolderIndex + FCString::Strlen(ContentFolderString));
		}
		TempPackageName = FPaths::Combine(TEXT("/Temp"), *FPaths::GetPath(TempPackageName.Mid(TempPackageName.Find(TEXT(":"), ESearchCase::CaseSensitive) + 1)), *FPaths::GetBaseFilename(TempPackageName));
		Package = FindObjectFast<UPackage>(nullptr, *TempPackageName);

		if (!Package)
		{
			Package = CreatePackage(nullptr, *TempPackageName);
		}

		Reader = FArchiveStackTraceReader::CreateFromFile(*Filename);

		if (Reader)
		{
			TGuardValue<bool> GuardAllowUnversionedContentInEditor(GAllowUnversionedContentInEditor, true);
			TGuardValue<int32> GuardAllowCookedContentInEditor(GAllowCookedDataInEditorBuilds, 1);
			UPackage* LoadedPackage = LoadPackage(Package, *Filename, LOAD_NoVerify, Reader);
			if (LoadedPackage)
			{
				check(LoadedPackage == Package);
				Linker = Package->LinkerLoad;
				check(Linker);
				UE_LOG(LogTemp, Display, TEXT("String value: %s"), *Linker->Filename);
				SentinelPackageReport(Linker);

			}
		}
	}

	/*
	if (FParse::Value(*Params, TEXT("dumptofile="), OutputFilename))
	{
		OutputOverride = new FOutputDeviceFile(*OutputFilename, true);
	}
	*/
};

void USentinelTestCommandlet::SentinelPackageReport(FLinkerLoad* Linker)
{
	for (int32 i = 0; i < Linker->ImportMap.Num(); ++i)
	{
		FObjectImport& import = Linker->ImportMap[i];

		FName PackageName = NAME_None;
		FName OuterName = NAME_None;

		if (!import.OuterIndex.IsNull())
		{
			// OuterName = Linker->ImpExp(import.OuterIndex).ObjectName;
			
			OuterName = *Linker->GetPathName(import.OuterIndex);

			// Find the package which contains this import.  import.SourceLinker is cleared in EndLoad, so we'll need to do this manually now.
			FPackageIndex OutermostLinkerIndex = import.OuterIndex;
			for (FPackageIndex LinkerIndex = import.OuterIndex; !LinkerIndex.IsNull(); )
			{
				OutermostLinkerIndex = LinkerIndex;
				LinkerIndex = Linker->ImpExp(LinkerIndex).OuterIndex;
			}
			check(!OutermostLinkerIndex.IsNull());

			PackageName = Linker->ImpExp(OutermostLinkerIndex).ObjectName;
			UE_LOG(LogTemp, Display, TEXT("String value: %s"), *PackageName.ToString());
			
		}
	}
};