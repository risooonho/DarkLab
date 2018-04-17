// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Darkness.generated.h"

// The darkness that hunts the player
UCLASS(Blueprintable)
class DARKLAB_API ADarkness : public APawn
{
	GENERATED_BODY()

private:
	// The particle system, forming the main body of the Darkness
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Darkness", meta = (AllowPrivateAccess = "true"))
	class UParticleSystemComponent* DarkParticles;

	// The spherical collision of the darkness
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Darkness", meta = (AllowPrivateAccess = "true"))
	class USphereComponent* Collision;

public:
	// Sets default values
	ADarkness();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};