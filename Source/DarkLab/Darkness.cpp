// Fill out your copyright notice in the Description page of Project Settings.

#include "Darkness.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "DarknessController.h"
#include "MainCharacter.h"
#include "MainGameMode.h"

// Movement functions
void ADarkness::Move(const FVector direction)
{
	UE_LOG(LogTemp, Warning, TEXT("Darkness::Move"));

	// TODO delete, this is only used for debug to prevent darkness from moving
	if (bIsStationary)
		return;

	// Moves slower in light, but light resistance helps
	// float temp = 1 - LightFearK * Luminosity * FMath::Max(0.0f, Luminosity - LightResistance);
	float temp = 1 - 2 * FMath::Max(0.f, Luminosity - LightResistance / 2.f) / (LightResistance + 0.2f);
	Movement->AddInputVector(direction * FMath::Max(0.0f, temp));
}
void ADarkness::MoveToLocation(FVector location)
{
	TrackedLocation = location;
	TrackingType = ETrackingEnum::VE_Location;
}
void ADarkness::MoveToActor(AActor* actor)
{
	TrackedActor = actor;
	TrackingType = ETrackingEnum::VE_Actor;
}
void ADarkness::Stop()
{
	TrackingType = ETrackingEnum::VE_None;
}
void ADarkness::TeleportToLocation(FVector location)
{
	UE_LOG(LogTemp, Warning, TEXT("Darkness::TeleportToLocation"));

	DarkParticles->SetEmitterEnable(FName("Darkness"), false);
	SetActorLocation(location);

	// Reenables particles after a delay
	FTimerHandle handler;
	((AActor*)this)->GetWorldTimerManager().SetTimer(handler, this, &ADarkness::ReenableParticles, 1.0f, false, ReenableParticlesAfterTeleportDelay);
}
// When light is too strong goes backwards and returns true. Great for some situations and will look weird in others. Returns false if light aint too strong
bool ADarkness::RetreatFromLight()
{
	UE_LOG(LogTemp, Warning, TEXT("Darkness::RetreatFromLight"));

	// Retreats faster in brighter light, but resistance helps
	// float temp = 1 - LightFearK * Luminosity * FMath::Max(0.0f, Luminosity - LightResistance);
	// float temp = 1 - Luminosity / (LightResistance + 0.1f);
	float temp = 1 - 2 * FMath::Max(0.f, Luminosity - LightResistance / 2.f) / (LightResistance + 0.2f);
	if (temp >= 0)
		return false; // Doesn't retreat
	
	// Away from brightest light
	FVector fleeDirection = GetActorLocation() - BrightestLightLocation;
	fleeDirection.Normalize();
	Movement->AddInputVector(fleeDirection * temp * -1); // temo is lower than 0 at this point
	
	return true; // Does retreat
}
// Track something
void ADarkness::Tracking()
{
	UE_LOG(LogTemp, Warning, TEXT("Darkness::Tracking"));

	FVector currentLocation;
	switch (TrackingType)
	{
	case ETrackingEnum::VE_None:
		return;
	case ETrackingEnum::VE_Location:
		currentLocation = TrackedLocation;
		break;
	case ETrackingEnum::VE_Actor:
		if (!TrackedActor)
			return;
		currentLocation = TrackedActor->GetActorLocation();
		break;
	}

	FVector direction = currentLocation - GetActorLocation();

	// We don't move if objects are already close
	// TODO delete magic number
	if (direction.Size() < 20.0f)
		return;

	direction.Normalize();

	Move(direction);
}
// Goes away from last brightest light
void ADarkness::IntoDarkness()
{
	// Away from brightest light
	FVector fleeDirection = GetActorLocation() - BrightestLightLocation;
	fleeDirection.Normalize();
	Move(fleeDirection);
}

// Reenables particles
void ADarkness::ReenableParticles()
{
	DarkParticles->SetEmitterEnable(FName("Darkness"), true);
}

// Used for collision overlaps
void ADarkness::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	// UE_LOG(LogTemp, Warning, TEXT("Entered the darkness!"));
	
	// Darkness is harmless when it isn't hunting
	if (DarknessController->State != EDarkStateEnum::VE_Hunting)
		return;

	AMainCharacter* character = Cast<AMainCharacter>(OtherActor);
	if (!character)
		return;

	character->Disable();

	// Tell the controller that darkness disabled player
	DarknessController->OnDisabling();
}
void ADarkness::OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex)
{
	// UE_LOG(LogTemp, Warning, TEXT("Escaped the darkness!"));

	// TODO
}

// Sets default values
ADarkness::ADarkness()
{
	// Create a particle system
	DarkParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("DarkParticles"));
	SetRootComponent(DarkParticles);

	// Create a sphere for collision
	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	Collision->SetupAttachment(RootComponent);
	Collision->OnComponentBeginOverlap.AddDynamic(this, &ADarkness::OnBeginOverlap);
	Collision->OnComponentEndOverlap.AddDynamic(this, &ADarkness::OnEndOverlap);

	// Create a floating movement
	Movement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("FloatingMovement"));

 	// Set this pawn to call Tick() every frame
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ADarkness::BeginPlay()
{
	Super::BeginPlay();

	if (!bIsActive)
		return;

	GameMode = Cast<AMainGameMode>(GetWorld()->GetAuthGameMode());
	DarknessController = Cast<ADarknessController>(GetController());
}

// Called every frame
void ADarkness::Tick(const float deltaTime)
{
	UE_LOG(LogTemp, Warning, TEXT("Darkness::Tick"));

	Super::Tick(deltaTime);

	if (!bIsActive)
		return;

	// We check the light level	
	Luminosity = GameMode->GetLightingAmount(BrightestLightLocation, this, true, Collision->GetScaledSphereRadius() + 30, true); // , false, bShowLightDebug);

	// Calculate time in darkness
	if (Luminosity > 0)
		TimeInDark = 0.f;
	else
		TimeInDark += deltaTime;

	// Increase resistance if stuck in bright light
	if (Luminosity > LightResistance)
		LightResistance += deltaTime * LightResGainSpeed;
	// Or decrease it if in no light at all
	else if (Luminosity <= 0.0f && LightResistance > 0.0f)
		LightResistance -= deltaTime * LightResLossSpeed;
	if (LightResistance < 0.0f)
		LightResistance = 0.0f;
}