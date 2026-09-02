// Learning project - written by hand, not from the template.

#include "EnemyCharacter.h"
#include "EnemyAIController.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "LearningUE.h"
#include "StatsComponent.h"
#include "TimerManager.h"

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

	// Face what the CONTROLLER is looking at, not where the feet are going.
	//
	// The player uses bOrientRotationToMovement, which turns the body to face its own
	// velocity - right for someone steering with a camera. An AI wants the opposite: it
	// should keep looking at you while it circles, backs off, or walks past. So the
	// controller decides the facing (via SetFocus) and the body turns toward it at
	// RotationRate.
	//
	// With no focus set, the AI controller points itself along its current path, so a
	// walking enemy still faces where it is going. Both cases come out right.
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Slower than the player's walk (500) on purpose. An enemy that outruns you turns
	// every encounter into a fight you did not choose, and it makes the whole detection
	// system pointless - there is no reason to sneak past something you cannot escape.
	GetCharacterMovement()->MaxWalkSpeed = EnemyWalkSpeed;

	// the same component the player and the training dummy carry. No shared game class
	// between the three of them.
	Stats = CreateDefaultSubobject<UStatsComponent>(TEXT("Stats"));

	// --- the brain ---

	// Which controller class to spawn for this body. The pawn does not contain its own
	// AI; it names the controller that will drive it.
	AIControllerClass = AEnemyAIController::StaticClass();

	// And when to spawn it. PlacedInWorldOrSpawned covers both enemies already standing
	// in the level and any spawned later. The default is Disabled, which is why an
	// AIControllerClass alone possesses nothing and the enemy just stands there.
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
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

UAnimMontage* AEnemyCharacter::SelectDeathMontage(AActor* Killer) const
{
	// nobody to measure against: a fall, a scripted kill, damage from no actor at all
	if (!Killer)
	{
		return DeathMontageFront;
	}

	// Direction from me to whoever killed me. Normal2D flattens Z, so an attacker
	// standing on a ledge above me still reads as "in front", not "overhead".
	const FVector ToKiller = (Killer->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();

	// UE idiom: the dot product of two unit vectors is how much they agree.
	// +1 exactly the same way, 0 perpendicular, -1 exactly opposite.
	const float ForwardDot = FVector::DotProduct(GetActorForwardVector(), ToKiller);
	const float RightDot = FVector::DotProduct(GetActorRightVector(), ToKiller);

	// Bigger magnitude wins the axis - was this more of a front/back hit or a side one?
	// Then the sign picks the end of that axis.
	if (FMath::Abs(ForwardDot) >= FMath::Abs(RightDot))
	{
		return ForwardDot >= 0.0f ? DeathMontageFront : DeathMontageBack;
	}

	return RightDot >= 0.0f ? DeathMontageRight : DeathMontageLeft;
}

void AEnemyCharacter::StartRagdoll()
{
	// "Ragdoll" is a stock collision profile: the mesh stops being a query-only shape
	// and starts colliding with the world as physics bodies.
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));

	// setting a profile replaces every response, so the camera exemption goes back on
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	// hands the skeleton to the physics asset (PA_Mannequin). From here the animation
	// system no longer drives the bones - gravity and collisions do.
	GetMesh()->SetSimulatePhysics(true);
}

void AEnemyCharacter::HandleDeath(AActor* Killer)
{
	UE_LOG(LogLearningUE, Warning, TEXT("%s died"), *GetName());

	// a corpse should neither walk nor be walked into
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// A corpse must not shove the player's camera about. The spring arm traces on the
	// Camera channel every frame and pulls in when something blocks it, so a body you
	// just walked into springs the camera to your shoulders.
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	const float Duration = PlayAnimMontage(SelectDeathMontage(Killer));

	if (Duration > 0.0f)
	{
		// A timer, not an event - and for once that is the right call. The death montages
		// have Auto Blend Out switched off so they hold their final pose, which means
		// they never "end" and OnMontageEnded never fires. With no event to listen to,
		// the montage's own duration is the only signal available.
		GetWorldTimerManager().SetTimer(RagdollTimer, this, &AEnemyCharacter::StartRagdoll, Duration, false);
	}
	else
	{
		// no montage played, so go straight to physics rather than freezing upright
		StartRagdoll();
	}

	// UE idiom: SetLifeSpan destroys the actor after N seconds, with no timer to own or
	// clear. The ragdoll gets CorpseLingerTime to settle before the body is removed.
	SetLifeSpan(Duration + CorpseLingerTime);
}
