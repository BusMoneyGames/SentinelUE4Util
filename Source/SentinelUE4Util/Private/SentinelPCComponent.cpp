// Fill out your copyright notice in the Description page of Project Settings.


#include "SentinelPCComponent.h"


// Sets default values for this component's properties
USentinelPCComponent::USentinelPCComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void USentinelPCComponent::BeginPlay()
{
	Super::BeginPlay();

	OwningPlayerController = Cast<APlayerController>(GetOwner());

	viewmodes.Add("ShowFlag.LightMapDensity");
	//viewmodes.Add("ShowFlag.ShaderComplexity");
	viewmodes.Add("ShowFlag.LODColoration");
}

// Called every frame
void USentinelPCComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// GPU Capture
	if (should_gpu_capture && OutputOverride)
	{
		profileGPUFrameCounter++;

		if (profileGPUFrameCounter > 5)
		{
			OutputOverride->TearDown();
			GLog->RemoveOutputDevice(OutputOverride);
			should_gpu_capture = false;
			should_capture_viewmodes = true;
		}

		return;
	}

	// View modes capture
	if (should_capture_viewmodes) {

		DisableLastViewmode();
		SetNextViewmode();
		TriggerScreenshot(viewmodes[viewmode_index]);

		last_frame_viewmode = viewmodes[viewmode_index];
		viewmode_index++;
	}

	if (viewmode_index >= viewmodes.Num())
	{
		FinishProfiling();
	}
}

void USentinelPCComponent::FinishProfiling()
{
	DisableLastViewmode();

	viewmode_index = 0;
	should_capture_viewmodes = false;
	isProfiling = false;
	profileGPUFrameCounter = 0;

	onCaptureFinished.Broadcast();

}

void USentinelPCComponent::SetNextViewmode()
{
	OwningPlayerController->ConsoleCommand(viewmodes[viewmode_index] + " 1");
}

void USentinelPCComponent::DisableLastViewmode()
{

	OwningPlayerController->ConsoleCommand(last_frame_viewmode + " 0");
}

FString USentinelPCComponent::GetTestOutputFolder() {

	// TODO Make this function handle not overwriting test results

	// TODO Make the string combine platform safe
	FString path = SentinelRelativePath + "/" + FString::FromInt(testIterator) + "_" + TestName + "/";
	return IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*path);
}

void USentinelPCComponent::TriggerScreenshot(FString viewmode) {

	// Take screenshot
	FScreenshotRequest::RequestScreenshot(*GetTestOutputFolder() + viewmode + ".png", false, false);
}

void USentinelPCComponent::SaveTextureData() {

	// Create output device and add the include filter for the category we want
	FString LogOutput = GetTestOutputFolder() + "Texture_Data.log";

	FOutputDeviceFile* TextureOutputDevice = new FOutputDeviceFile(*LogOutput, true);;

	TextureOutputDevice = new FOutputDeviceFile(*LogOutput, true);

	// Attach the logger to the output device
	GLog->AddOutputDevice(TextureOutputDevice);

	// Send console commands
	OwningPlayerController->ConsoleCommand("listtextures");

	GLog->RemoveOutputDevice(TextureOutputDevice);
	TextureOutputDevice->TearDown();
}

void USentinelPCComponent::CaptureGPUData(FString TestID = "DefaultTest")
{
	testIterator = testIterator + 1;

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
	FString LogOutput = GetTestOutputFolder() + "GPU_Data.log";

	OutputOverride = new FOutputDeviceFile(*LogOutput, true);
	OutputOverride->IncludeCategory(LogRHICategory);

	// Attach the logger to the output device
	GLog->AddOutputDevice(OutputOverride);

	// Send console commands
	OwningPlayerController->ConsoleCommand(profileGPUSettings);
	OwningPlayerController->ConsoleCommand(profileGPUCommand);

	// Reset the frame counter to turn off the log capture after a fixed number of frames
	profileGPUFrameCounter = 0;

}

