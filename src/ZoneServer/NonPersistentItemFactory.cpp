/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "NonPersistentItemFactory.h"
#include "Food.h"
#include "ItemFactory.h"
#include "ObjectFactoryCallback.h"
#include "WorldManager.h"
#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "Utils/utils.h"

//=============================================================================

NonPersistentItemFactory* NonPersistentItemFactory::mSingleton  = NULL;

//======================================================================================================================

NonPersistentItemFactory* NonPersistentItemFactory::Instance(void)
{
	if (!mSingleton)
	{
		mSingleton = new NonPersistentItemFactory(WorldManager::getSingletonPtr()->getDatabase());
	}
	return mSingleton;
}


// This constructor prevents the default constructor to be used, as long as the constructor is kept private.

NonPersistentItemFactory::NonPersistentItemFactory() : FactoryBase(NULL)
{
}

//=============================================================================


NonPersistentItemFactory::NonPersistentItemFactory(Database* database) : FactoryBase(database)
{
	_setupDatabindings();
}


//=============================================================================

NonPersistentItemFactory::~NonPersistentItemFactory()
{
	_destroyDatabindings();
	mSingleton = NULL;
}

//=============================================================================

void NonPersistentItemFactory::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
	QueryNonPersistentItemFactory* asyncContainer = reinterpret_cast<QueryNonPersistentItemFactory*>(ref);

	switch(asyncContainer->mQueryType)
	{
		case NPQuery_MainData:
		{
			Item* item = _createItem(result, asyncContainer->mId);

			// To be used when doing item factories.
			if (item->getLoadState() == LoadState_Attributes)
			{
				QueryNonPersistentItemFactory* asContainer = new(mQueryContainerPool.ordered_malloc()) QueryNonPersistentItemFactory(asyncContainer->mOfCallback,NPQuery_Attributes,asyncContainer->mId);
				asContainer->mObject = item;

				mDatabase->ExecuteSqlAsync(this,asContainer,"SELECT attributes.name,item_family_attribute_defaults.attribute_value,attributes.internal"
															 " FROM item_family_attribute_defaults"
															 " INNER JOIN attributes ON (item_family_attribute_defaults.attribute_id = attributes.id)"
															 " WHERE item_family_attribute_defaults.item_type_id = %u ORDER BY item_family_attribute_defaults.attribute_order",item->getItemType());
			}
			else if (item->getLoadState() == LoadState_Loaded && asyncContainer->mOfCallback)
			{
				asyncContainer->mOfCallback->handleObjectReady(item);
			}
		}
		break;

		case NPQuery_Attributes:
		{
			_buildAttributeMap(asyncContainer->mObject,result);
			if ((asyncContainer->mObject->getLoadState() == LoadState_Loaded) && (asyncContainer->mOfCallback))
			{
				asyncContainer->mOfCallback->handleObjectReady(asyncContainer->mObject);
			}
		}
		break;

		default:break;
	}
	mQueryContainerPool.free(asyncContainer);
}

//=============================================================================
void NonPersistentItemFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client)
{
	requestObject(ofCallback,id, 0);
}

//=============================================================================

void NonPersistentItemFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id, uint64 newId)
{
	// gLogger->logMsgF("NonPersistentItemFactory::requestObject: item_types = %"PRIu64"",MSG_NORMAL, id);
	mDatabase->ExecuteSqlAsync(this,new(mQueryContainerPool.ordered_malloc()) QueryNonPersistentItemFactory(ofCallback,NPQuery_MainData,newId),
							"SELECT item_family_attribute_defaults.family_id, item_family_attribute_defaults.item_type_id, item_types.object_string, item_types.stf_name, item_types.stf_file, item_types.stf_detail_file"
							" FROM item_types"
							" INNER JOIN item_family_attribute_defaults ON (item_types.id = item_family_attribute_defaults.item_type_id AND item_family_attribute_defaults.attribute_id = 1)"
							" WHERE (item_types.id = %"PRIu64")",id);
}

//=============================================================================

Item* NonPersistentItemFactory::_createItem(DatabaseResult* result, uint64 newId)
{

	Item*			item;
	ItemIdentifier	itemIdentifier;

	uint64 count = result->getRowCount();

	result->GetNextRow(mItemIdentifierBinding,(void*)&itemIdentifier);
	result->ResetRowIndex();

	switch(itemIdentifier.mFamilyId)
	{
		// case ItemFamily_TravelTickets:			item	= new TravelTicket();				break;
		// case ItemFamily_SurveyTools:			item	= new SurveyTool();					break;
		// case ItemFamily_CraftingTools:			item	= new CraftingTool();				break;
		// case ItemFamily_CraftingStations:		item	= new CraftingStation();			break;
		case ItemFamily_Foods:					item	= new Food();						break;
		// case ItemFamily_Furniture:				item	= new Furniture();					break;
		// case ItemFamily_Wearable:				item	= new Wearable();					break;
		// case ItemFamily_ManufacturingSchematic:	item	= new ManufacturingSchematic();		break;
		// case ItemFamily_Instrument:
		case ItemFamily_Generic:				item	= new Item();						break;
		// case ItemFamily_Weapon:					item	= new Weapon();						break;

		default:
		{
			item = new Item();
			gLogger->logMsgF("ItemFactory::NonPersistentItemFactory::_createItem unknown Family %u",MSG_NORMAL,itemIdentifier.mFamilyId);
		}
		break;
	}

	result->GetNextRow(mItemBinding,item);
	
	item->setId(newId);
	item->setItemFamily(itemIdentifier.mFamilyId);
	item->setItemType(itemIdentifier.mTypeId);
	item->setLoadState(LoadState_Attributes);
	return item;
}

//=============================================================================

void NonPersistentItemFactory::_setupDatabindings()
{
	mItemBinding = mDatabase->CreateDataBinding(4);
	mItemBinding->addField(DFT_bstring,offsetof(Item,mModel),256,2);
	mItemBinding->addField(DFT_bstring,offsetof(Item,mName),64,3);
	mItemBinding->addField(DFT_bstring,offsetof(Item,mNameFile),64,4);
	mItemBinding->addField(DFT_bstring,offsetof(Item,mDetailFile),64,5);

	mItemIdentifierBinding = mDatabase->CreateDataBinding(2);
	mItemIdentifierBinding->addField(DFT_uint32,offsetof(ItemIdentifier,mFamilyId),4,0);
	mItemIdentifierBinding->addField(DFT_uint32,offsetof(ItemIdentifier,mTypeId),4,1);

}

//=============================================================================

void NonPersistentItemFactory::_destroyDatabindings()
{
	mDatabase->DestroyDataBinding(mItemBinding);
	mDatabase->DestroyDataBinding(mItemIdentifierBinding);
}

//=============================================================================

