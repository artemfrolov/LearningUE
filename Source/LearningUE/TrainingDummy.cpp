// Learning project - written by hand, not from the template.

#include "TrainingDummy.h"
#include "Components/StaticMeshComponent.h"
#include "LearningUE.h"
#include "StatsComponent.h"

ATrainingDummy::ATrainingDummy()
{
	// nothing to do per frame
	PrimaryActorTick.bCanEverTick = false;

	// UE idiom: an actor needs a root component with a transform, and the mesh is the
	// obvious candidate - everything else hangs off where the body is.
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	// "BlockAllDynamic" gives this the WorldDynamic object type, which is one of the two
	// types the attack trace looks for. Get this wrong and punches pass straight through
	// with no error anywhere - the sweep simply never considers it.
	Mesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	Mesh->SetMobility(EComponentMobility::Movable);

	// the same component the player has. No shared parent class involved.
	Stats = CreateDefaultSubobject<UStatsComponent>(TEXT("Stats"));
}

void ATrainingDummy::BeginPlay()
{
	Super::BeginPlay();

	Stats->OnHealthChanged.AddDynamic(this, &ATrainingDummy::HandleHealthChanged);
	Stats->OnDied.AddDynamic(this, &ATrainingDummy::HandleDeath);
}

void ATrainingDummy::HandleHealthChanged(float NewValue, float MaxValue)
{
	UE_LOG(LogLearningUE, Warning, TEXT("%s: %.0f / %.0f"), *GetName(), NewValue, MaxValue);
}

void ATrainingDummy::HandleDeath(AActor* Killer)
{
	UE_LOG(LogLearningUE, Warning, TEXT("%s destroyed"), *GetName());

	// crude on purpose. A real enemy gets a death montage and a ragdoll in 3.6.
	Destroy();
}
