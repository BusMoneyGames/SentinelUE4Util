// Fill out your copyright notice in the Description page of Project Settings.

#include "SentinelProfiler.h"

// Sets default values for this component's properties
USentinelProfiler::USentinelProfiler()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void USentinelProfiler::BeginPlay()
{
	Super::BeginPlay();

	OwningPlayerController = Cast<APlayerController>(GetOwner());
	
	// ...
	UE_LOG(LogTemp, Warning, TEXT("Starting console command trigger"));

}


// Called every frame
void USentinelProfiler::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (profileGPUFrameCounter == 5 && OutputOverride) {
		OutputOverride->TearDown();
	}
	else { 
		profileGPUFrameCounter++;
	}
}

FString USentinelProfiler::GetTestOutputString() {

	// TODO Make this function handle not overwriting test results

	return IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*SentinelRelativePath);
}

void USentinelProfiler::SaveTextureData(FString TestName = "DefaultTest") {
	// Create output device and add the include filter for the category we want
	FString LogOutput = GetTestOutputString() + TestName + "_texture.log";

	FOutputDeviceFile* TextureOutputDevice = new FOutputDeviceFile(*LogOutput, true);;

	TextureOutputDevice = new FOutputDeviceFile(*LogOutput, true);

	// Attach the logger to the output device
	GLog->AddOutputDevice(TextureOutputDevice);

	// Send console commands
	OwningPlayerController->ConsoleCommand("listtextures");

	TestNameIterator = TestNameIterator + 1;

	GLog->RemoveOutputDevice(TextureOutputDevice);
}

void USentinelProfiler::SaveGPUData(FString TestName="DefaultTest")
{
	/*
	r.ProfileGPU.AssetSummaryCallOuts
	r.ProfileGPU.Pattern
	r.ProfileGPU.PrintAssetSummary

	*/
	// Saving the texture data
	SaveTextureData(TestName);

	// Create output device and add the include filter for the category we want
	FString LogOutput = GetTestOutputString() + TestName + "_gpu.log";

	OutputOverride = new FOutputDeviceFile(*LogOutput, true);
	OutputOverride->IncludeCategory(LogRHICategory);

	// Attach the logger to the output device
	GLog->AddOutputDevice(OutputOverride);

	// Send console commands
	OwningPlayerController->ConsoleCommand(profileGPUSettings);
	OwningPlayerController->ConsoleCommand(profileGPUCommand);
	
	// Take screenshot
	FScreenshotRequest::RequestScreenshot(*GetTestOutputString() + TestName + ".png", false, false);

	// Reset the frame counter to turn off the log capture after a fixed number of frames
	profileGPUFrameCounter = 0;

	TestNameIterator = TestNameIterator + 1;

}

