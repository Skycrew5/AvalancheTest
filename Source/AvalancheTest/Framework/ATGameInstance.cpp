// Scientific Ways

#include "Framework/ATGameInstance.h"

#include "World/ATVoxelChunk.h"

UATGameInstance::UATGameInstance()
{
	
}

//~ Begin Statics
UATGameInstance* UATGameInstance::TryGetATGameInstance(const UObject* InWCO)
{
	return Cast<UATGameInstance>(TryGetScWGameInstance(InWCO));
}
//~ End Statics

//~ Begin Initialize
void UATGameInstance::Init() // UGameInstance
{
	Super::Init();

	
}

void UATGameInstance::Shutdown() // UGameInstance
{
	

	Super::Shutdown();
}
//~ End Initialize

//~ Begin Voxels
//~ End Voxels
