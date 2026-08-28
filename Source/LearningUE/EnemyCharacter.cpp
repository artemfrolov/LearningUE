// Learning project - written by hand, not from the template.

#include "EnemyCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "LearningUE.h"
#include "StatsComponent.h"

AEnemyCharacter::AEnemyCharacter()
{
	// nothing to do per frame yet - the AI in Phase 5 will not need Tick either,
	// because StateTree drives itself
	PrimaryActorTick.bCanEverTick = false;

	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	// UE idiom: a fresh ACharacter's mesh is misaligned with its capsule - it floats at
	// the capsule's centre and faces along +X instead of forward. Every UE character
	// needs this exact offset, which is why the template's Blueprint already has it.
	// Half the capsule height down, ninety degrees round.
	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -89.0f), FRotator(0.0f, -90.0f, 0.0f));

	// face where it is moving, not where some controller is looking - the same
	// third-person setup the player uses
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// the same component the player and the training dummy carry. No shared game class
	// between the three of them.
	Stats = CreateDefaultSubobject<UStatsComponent>(TEXT("Stats"));
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	Stats->OnHealthChanged.AddDynamic(this, &AEnemyCharacter::HandleHealthChanged);
	Stats->OnDamaged.AddDynamic(this, &AEnemyCharacter::HandleDamaged);
	Stats->OnDied.AddDynamic(this, &AEnemyCharacter::HandleDeath);
}

void AEnemyCharacter::HandleDamaged(float Amount, AActor* Causer)
{
	// A fatal blow fires OnDied first, from inside SetHealth - so by the time we get
	// here the body is already dead and a flinch would fight the death animation.
	// Death gets its own reaction in 3.6c.
	if (!Stats->IsAlive())
	{
		return;
	}

	// if a usable flinch montage is ever set, prefer it over the shove
	if (HitReactMontage)
	{
		PlayAnimMontage(HitReactMontage);
		return;
	}

	// otherwise read the impact physically: shove straight away from whoever hit us.
	// This is why OnDamaged carries the Causer - the component never needed it, the
	// reaction does.
	if (Causer)
	{
		// 2D so a punch staggers rather than launches - GetSafeNormal2D flattens Z out
		const FVector AwayFromAttacker = (GetActorLocation() - Causer->GetActorLocation()).GetSafeNormal2D();

		// false on the Z override keeps whatever vertical velocity we already had,
		// so being hit mid-fall does not freeze us in the air
		LaunchCharacter(AwayFromAttacker * HitKnockbackImpulse, true, false);
	}
}

void AEnemyCharacter::HandleHealthChanged(float NewValue, float MaxValue)
{
	UE_LOG(LogLearningUE, Warning, TEXT("%s: %.0f / %.0f"), *GetName(), NewValue, MaxValue);
}

void AEnemyCharacter::HandleDeath()
{
	UE_LOG(LogLearningUE, Warning, TEXT("%s died"), *GetName());

	// placeholder, same as the training dummy. 3.6c replaces this with a death montage
	// and stops the body vanishing mid-air.
	Destroy();
}
