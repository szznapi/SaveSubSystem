// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveGISubsystem.h"

#include "EngineUtils.h"
#include "SubSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetStringLibrary.h"
#include "SaveableInterface.h"
#include "SaveConfigFile.h"
#include "Kismet/KismetMathLibrary.h"
#include "SaveSubsystemBFL.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"





DEFINE_LOG_CATEGORY(SubloadingSubsystemLOG);

#define print(text,Color) if (GEngine) GEngine->AddOnScreenDebugMessage(-1,100,Color,text)

void USaveGISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}



void USaveGISubsystem::SetCurrentSave(const FString& InSaveName, USubSaveGame* InSaveGame,bool UpdatePlaytime)
{
	if (!InSaveGame) return;

	CurrentSlotName = InSaveName;
	int SecondsPlayed = 0;
	
	if (UpdatePlaytime && CurrentSaveGame != nullptr)
	{
		SecondsPlayed = CurrentSaveGame->SaveID.TimePlayed;
	}
	CurrentSaveGame = InSaveGame;
	CurrentSaveGame->SaveID.TimePlayed = SecondsPlayed;	
}

void USaveGISubsystem::UpdateSessionTime(const int Value)
{
	if (!CurrentSaveGame) return;
	CurrentSaveGame->SaveID.TimePlayed += Value;
}

USubSaveGame* USaveGISubsystem::CreateNewGameSave(FString& OutSlotName,const int SlotIndex,TSubclassOf<USubSaveGame> SaveClass)
{
	// find viable save game slot name  
	for (int i = 0 ; i < SSGConfig::MaxSaves ;++i )
	{
		FString CheckedSlotName=  SUB_QUICK_SLOTNAME + UKismetStringLibrary::Conv_IntToString(i);
		
		if (!UGameplayStatics::DoesSaveGameExist(CheckedSlotName,SlotIndex))
		{
			OutSlotName  = CheckedSlotName;
			break;
		}
	}
	
	USubSaveGame* SaveGame  = NewObject<USubSaveGame>(GetTransientPackage(),SaveClass);
		
	// setup object
	SetCurrentSave(OutSlotName,SaveGame,false);
	SaveGame->SlotName = FName(OutSlotName);
	
	return SaveGame;
	
}

USubSaveGame* USaveGISubsystem::CreateNewAutoSave(FString& OutSlotName,int SlotIndex,TSubclassOf<USubSaveGame> SaveClass)
{
	/// do poprawki
	for (int i = 0; i < SSGConfig::MaxAutoSaves ; ++i) // check if we can create new auto save
	{
		FString CheckedSlotName =  SUB_AUTO_SLOTNAME + UKismetStringLibrary::Conv_IntToString(i); 
		
		if (!UGameplayStatics::DoesSaveGameExist(CheckedSlotName,SlotIndex))
		{
			OutSlotName = CheckedSlotName;
			break;
		}
		
	}
	USubSaveGame* SaveGame = nullptr;
	
	if (OutSlotName.IsEmpty()) // find last saved game
	{
		//IFileManager::Get().FileSize(*(FString::Printf(TEXT("%sSaveGames/%s.sav"), *FPaths::ProjectSavedDir(), Name))) >= 0;
		//// a clue how to make it better
		
		for (int i = 0; i < SSGConfig::MaxAutoSaves; ++i)
		{
			FString CheckedSlotName = SUB_AUTO_SLOTNAME + UKismetStringLibrary::Conv_IntToString(i);

			if (USubSaveGame* CompareSave = USaveSubsystemBFL::LoadSaveObjectIfExist(  CheckedSlotName,SlotIndex ))
			{
				if (SaveGame)
				{
					if (CompareSave->SaveID.Date > SaveGame->SaveID.Date) continue;
				
					SaveGame = CompareSave;
					OutSlotName = CheckedSlotName;
				}
				else
				{
					SaveGame = CompareSave;
					OutSlotName =  CheckedSlotName;
				}
			}
		}
	}
	else // create new auto save
	{
		SaveGame = NewObject<USubSaveGame>(GetTransientPackage(),SaveClass);
	}
	
	SetCurrentSave(OutSlotName,SaveGame,true);
	SaveGame->SlotName = FName(OutSlotName);
	return SaveGame;
}

