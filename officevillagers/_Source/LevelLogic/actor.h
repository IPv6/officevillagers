#ifndef _H_PROTECTOR_LEVELACTOR
#define _H_PROTECTOR_LEVELACTOR
#include "../engineBindings.h"
#include "navigation.h"
#include "actorItem.h"

#define SUBID_BODY					10000
#define SUBID_HEAD					10001
#define SUBID_LABELS				10002
#define SUBID_LB_NAME				10003
#define SUBID_ITEMATTACH			10004
#define SUBID_SHADOW				10005
#define SUBID_SIT_AVATAR			10099
#define SUBID_SIT_0					10100
#define SUBID_SIT_MAX				10109
#define SUBID_SITX_0				10110
#define SUBID_SITX_MAX				10119
#define SUBID_DEFDROP				10200

#define BY_STOP_MOTION				10
#define BY_STOP_LEVELEND			11

#define DEF_ACTOR_Z					-0.001f

#define DEF_NONEXISTABLEACTOR		"NONEXISTABLENAME!"

class CAction;
class CProphession: public StorageNamed::CXmlStruct
{
public:
	CProphession()
	{
		lCounter=0;
		bChangeNavigation=0;
		lNoPocketPosSync=0;
	}
	CString sName;
	CString sBaseName;
	long iBodySpr;
	long lNoThink;
	long lNoPocketPosSync;
	long lCounter;
	long bChangeNavigation;//0-не меняет проходимость. 1-меняет, автогенерация подходов. 2-меняет, без автогенерации подходов (стены)
	float w;
	float h;
	MAP2XML_BEGIN("Prophession")
		MAP2XML (&sName, "Name")
		MAP2XML_DEF (&sBaseName, "BaseName", "")
		MAP2XML_DEF (&iBodySpr, "BodySpr", -1)
		MAP2XML_DEF (&lNoThink, "NoThink", 0)
		MAP2XML_DEF (&lNoPocketPosSync, "NoPocketPosSync", 0)
		MAP2XML_DEF (&bChangeNavigation, "ChangeNavigation", 0)
		MAP2XML_DEF (&w, "defWidth", 2.0f)
		MAP2XML_DEF (&h, "defHeight", 2.0f)
	MAP2XML_END()
};
typedef core::array<CProphession*> _array_CProphession;

class CNextActionOrder
{
public:
	CNextActionOrder()
	{
		sActionToSwitchTo="";
		bManual=0;
	}
	CString sActionToSwitchTo;
	BOOL bManual;
	CNextActionOrder& CNextActionOrder::operator=(const CNextActionOrder& other)
	{
		sActionToSwitchTo=other.sActionToSwitchTo;
		bManual=other.bManual;
		return *this;
	}
};

#define NO_ACTION "#NOACTION!#"
class CActor: public irr::IReferenceCounted, public StorageNamed::CXmlStruct
{
public:
	void Init();
	CActor(){Init();};
	~CActor();
	//===================
	//===================
	// Сохраняемые данные
	struct CSerializableData
	{
		CString		p_sName;// Уникальное в рамкам всего-всего имя
		CString		p_sNameLocalized;
		long		p_Enabled;
		long		p_iBody;
		long		p_iHead;
		long		p_iBodyLock;//У перса 0-от професси меняется тело,1-не меняется. У объекта 0 - локейшн не влияет на проходимость, 1- клияет
		long		p_iActorType;//-1-Офис, глобальный, 0-Неподвижный, 1-Подвижный
		f32			p_fCreationTime;
		_v2			p_pos;
		CString		p_posLocator;
		CString		p_actorLocationInitial;
		CString		p_actorLocationEntrance;

		f32			p_speedRate;
		f32			p_CurrentFaceDirection;
		_v2			p_CurrentBodyDirection;
		CString		p_Sprite;
		CString		p_CurrentActionName;
		f32			fActionBeginTime;
		CString		p_ProfessionName;
		CString		p_ValidProphession;
		CString		p_FollowToActor;
		CUniOptionsHolder p_Attributes;
		CString		p_AttributesAsStr;// Паковка в строку...
		CString		p_AttributesOverload;// Хардкод некоторых левых значений...
		_array_CItemP p_items;// Важен именно масиив и порядок...
		CString		p_sScriptOnCreate;
		CString		p_sScriptOnDelete;
		CString		p_sScriptOnEnable;
		CString		p_sScriptOnDisable;

