/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_BUFF_H
#define ANH_BUFF_H

#include "BuffAttributeEnums.h"
#include "BuffIconsEnum.h"

#include "Utils/typedefs.h"

#include <list>

class CreatureObject;
class BuffAttribute;
class BuffAttributeDBItem;
class BuffDBItem;

typedef std::list<BuffAttribute*>	AttributeList;

class BuffAttribute
{

public:
	BuffAttribute(BuffAttributeEnum Type, int32 InitialValue, int32	TickValue, int32 FinalValue);
	~BuffAttribute();

	static BuffAttribute* FromDB(BuffAttributeDBItem* item);

	BuffAttributeEnum	GetType();
	int32				GetInitialValue();
	int32				GetTickValue();
	int32				GetFinalValue();

	void				SetInitialValue(int32 v){mInitialValue = v;} 
	void				SetFinalValue(int32 v){mFinalValue = v;} 

private:
	BuffAttributeEnum	mAttribute;
	int32				mInitialValue;
	int32				mTickValue;
	int32				mFinalValue;
};


// Buffs might make problems in cases were we modify a debuff of the ham in the ham object in order to prevent the ham from getting negativ
// we still need mechanisms there to prevent the debuff from restoring the full amount of ham in these cases

class Buff
{
	friend class CreatureObject;
	friend class BuffManager;

public:
	///////////////////////////////////////////////////////////////////////////////////////////////
	//Public Constructor/Destructor
	///////////////////////////////////////////////////////////////////////////////////////////////
	static Buff* SimpleBuff(CreatureObject* Target, CreatureObject* Instigator, uint64 Duration, uint32 Icon, uint64 CurrentGlobalTick);
	static Buff* TickingBuff(CreatureObject* Target, CreatureObject* Instigator, uint NoOfTicks, uint64 Tick, uint32 Icon, uint64 CurrentGlobalTick);
	static Buff* FromDB(BuffDBItem* Item, uint64 CurrentGlobalTick);

	~Buff(){EraseAttributes();}

	///////////////////////////////////////////////////////////////////////////////////////////////
	//Public Methods
	///////////////////////////////////////////////////////////////////////////////////////////////
	uint64 Update(uint64 CurrentTime, void* ref); //Callback for Tick
	void ReInit();

	void AddAttribute(BuffAttribute* Attribute);
	void SetID(uint64 value);
	void SetChildBuff(Buff* value);
	void setTarget(CreatureObject* creature);
	void EraseAttributes();
	bool GetIsMarkedForDeletion();

	//mID is the process ID for the timer
	uint64 GetID();
	//whereas dbId is the Id in the db
	uint64 GetDBID();

	CreatureObject*	GetTarget();
	CreatureObject*	GetInstigator();
	uint64 GetTickLength();
	uint32 GetNoOfTicks();
	uint32 GetCurrentTickNumber();
	uint32 GetIcon();
	string GetName();
	uint64 GetStartGlobalTick();
	void SetInit(bool init);

private:
	///////////////////////////////////////////////////////////////////////////////////////////////
	//Private Member Variables
	///////////////////////////////////////////////////////////////////////////////////////////////

	AttributeList			Attributes;
	string					mName;
	Buff*					mChild;
	Buff*					mParent;
	CreatureObject*			mInstigator;
	CreatureObject*			mTarget;
	int64					mTick; //Length of Tick in ms
	uint64					mDBID;
	uint64					mID;
	uint64					mNextTickLength; //Store the length of the next tick (for when reloading from DB)
	uint64					mStartTime; //GlobalTickCount at Start of Buff
	uint32					mCurrentTick; //Current Tick Number
	uint32					mIcon;
	uint32					mNoTicks; //Total number of Ticks
	bool					mCancelled;
	bool					mDoInit; //Set whether we should do the Init straight away
	bool					mMarkedForDeletion;
	///////////////////////////////////////////////////////////////////////////////////////////////
	//Private Methods
	///////////////////////////////////////////////////////////////////////////////////////////////
	int32					ModifyAttribute(BuffAttributeEnum Type, int32 Value, bool damage = false, bool debuff = false);//returns the amount of the buff that got applied
	void					IncrementTick();
	uint64					GetRemainingTime(uint64 CurrentTime);
	bool					UpdateTick(uint64 CurrentTime);
	void					FinalChanges();		//last change of a buff before deletion
	void					InitialChanges();	//resend icons and attribute changes
	void					InitializeIcons();	//just resend the icons when we rezone before char delete

protected:
	///////////////////////////////////////////////////////////////////////////////////////////////
	//Protected Methods
	///////////////////////////////////////////////////////////////////////////////////////////////
	Buff(CreatureObject* Target, CreatureObject* Instigator, uint NoOfTicks, uint64 Tick, uint32 Icon, uint64 CurrentGlobalTick);
	void SetParent(Buff* value);
};

#endif // ANH_BUFF_H
