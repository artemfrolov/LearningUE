// Copyright Epic Games, Inc. All Rights Reserved.

#include "LearningUECharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "LearningUE.h"
#include "StatsComponent.h"

ALearningUECharacter::ALearningUECharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Create the stats component. No SetupAttachment call: a UActorComponent has no
	// transform, so there is nothing to attach it to - it just belongs to this actor.
	Stats = CreateDefaultSubobject<UStatsComponent>(TEXT("Stats"));

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void ALearningUECharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ALearningUECharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ALearningUECharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ALearningUECharacter::Look);

		// Sprinting
		// UE idiom: one action, two events - key down and key up drive separate functions
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ALearningUECharacter::SprintStart);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ALearningUECharacter::SprintEnd);

		// Dodging
		// only Started - a dodge is a one-shot, not something you hold
		EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Started, this, &ALearningUECharacter::Dodge);
	}
	else
	{
		UE_LOG(LogLearningUE, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ALearningUECharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void ALearningUECharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void ALearningUECharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void ALearningUECharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void ALearningUECharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void ALearningUECharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

void ALearningUECharacter::SprintStart()
{
	// raise the movement component's speed cap for as long as the key is held
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}

void ALearningUECharacter::SprintEnd()
{
	// restore the normal cap
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void ALearningUECharacter::Dodge()
{
	// a dodge is a grounded move: in the air there is no ground friction to decay
	// the burst, and zeroing vertical velocity turns a jump into a glide
	if (GetCharacterMovement()->IsFalling())
	{
		return;
	}

	// refuse if the cooldown has not elapsed yet
	const float Now = GetWorld()->GetTimeSeconds();

	if (Now - LastDodgeTime < DodgeCooldown)
	{
		return;
	}

	// last of the three refusals, and the only one that costs something to ask:
	// TryConsume spends on success, so nothing below this line may fail
	if (!Stats->TryConsumeStamina(DodgeStaminaCost))
	{
		return;
	}

	// the dodge is committed now - record it and go
	LastDodgeTime = Now;

	// dodge where the player is steering; sidestep right when standing still
	FVector Direction = GetLastMovementInputVector();

	if (Direction.IsNearlyZero())
	{
		Direction = GetActorRightVector();
	}

	// UE idiom: normalise before scaling, so a half-pushed stick dodges as far as a key press
	// the two trues override existing velocity instead of adding to it, so dodges don't compound
	LaunchCharacter(Direction.GetSafeNormal() * DodgeImpulse, true, true);
	

}
