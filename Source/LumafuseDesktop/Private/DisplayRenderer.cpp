// Fill out your copyright notice in the Description page of Project Settings.


#include "DisplayRenderer.h"

#include "Materials/MaterialInstanceDynamic.h"
#include "DatasmithDefinitions.h"
#include "Components/SceneCaptureComponent2D.h"

// Sets default values
ADisplayRenderer::ADisplayRenderer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	DisplayPlaneInner = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DisplayPlane_Inner"));
	DisplayPlaneMiddle = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DisplayPlane_Middle"));
	DisplayPlaneOuter = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DisplayPlane_Outer"));
	DisplayCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("DisplayCapture"));

	Scene->SetupAttachment(GetRootComponent());
	DisplayPlaneInner->SetupAttachment(Scene);
	DisplayPlaneMiddle->SetupAttachment(Scene);
	DisplayPlaneOuter->SetupAttachment(Scene);
	DisplayCapture->SetupAttachment(Scene);
	
}

// Called when the game starts or when spawned
void ADisplayRenderer::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADisplayRenderer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}