void USaveGISubsystem::WriteSaveGame()
{
	if (!CurrentSaveGame)
	{
		print("Current save game file is INVALID ", FColor::Red);
		return;
	}
	if (!GetWorld())
	{
		print("Save game called but UWorld is invalid", FColor::Red);
		return;
	}

	CurrentSaveGame->SavedActors.Empty();

	for (FActorIterator AI(GetWorld());AI ; ++AI) // iterate through all actors
	{
		AActor* CurrentActor = *AI;
		if (!IsValid(CurrentActor) || !CurrentActor->Implements<USaveableInterface>()) continue;//go next if not interested

		CreateSingleActorData(CurrentActor);
	}

	CurrentSaveGame->SavedActors.Sort([](const FActorSaveData& FirstActorData, const FActorSaveData& SecondActorData)//sort saved actors
	{
		return FirstActorData>SecondActorData;
	});
	
	//const FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(GetWorld());
	CurrentSaveGame->SetSaveID(FName(CurrentSlotName),UKismetMathLibrary::UtcNow(),CurrentSaveGame->SaveID.TimePlayed,LevelName);

	OnSaveGameWritten.Broadcast(CurrentSaveGame);
	UGameplayStatics::SaveGameToSlot(CurrentSaveGame,CurrentSlotName,CurrentSlotIndex);
}

void USaveGISubsystem::CreateSingleActorData(AActor* ActorToSave)
{
	//Save actor
	FActorSaveData CurrentActorData;

	ISaveableInterface* InterfaceActor = Cast<ISaveableInterface>(ActorToSave);
	InterfaceActor->Execute_BeforeActorSaved(ActorToSave,CurrentActorData);
	CurrentActorData.ActorName = FName(ActorToSave->GetName());
	CurrentActorData.ActorTransform = ActorToSave->GetActorTransform();
	CurrentActorData.ActorCentroid = ActorToSave->GetRootComponent()->Bounds.Origin;
	CurrentActorData.ActorClass = ActorToSave->GetClass();
			
	FMemoryReader MemReader(CurrentActorData.ActorByteData);
	FObjectAndNameAsStringProxyArchive Archive(MemReader,false);
	Archive.ArNoDelta = true;
	Archive.ArIsSaveGame = true;
	ActorToSave->Serialize(Archive);

	// and actor components
	for (auto Comp : ActorToSave->GetComponentsByInterface(USaveableInterface::StaticClass()))
	{
		if(IsValid(Comp) )
		{
			if(UStaticMeshComponent* SMC = Cast<UStaticMeshComponent>(Comp))
			{
				CurrentActorData.ComponentsData.Add(CreateSingleComponentData(SMC));
			}
		}
	}
	CurrentSaveGame->SavedActors.Add(CurrentActorData);
}

FComponentSaveData USaveGISubsystem::CreateSingleComponentData(UStaticMeshComponent* ComponentToSave)
{
	FComponentSaveData SMCData;

	ISaveableInterface* InterfaceComponent = Cast<ISaveableInterface>(ComponentToSave);
	InterfaceComponent->Execute_BeforeComponentSaved(ComponentToSave,SMCData);
	SMCData.ComponentTransform = ComponentToSave->GetComponentTransform();
	SMCData.ComponentName = FName(ComponentToSave->GetName());

	FMemoryWriter MemWriter(SMCData.ComponentByteData);
	FObjectAndNameAsStringProxyArchive ArchiveC(MemWriter,false);

	ArchiveC.ArNoDelta = true;
	ArchiveC.ArIsSaveGame = true;
	ComponentToSave->Serialize(ArchiveC);
	
	return SMCData;
}

