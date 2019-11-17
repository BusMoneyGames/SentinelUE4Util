// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "GenericPlatform/GenericPlatformApplicationMisc.h"
#include "HAL/PlatformOutputDevices.h"
#include "GameFramework/PlayerController.h"
#include "Misc/FeedbackContext.h"
#include "Misc/OutputDeviceFile.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"
#include "HAL/FileManager.h"
#include "SentinelProfiler.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SENTINELUE4PLUGIN_API USentinelProfiler : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USentinelProfiler();

	UFUNCTION(BlueprintCallable, Category = "Profiling")
	void SaveGPUData(FString TestName);

	void SaveTextureData(FString TestName);
	FString GetTestOutputString();

	FOutputDeviceFile* OutputOverride;

	APlayerController* OwningPlayerController;
	int TestNameIterator = 0;
	int profileGPUFrameCounter = 0;

	FString profileGPUSettings = "r.profileGPU.showUI 0 | r.ProfileGPU.PrintAssetSummary 1 | r.ShowMaterialDrawEvents 1 | r.ProfileGPU.PrintAssetSummary 1";
	FString profileGPUCommand = "profileGPU";
	FName LogRHICategory = "LogRHI";

	const FString& SentinelRelativePath = FPaths::ProjectSavedDir() + "SentinelOutput/";

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