		CString		p_sInPocket_Actor;
		CString		p_sInPocket_Item;

		void CopyFrom(const CActor& other)
		{
			// Копируем только data!
			p_sName=other.data.p_sName;
			p_sNameLocalized=other.data.p_sNameLocalized;
			p_Enabled=other.data.p_Enabled;
			p_iBody=other.data.p_iBody;
			p_iHead=other.data.p_iHead;
			p_iBodyLock=other.data.p_iBodyLock;
			p_iActorType=other.data.p_iActorType;
			p_fCreationTime=other.data.p_fCreationTime;
			p_pos=other.data.p_pos;
			p_posLocator=other.data.p_posLocator;
			p_actorLocationInitial=other.data.p_actorLocationInitial;
			p_actorLocationEntrance=other.data.p_actorLocationEntrance;
			p_speedRate=other.data.p_speedRate;
			p_CurrentFaceDirection=other.data.p_CurrentFaceDirection;
			p_CurrentBodyDirection=other.data.p_CurrentBodyDirection;
			p_Sprite=other.data.p_Sprite;
			p_CurrentActionName=other.data.p_CurrentActionName;
			fActionBeginTime=other.data.fActionBeginTime;
			p_ProfessionName=other.data.p_ProfessionName;
			p_ValidProphession=other.data.p_ValidProphession;
			p_FollowToActor=other.data.p_FollowToActor;
			p_AttributesAsStr=other.data.p_AttributesAsStr;
			p_sScriptOnCreate=other.data.p_sScriptOnCreate;
			p_sScriptOnDelete=other.data.p_sScriptOnDelete;
			p_sScriptOnEnable=other.data.p_sScriptOnEnable;
			p_sScriptOnDisable=other.data.p_sScriptOnDisable;
			p_sInPocket_Actor=other.data.p_sInPocket_Actor;
			p_sInPocket_Item=other.data.p_sInPocket_Item;
		}
	} data;
	long iActorID;// Уникальный в рамках игросессии код
	SquirrelObject scriptMirror;
	BOOL		actionAttachDone;
	CAction*	p_CurrentAction;
	CProphession* p_Profession;
	CLocation*	actorLocationLocator;
	CString		sActorStatusTextOverload;
	void		SetStatusTextOverload(const char* sz);
	long		_autoClone;
	_v3			posOffset;
	MAP2XML_BEGIN("Actor")
		MAP2XML		(&data.p_sName, "Name")
		MAP2XML_DEF (&data.p_sNameLocalized, "NameLocalized", "")
		MAP2XML_DEF (&data.p_Enabled, "Enabled", 1)
		MAP2XML_DEF (&data.p_Sprite, "Sprite", "")
		MAP2XML_DEF (&data.p_iBody, "Body", 0)
		MAP2XML_DEF (&data.p_iHead, "Head", 0)
		MAP2XML_DEF (&data.p_iBodyLock, "BodyLock", 0)
		MAP2XML_DEF (&data.p_pos.X, "Position.x", 0)
		MAP2XML_DEF (&data.p_pos.Y, "Position.y", 0)
		MAP2XML_DEF (&data.p_posLocator, "Position.Location", "")
		MAP2XML_DEF (&data.p_actorLocationInitial, "Position.LocationInit", "")
		MAP2XML_DEF (&data.p_actorLocationEntrance, "Position.LocationEntr", "")
		MAP2XML_DEF (&data.p_iActorType, "ActorType", 0)
		MAP2XML_DEF (&data.p_fCreationTime, "CreationTime", -1.0f)
		MAP2XML		(&data.p_ProfessionName, "ActorProphession")
		MAP2XML_DEF (&data.p_ValidProphession, "ActorValidProphession", "NONE")
		MAP2XML_DEF (&data.p_FollowToActor, "FollowToActor", "")
		MAP2XML_DEF (&data.p_sInPocket_Actor, "InPocket_Actor", "")
		MAP2XML_DEF (&data.p_sInPocket_Item, "InPocket_Item", "")
		MAP2XML_DEF (&data.p_CurrentActionName, "CurrentAction", "")
		MAP2XML_DEF (&data.p_AttributesAsStr, "Attributes", "")
		MAP2XML_DEF (&data.p_AttributesOverload, "AttributesOverload", "")
		MAP2XML		(&data.p_items, "Items")
		MAP2XML_DEF (&_autoClone, "AutoClone", 0)
		MAP2XML_DEF (&data.p_sScriptOnCreate, "ScriptOnCreate", "")
		MAP2XML_DEF (&data.p_sScriptOnDelete, "ScriptOnDelete", "")
		MAP2XML_DEF (&data.p_sScriptOnEnable, "ScriptOnEnable", "")
		MAP2XML_DEF (&data.p_sScriptOnDisable, "ScriptOnDisable", "")
	MAP2XML_END()
	void PrepareSerialization();
	void ApplySerialization();
	//===================
	//===================
	BOOL isError;
	BOOL bNodeSpawned;
	BOOL isInteractive;
	BOOL GetInteractive();
	void SetInteractive(BOOL bValue);
	void SetProf(const char* szProf,BOOL bUIOnly);
	void SetLocation(CLocation* loc);
	void SetLocation(const _v2& newPos);
	void SetEnable(BOOL bEnable,BOOL bForceScriptUpdate);
	float GetWalkSpeed();
	BOOL AI_TuneCrowdBehaviour(_v2& fromp, _v2& point, _rctf& safeZone);
	CCriticalSection csactiveNavigationMap;

