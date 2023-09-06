// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveSubsystemBFL.h"
#include "SubSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetStringLibrary.h"
#include "SaveConfigFile.h"




USubSaveGame* USaveSubsystemBFL::GetLastSavedGame(const int SlotIndex)
{
	USubSaveGame* LastSaveGame = nullptr;
	
	for (int i = 0 ; i < SSGConfig::MaxSaves ;++i )//find last quick save
	{
		FString SlotName = SUB_QUICK_SLOTNAME + UKismetStringLibrary::Conv_IntToString(i);

		
		if (USubSaveGame* HelperSave = LoadSaveObjectIfExist(SlotName,SlotIndex ) )
		{
			if (LastSaveGame != nullptr)
			{
				if (HelperSave->SaveID.Date > LastSaveGame->SaveID.Date)
				{
					LastSaveGame = HelperSave;
				}
			}
			else LastSaveGame = HelperSave;
		
		}	
	}
	
	for (int i = 0; i < SSGConfig::MaxAutoSaves ; ++i)
	{
		FString SlotName = SUB_AUTO_SLOTNAME+UKismetStringLibrary::Conv_IntToString(i);
		if (USubSaveGame* HelperSave = LoadSaveObjectIfExist(SlotName,SlotIndex ) )
		{
			if (LastSaveGame != nullptr)
			{
				if (HelperSave->SaveID.Date > LastSaveGame->SaveID.Date)
				{
					LastSaveGame = HelperSave;
				}
			}
			else LastSaveGame = HelperSave;
		}
	}
	return LastSaveGame;
	
}

void USaveSubsystemBFL::DeleteSavedGame(const FString& InSlotName,const int SlotIndex)
{
	if (UGameplayStatics::DoesSaveGameExist(InSlotName,SlotIndex))
	{
		UGameplayStatics::DeleteGameInSlot(InSlotName,SlotIndex);
	}
}

void USaveSubsystemBFL::GetAllAutoSaveGames(TMap<FString, USubSaveGame*>& OutAutoSaveGameSaves,const int SlotIndex)
{
	
	for (int i = 0; i < SSGConfig::MaxAutoSaves; ++i)
	{
		FString PossibleSlotName = SUB_AUTO_SLOTNAME + UKismetStringLibrary::Conv_IntToString(i);
		USubSaveGame* Save = LoadSaveObjectIfExist(PossibleSlotName,SlotIndex );
		
		if (Save != nullptr) OutAutoSaveGameSaves.Add(PossibleSlotName,Save);
	}
}

TArray<USubSaveGame*> USaveSubsystemBFL::GetAllSaveGames(const int SlotIndex)
{
	TArray<USubSaveGame*> Saves ;
	for (int i = 0; i < SSGConfig::MaxSaves; ++i)
	{
		FString PossibleSlotName = SUB_QUICK_SLOTNAME + UKismetStringLibrary::Conv_IntToString(i);
		USubSaveGame* Save = LoadSaveObjectIfExist(PossibleSlotName,SlotIndex );
		
		if (Save != nullptr) Saves.Add(Save);
	}
	return Saves;
}

void USaveSubsystemBFL::GetAllSaveGameNames(TArray<FString>& OutSlotNames,const int SlotIndex)
{
	for (int i = 0; i < SSGConfig::MaxSaves; ++i)
	{
		
		FString PossibleSlotName = SUB_QUICK_SLOTNAME +UKismetStringLibrary::Conv_IntToString(i);
		if (UGameplayStatics::DoesSaveGameExist(PossibleSlotName,SlotIndex))
		{
			OutSlotNames.Add(PossibleSlotName);
		}
	}
}

USubSaveGame* USaveSubsystemBFL::LoadSaveObjectIfExist(const FString& Name, const int Index)
{
	if (UGameplayStatics::DoesSaveGameExist(Name,Index))
	{
		USubSaveGame* SaveGame = Cast<USubSaveGame>(UGameplayStatics::LoadGameFromSlot(Name,Index));
		return SaveGame;
	}
	return nullptr;
}




