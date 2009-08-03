/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_CRAFTINGSESSION_H
#define ANH_ZONESERVER_CRAFTINGSESSION_H

//#include "Utils/typedefs.h"
#include "CraftingEnums.h"
#include "CraftingTool.h"
#include "CraftingStation.h"
#include "ObjectFactoryCallback.h"
#include "Item.h"
#include "ManufacturingSchematic.h"
#include "Utils/rand.h"


//=============================================================================

class Database;
class DatbaseCallback;
class PlayerObject;
class DraftSchematic;
class ObjectFactoryCallback;
class DispatchClient;
class ManufacturingSchematic;
class CraftSessionQueryContainer;
class Clock;
class CraftWeight;

typedef std::vector<CraftWeight*>			CraftWeights;
typedef std::map<uint64,uint32>				CheckResources;

//=============================================================================

class CraftingSession : public DatabaseCallback, public ObjectFactoryCallback
{
	public:

		CraftingSession(Anh_Utils::Clock* clock,Database* database,PlayerObject* player,CraftingTool* tool,CraftingStation* station,uint32 expFlag);
		~CraftingSession();

		virtual void			handleObjectReady(Object* object,DispatchClient* client);
		virtual void			handleDatabaseJobComplete(void* ref,DatabaseResult* result);

		void					handleFillSlot(uint64 resContainerId,uint32 slotId,uint32 unknown,uint8 counter);
		void					handleFillSlotResource(uint64 resContainerId,uint32 slotId,uint32 unknown,uint8 counter);
		void					handleFillSlotComponent(uint64 componentId,uint32 slotId,uint32 unknown,uint8 counter);

		void					handleEmptySlot(uint32 slotId,uint64 containerId,uint8 counter);
		void					emptySlot(uint32 slotId,ManufactureSlot* manSlot,uint64 containerId);
		void					bagResource(ManufactureSlot* manSlot,uint64 containerId);
		void					bagComponents(ManufactureSlot* manSlot,uint64 containerId);

		CraftingTool*			getTool(){ return mTool; }
		CraftingStation*		getStation(){ return mStation; }
		uint32					getStage(){ return mStage; }
		ManufacturingSchematic*	getManufacturingSchematic(){ return mManufacturingSchematic; }
		Item*					getItem(){ return mItem; }
		uint32					getOwnerExpSkillMod(){ return mOwnerExpSkillMod; }
		uint32					getOwnerAssSkillMod(){ return mOwnerAssSkillMod; }
		
		uint32					isExperimentationAllowed(){ return mExpFlag; }

		void					setStage(uint32 stage){ mStage = stage; }
		void					setItem(Item* item){ mItem = item; }
		void					setManufactureSchematic(ManufacturingSchematic* manSchem){ mManufacturingSchematic = manSchem; }
		void					setOwnerExpSkillMod(uint32 skillMod){ mOwnerExpSkillMod = skillMod; }
		void					setOwnerAssSkillMod(uint32 skillMod){ mOwnerAssSkillMod = skillMod; }

		bool					selectDraftSchematic(uint32 schematicIndex);
		void					assemble(uint32 counter);

		void					experiment(uint8 counter,std::vector<std::pair<uint32,uint32> > properties);
		void					customizationStage(uint32 counter);
		void					creationStage(uint32 counter);
		void					experimentationStage(uint32 counter);
		void					customize(const int8* itemName);
		void					createPrototype(uint32 noPractice,uint32 counter);
		string					getSerial();

		//sets/retrieves the amounts a man schematic can produce
		void					setProductionAmount(uint32 amount){mProductionAmount = amount;}
		uint32					getProductionAmount(){return mProductionAmount;}

		//sets/retrieves the customization value we have for the given schematic
		void					setCustomization(uint32 cust){mCustomization = cust;}
		uint32					getCustomization(){return mCustomization;}

		void					createManufactureSchematic(uint32 counter);
		
	private:

		float					_calcWeightedResourceValue(CraftWeights* weights);
		void					_cleanUp();
		//===========================
		//gets the type of success failur for experimentation
		uint8					_experimentRoll(uint32 expPoints);
		
		//===========================
		//gets the type of success failur assembly
		uint8					_assembleRoll();

		//===========================
		//gets the average Malleability
		float					_calcAverageMalleability();
			
		PlayerObject*			mOwner;
		DraftSchematic*			mDraftSchematic;
		ManufacturingSchematic*	mManufacturingSchematic;
		Item*					mItem;
		Database*				mDatabase;
		CraftingTool*			mTool;
		float					mToolEffectivity;
		uint32					mExpFlag;
		uint32					mStage;
		CraftingStation*		mStation;
		bool					mFirstFill;
		Anh_Utils::Clock*		mClock;
		uint32					mExpSkillModId;
		uint32					mAssSkillModId;
		uint32					mOwnerExpSkillMod;
		uint32					mOwnerAssSkillMod;

		//sets the amount the schematic can produce
		uint32					mProductionAmount;
		uint32					mSubCategory;
		uint32					mCustomization;
		uint32					mCriticalCount;

};

//=============================================================================

class CraftSessionQueryContainer
{
	public:

		CraftSessionQueryContainer(CraftSessionQuery qType,uint8 counter) : mQType(qType),mCounter(counter){}
		~CraftSessionQueryContainer(){}

		CraftSessionQuery	mQType;
		uint8				mCounter;
};

//=============================================================================

#endif