void USaveGISubsystem::LoadSaveGame()
{
	if (!UGameplayStatics::DoesSaveGameExist(CurrentSlotName,CurrentSlotIndex))
	{
		UE_LOG(SubloadingSubsystemLOG,Log,TEXT("Save object does not exist %s"),*CurrentSlotName);
		return;
	}
	CurrentSaveGame = Cast<USubSaveGame>(UGameplayStatics::LoadGameFromSlot(CurrentSlotName,CurrentSlotIndex));
		if (CurrentSaveGame == nullptr)
		{
			UE_LOG(SubloadingSubsystemLOG,Error, TEXT("Failed to Load game data"));
			return;
		}
	
		UE_LOG(SubloadingSubsystemLOG,Display,TEXT("Loading Game"));

		TArray<AActor*> LoadableActors;

		for (FActorIterator It(GetWorld(),EActorIteratorFlags::AllActors); It ; ++It)// get all actors in the world that could be loaded
		{
			if ((*It)->Implements<USaveableInterface>()) LoadableActors.Add(*It);
		}

		TArray<bool> WasLoadedActors;
		WasLoadedActors.Init(false,CurrentSaveGame->SavedActors.Num());
		
		while (LoadableActors.Num() != 0)
		{
			AActor* CurrentA = LoadableActors.Last();
			
			const int i = CurrentSaveGame->SavedActors.IndexOfByPredicate([&CurrentA](const FActorSaveData& Record)// find current actor record index
			{
				return Record.ActorTransform.GetLocation().Equals(CurrentA->GetActorLocation(),0.1)
				&& Record.ActorClass == CurrentA->GetClass()
				&& Record.ActorCentroid.Equals(CurrentA->GetRootComponent()->Bounds.Origin,0.1);
			}) ;

			if (i != -1)
			{
				SimpleLoadActor(CurrentA,CurrentSaveGame->SavedActors[i]);
				LoadableActors.RemoveSingle(CurrentA);
				WasLoadedActors[i] = true;
			}
			else
			{
				UE_LOG(SubloadingSubsystemLOG,Log,TEXT("Actor destroyed in loading process - not valid data found - %s"), *CurrentA->GetName());
				LoadableActors.RemoveSingle(CurrentA);
				CurrentA->Destroy();
			}
			
		}
		// if some records are left with " WasLoaded = false" its mean it was not find in the world - there is no actor record to fill we need to spawn one
		for(int i = 0; i < WasLoadedActors.Num(); ++i)
		{
			if (WasLoadedActors[i] == false)
			{
				WasLoadedActors[i] = true;
				FActorSaveData DataLeft = CurrentSaveGame->SavedActors[i];
				
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				if (auto SpawnedActor = GetWorld()->SpawnActor(DataLeft.ActorClass)) SimpleLoadActor(SpawnedActor,DataLeft);
				
			}
		}
		OnSaveGameLoaded.Broadcast(CurrentSaveGame);	
}

void USaveGISubsystem::SimpleLoadActor(AActor* ActorToLoad,const FActorSaveData& Record)
{
	ISaveableInterface* IA = Cast<ISaveableInterface>(ActorToLoad);
	// load actor
	FMemoryReader MemReader(Record.ActorByteData);
	FObjectAndNameAsStringProxyArchive Archive(MemReader, true);
	Archive.ArIsSaveGame = true;
	ActorToLoad->Serialize(Archive);

	ActorToLoad->SetActorTransform(Record.ActorTransform,false,nullptr,ETeleportType::ResetPhysics);

	TArray<UStaticMeshComponent*> TempParts;
	for (auto Comp : ActorToLoad->GetComponentsByInterface(USaveableInterface::StaticClass()))
	{
		if(IsValid(Comp) )
		{
			if(UStaticMeshComponent* SMC = Cast<UStaticMeshComponent>(Comp))
				TempParts.Add(SMC);
		}
	}
	
	for (int i = TempParts.Num()-1;i>=0;--i)// load actor components
	{
		UStaticMeshComponent* Part = TempParts[i];
		ISaveableInterface* IC = Cast<ISaveableInterface>(Part);
		const FComponentSaveData* CurrentData = Record.ComponentsData.FindByPredicate([Part](const FComponentSaveData& PartData)
		{ return Part->GetName() == PartData.ComponentName;});

		if (CurrentData)
		{
			FMemoryReader MReader(Record.ActorByteData);
			FObjectAndNameAsStringProxyArchive ArchiveC(MReader,true);
			ArchiveC.ArIsSaveGame = true;
			Part->Serialize(ArchiveC);

			Part->SetWorldTransform(CurrentData->ComponentTransform,false,nullptr, ETeleportType::ResetPhysics);
			
		}
		IC->Execute_AfterComponentLoaded(Part,*CurrentData);
	}
	IA->Execute_AfterActorLoaded(ActorToLoad,Record);
}

bool USaveGISubsystem::IsReadyToLoad()
{

	return GetWorld()->HasStreamingLevelsToConsider();
}