	CNextActionOrder nextAction;
	CWalkData walkData;

	void StopBodyMotion(BOOL bLevelEnd=FALSE);
	void StopBodyMotionUI(BOOL bLevelEnd=FALSE);
	u32 lastThinkTime;

	void SpawnPersUI();
	void SetBodyZeroSprite();
	void RefreshBodyAccordingDirection(_v2 direction,BOOL bWalking);
	void RefreshHeadAccordingDirection(float fBodyAddonAngle,int iAngleShift=0, BOOL bRelative=1);
	void ReattachItems();

	void Think(u32 timeMs,BOOL bFastForward=0);
	void ThinkMovements(u32 timeMs);
	void ThinkFollower();
	void ThinkActions();
	void ThinkItems();
	BOOL ThinkPocket(BOOL bForceUpdate=FALSE);

	// Локация - цель. юзаем динамические координаты
	CString	activeNavigationDotLocationName;// По идее надо запоминать. НО! с учетом того, что при загрузке степ начинается сначала, activeNavigationDotLocationName при активации действия
	_v2 lastFollowerToPosition;
	CSpriteNode* node;
	BOOL bMovementsIsPaused;
	int  bMovementsIsPausedToTime;
#ifdef _DEBUG
	CString dbgActionHistory;
#endif
	CActor* lastInPocketActor;
	u32 lastHeadYawTime;
	void ShowActorLabels(BOOL b, CNodeBasement* onNode=0);
	BOOL bHasBodyAndCanMove;
	_v2 getPosition();
	void AI_SyncPosition(BOOL bForceUpdate=0);//0-взять координаты, 1-установить координаты в спрайт, 2-синхронизировать локатор наплевав на тип

	BOOL AI_SetMove2TargetPos(_v2 newpos);
	BOOL AI_SetMove2TargetPos(const char* szLocation);
	int AI_PauseMovements(BOOL bPuase);
	void PauseMovementsForSec(int iSecs);
	BOOL AI_MakeWalkablePathTo(_v2 targetPosition);
	BOOL AI_FinishWalkablePathTo();
	BOOL AI_SwitchToAction(const char* szActionName,BOOL bManual, BOOL bNOW=FALSE);
	BOOL AI_SetFollowTo(CActor* targetAuthor);
	BOOL AI_EnsureMinDist2Actor(CActor* movableActor, f32 dist,bool forceDistAsNeared);
	BOOL SetActionAccordingOrder();

	BOOL AddAttribute(const char* szName, const char* value);
	BOOL AddAttribute(const char* szName, long value);
	BOOL AddAttribute(const char* szName, f32 value);
	CString GetAttributeStr(const char* szName);
	long GetAttributeLong(const char* szName);
	f32 GetAttributeF32(const char* szName);
	BOOL DelAttribute(const char* szName);

	BOOL AddItem(const char* szName,const char* szParamsRaw,const char* szCustomParam);
	BOOL DelItem(const char* szName);
	BOOL DelItem(CActorItem* p);
	CActorItem* GetItemByName(const char* szName);
	CActorItem* GetItemByIndex(int iIndex);
	CActorItem* GetItemByMask(const char* szName);
	

	BOOL PutInPocket(const char* szToActor,const char* szToItem);
	BOOL MakePocketEmpty();
};
#endif