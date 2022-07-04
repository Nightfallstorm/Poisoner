#include "Papyrus.h"

using namespace RE;

namespace Papyrus
{
	constexpr std::string_view PapyrusClass = "PoisonWeaponNative";

	int32_t PoisonActorWeapon(VM*, StackID, StaticFunctionTag*, Actor* akActor, bool isLeftHand, AlchemyItem* poison, int32_t poisonCharge)
	{
		if (akActor == nullptr || !akActor->Is3DLoaded() || poison == nullptr || !poison->IsPoison()) {
			logger::error("Actor or poison is invalid, returning");
			return -1;
		}	

		if (akActor->GetEquippedObject(isLeftHand) == nullptr) {
			logger::error("Equipped object is null");
			return -1;
		}

		try {
			assert(akActor->GetEquippedObject(isLeftHand)->As<TESObjectWEAP>() != nullptr);
			assert(akActor->GetEquippedEntryData(isLeftHand)->extraLists != nullptr);
		} catch (...) {
			logger::error("Invalid object equipped");
			return -1;	
		}

		BSSimpleList<ExtraDataList*>* inventoryDataList = akActor->GetEquippedEntryData(isLeftHand)->extraLists;
		ExtraDataList *weaponList = nullptr;
		std::for_each(inventoryDataList->begin(), inventoryDataList->end(), [&](ExtraDataList* dataList) {
			if ((isLeftHand && dataList->HasType<ExtraWornLeft>()) || (!isLeftHand && dataList->HasType<ExtraWorn>())) {
				if (!dataList->HasType<ExtraPoison>()) {
					weaponList = dataList;
					logger::info("Grabbed unpoisoned weapon data");
				} else {
					logger::info("Grabbed poisoned weapon data, removing poison...");
					dataList->RemoveByType(ExtraPoison::EXTRADATATYPE);
					weaponList = dataList;
					logger::info("Poison removed");
				}
			}
		});
		
		ExtraPoison* poisonData = BSExtraData::Create<ExtraPoison>();
		poisonData->poison = poison;
		poisonData->count = poisonCharge;
		
		if (&weaponList == nullptr) {
			logger::error("List is null");
			return -1;
		}
		weaponList->Add(poisonData);


		logger::info("Poison Added");

		return poisonCharge;
	}

	int32_t GetPoisonWeaponCharges(VM*, StackID, StaticFunctionTag*, Actor* akActor, bool isLeftHand)
	{
		if (akActor == nullptr || !akActor->Is3DLoaded()) {
			logger::error("Actor is null or not loaded, returning");
			return -1;
		}

		BSSimpleList<ExtraDataList*>* weaponData = nullptr;
		try {
			weaponData = akActor->GetEquippedEntryData(isLeftHand)->extraLists; 
			assert(akActor->GetEquippedEntryData(isLeftHand)->extraLists != nullptr);
		} catch (...) {
			logger::error("Object equipped to actor null or invalid");
			return -1;
		}
		
		int poisonCount = -1;
		std::for_each(weaponData->begin(), weaponData->end(), [&](const ExtraDataList* data) {
			if ((isLeftHand && data->HasType<ExtraWornLeft>()) || (!isLeftHand && data->HasType<ExtraWorn>())) {
				if (data->HasType<ExtraPoison>()) {
					poisonCount = data->GetByType<ExtraPoison>()->count;
				}
			}
		});
		return poisonCount;
	}
	bool Bind(VM* a_vm)
	{
		if (!a_vm) {
			logger::critical("couldn't get VM State"sv);
			return false;
		}

		logger::info("{:*^30}", "FUNCTIONS"sv);

		a_vm->RegisterFunction("PoisonActorWeapon"sv, PapyrusClass, PoisonActorWeapon, true);
		a_vm->RegisterFunction("GetPoisonWeaponCharges"sv, PapyrusClass, GetPoisonWeaponCharges, true);

		logger::info("Registered PoisonActorWeapon"sv);

		return true;
	}
}
