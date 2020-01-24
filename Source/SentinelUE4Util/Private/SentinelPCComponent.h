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
#include "Misc/AutomationTest.h"
#include "SentinelPCComponent.generated.h"


DECLARE_DELEGATE(FGPUCaptureFinishedDelegate);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SENTINELUE4UTIL_API USentinelPCComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USentinelPCComponent();


	UFUNCTION()
	void CaptureGPUData(FString TestID);

	void TriggerScreenshot(FString viewmode);
	void SaveTextureData();
	void DisableLastViewmode();
	void SetNextViewmode();
	void FinishProfiling();

	FString GetTestOutputFolder();

	APlayerController* OwningPlayerController;
	FOutputDeviceFile* OutputOverride;

	bool isProfiling = false;
	int testIterator = 0;
	int profileGPUFrameCounter = 0;
	int number_of_frames_to_capture = 5;
	bool should_gpu_capture = false;

	FString TestName;
	TArray<FString> viewmodes;
	const FString& SentinelRelativePath = FPaths::ProjectSavedDir() + "SentinelOutput/";
	bool should_capture_viewmodes = false;
	int viewmode_index = 0;
	FString last_frame_viewmode = "";
	FString profileGPUSettings = "r.profileGPU.showUI 0 | r.ProfileGPU.PrintAssetSummary 1 | r.ShowMaterialDrawEvents 1 | r.ProfileGPU.PrintAssetSummary 1";
	FString profileGPUCommand = "profileGPU";
	FName LogRHICategory = "LogRHI";

	FGPUCaptureFinishedDelegate onCaptureFinished;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
