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

	viewmodes.Add("ShowFlag.LightMapDensity");
	viewmodes.Add("ShowFlag.ShaderComplexity");
	viewmodes.Add("ShowFlag.LODColoration");
	
	// ...
	UE_LOG(LogTemp, Warning, TEXT("Starting console command trigger"));

}


// Called every frame
void USentinelProfiler::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (should_gpu_capture && profileGPUFrameCounter == 5 && OutputOverride) {
		OutputOverride->TearDown();
		should_gpu_capture = false;

		should_capture_viewmodes = true;
	}
	else { 
		profileGPUFrameCounter++;
	}

	if (should_capture_viewmodes && viewmodes.Num() > viewmode_index) {

		OwningPlayerController->ConsoleCommand(last_frame_viewmode + " 0");
		OwningPlayerController->ConsoleCommand(viewmodes[viewmode_index] + " 1");

		ScreenshotViewmode(viewmodes[viewmode_index]);
		last_frame_viewmode = viewmodes[viewmode_index];

		viewmode_index++;
	}
	else {
		OwningPlayerController->ConsoleCommand(last_frame_viewmode + " 0");
		viewmode_index = 0;
		should_capture_viewmodes = false;
		isProfiling = false;
	}
}

FString USentinelProfiler::GetTestOutputString() {

	// TODO Make this function handle not overwriting test results

	// TODO Make the string combine platform safe
	FString path = SentinelRelativePath + "/" + FString::FromInt(testIterator) + "_" + TestName + "/";
	return IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*path);
}

void USentinelProfiler::ScreenshotViewmode(FString viewmode) {

	// Take screenshot
	FScreenshotRequest::RequestScreenshot(*GetTestOutputString() + viewmode + ".png", false, false);

}

void USentinelProfiler::SaveTextureData() {

	// Create output device and add the include filter for the category we want
	FString LogOutput = GetTestOutputString() + "Texture_Data.log";

	FOutputDeviceFile* TextureOutputDevice = new FOutputDeviceFile(*LogOutput, true);;

	TextureOutputDevice = new FOutputDeviceFile(*LogOutput, true);

	// Attach the logger to the output device
	GLog->AddOutputDevice(TextureOutputDevice);

	// Send console commands
	OwningPlayerController->ConsoleCommand("listtextures");

	GLog->RemoveOutputDevice(TextureOutputDevice);
	TextureOutputDevice->TearDown();
}

void USentinelProfiler::CaptureGPUData(FString TestID="DefaultTest")
{
	TestName = TestID;
	should_gpu_capture = true;
	isProfiling = true;

	/*
	r.ProfileGPU.AssetSummaryCallOuts
	r.ProfileGPU.Pattern
	r.ProfileGPU.PrintAssetSummary

	*/

	// Saving the texture data
	SaveTextureData();
	
	// Create output device and add the include filter for the category we want
	FString LogOutput = GetTestOutputString() + "GPU_Data.log";

	OutputOverride = new FOutputDeviceFile(*LogOutput, true);
	OutputOverride->IncludeCategory(LogRHICategory);

	// Attach the logger to the output device
	GLog->AddOutputDevice(OutputOverride);

	// Send console commands
	OwningPlayerController->ConsoleCommand(profileGPUSettings);
	OwningPlayerController->ConsoleCommand(profileGPUCommand);

	// Reset the frame counter to turn off the log capture after a fixed number of frames
	profileGPUFrameCounter = 0;

	testIterator = testIterator + 1;

}

