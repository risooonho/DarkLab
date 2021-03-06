// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Darkness.generated.h"

// Tracking type
UENUM(BlueprintType)
enum class ETrackingEnum : uint8
{
	VE_None 	UMETA(DisplayName = "None"),
	VE_Actor 	UMETA(DisplayName = "Actor"),
	VE_Location	UMETA(DisplayName = "Location")
};

// The darkness that hunts the player
UCLASS(Blueprintable)
class DARKLAB_API ADarkness : public APawn
{
	GENERATED_BODY()

public:
	// Movement 
	void Move(const FVector direction);
	void MoveToLocation(FVector location);
	void MoveToActor(AActor* actor);
	void Stop();
	void TeleportToLocation(FVector locaton);
	// When light is too strong goes backwards and returns true. Great for some situations and will look weird in others. Returns false if light ain't too strong
	bool RetreatFromLight();
	// Tracks something
	void Tracking();
	// Goes away from last brightest light
	void IntoDarkness();

	// Called after player finds black card
	UFUNCTION(BlueprintImplementableEvent, Category = "Darkness")
	void OnEnraged();

private:
	// Reenables particles
	void ReenableParticles();
	float ReenableParticlesAfterTeleportDelay = 2.f;

public:
	// TODO delete, this is only used for debug
	UPROPERTY(EditAnywhere, Category = "Darkness: Debug")
	bool bIsStationary = false;

	// True if
	UPROPERTY(EditAnywhere, Category = "Darkness")
	bool bIsActive = true;

protected:
	// Tracking parameters
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Darkness: Tracking")
	ETrackingEnum TrackingType = ETrackingEnum::VE_None;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Darkness: Tracking")
	AActor* TrackedActor;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Darkness: Tracking")
	FVector TrackedLocation;

public:
	// The amount of light the darkness is in
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Darkness: Luminosity")
	float Luminosity = 0.0f;
	// The darkness's resistance to light
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Darkness: Luminosity")
	float LightResistance = 0.0f;
	// Current time spent in the darkness
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Darkness: Luminosity")
	float TimeInDark = 0.f;

protected:
	// The amount of light the darkness is in
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Darkness: Luminosity")
	FVector BrightestLightLocation;
	// The speed of resistance rising if in light higher than resistance
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Darkness: Luminosity")
	float LightResGainSpeed = 0.007f;
	// The speed of resistance dropping while not in any light
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Darkness: Luminosity")
	float LightResLossSpeed = 0.003f;
	// The coefficient affecting darkness's reaction to light
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Darkness: Luminosity")
	float LightFearK = 10.0f;

	// The particle system, forming the main body of the darkness
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Darkness: Components")
	class UParticleSystemComponent* DarkParticles;
	// The spherical collision of the darkness
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Darkness: Components")
	class USphereComponent* Collision;
	// The movement component of the darkness
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Darkness: Components")
	class UFloatingPawnMovement* Movement;

private:
	// A pointer to the game mode
	UPROPERTY()
	class AMainGameMode* GameMode;
	// A pointer to the controller
	UPROPERTY()
	class ADarknessController* DarknessController;

public:
	// Used for the collision overlaps
	UFUNCTION(BlueprintCallable, Category = "Darkness: Overlap")
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	UFUNCTION(BlueprintCallable, Category = "Darkness: Overlap")
	void OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:
	// Sets default values
	ADarkness();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(const float deltaTime) override;
};