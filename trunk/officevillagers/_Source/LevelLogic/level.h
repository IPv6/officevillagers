#ifndef _H_PROTECTOR_LEVELBIND
#define _H_PROTECTOR_LEVELBIND
#include "../engineBindings.h"
#include "navigation.h"
#include "action.h"
#include "actor.h"
#include "actorItem.h"

#define THINKING_DISCRETION			500
#define THINKING_LEVEL_DISCRETION	100
// До загрузки
#define LEVELSTATE_INIT				0
// Загрузилось, инициализируется
#define LEVELSTATE_BEFORGM			1
// Работает, но фаст форвард
#define LEVELSTATE_INGAME			2
// Активный этап, все работает
#define LEVELSTATE_INGAME_ACTIVE	3
// Пауза
#define LEVELSTATE_PAUSE			4
#define OFFICE_ACTOR		"Office"
#define DELETEDACTOR_MARK	"[DELETED]"
class CLevel: public irr::IReferenceCounted
{
public:
	struct CLevelSaveData: public StorageNamed::CXmlStruct
	{
		BOOL bFaked;
		CLevelSaveData()
		{
			bFaked=0;
			bDirtySave=0;
			pMainOfficeActor=0;
		}
		void ApplySerialization();
		void PrepareSerialization();
		void ClearActors();

		CArray< CActor*, CActor* > actors;
		long lProgress_Max;
		long lProgress_Cur;
		long bDirtySave;
		f32 lProgress_TimeMax;
		f32 lProgress_TimeNow;
		f32 fProgress_Crunch;
		f32 lProgress_TimeAbs;
		CString sMagazinName;
		CString sBaseSaveFile;
		CString sSafeFile;
		SquirrelObject scriptSafe;
		MAP2XML_BEGIN("Office")
			MAP2XML (&lProgress_Max, "ProgressMax")
			MAP2XML (&lProgress_Cur, "ProgressCur")
			MAP2XML (&lProgress_TimeNow, "ProgressTime_Now")
			MAP2XML (&lProgress_TimeMax, "ProgressTime_Max")
			MAP2XML (&fProgress_Crunch, "ProgressTime_Crunch")
			MAP2XML_DEF (&bDirtySave, "DirtySave", 0)
			MAP2XML_DEF (&lProgress_TimeAbs, "LevelAbsoluteTime", 0.0f)
			MAP2XML_DEF (&sBaseSaveFile, "BaseSaveFile", "")
			MAP2XML_DEF (&sMagazinName, "MagazinName", "Daily Times")
			MAP2XML (&actors, "Actors")
			MAP2XML_DEF (&sSafeFile, "SquirrelSafe", "")
		MAP2XML_END()
		CActor* pMainOfficeActor;
	} data;
	BOOL OnLevelClose();
	BOOL SaveGame(const char* szPostfix);
	void ResetOfficeObject();
	void InitOfficeObject();
	void UpdateProgress();
	BOOL bIsCutscene;
	BOOL bCutsceneType;
	BOOL EnableCutscene(BOOL b);
	BOOL isCrunch;
	f32 fProgressCur;
	f32 fProgressTime;
	CSliderControl* progressCurSlider;
	CSliderControl* progressCruSlider;
	CSliderControl* progressTimSlider;
	int timeMsPrevious;
	// Действия
	_array_CActionsP actions;
	struct CSerializableDataItems: public StorageNamed::CXmlStruct
	{
		// Итемы
		_array_CItemDscP itemDsc;
		MAP2XML_BEGIN("LevelItems")
			MAP2XML (&itemDsc, "Items")
		MAP2XML_END()
	} items;

	// Навигация
	CLocationManagerNavigator* levelNavigator;

	// По необходимости перевести на mapы! //TODO
	CProphession* getProphessionByName(const char* szName);
	CLocation* getLocationByName(const char* szName);
	CAction* getActionByName(const char* szName);
	CActor* getActorByName(const char* szName);
	CActor* getActorByID(int iID);
	CActorItemDesc* getItemDscByName(const char* szName);
	// Разное
	CAction*& getCurrentAction();
	struct CSerializableLevelInfo: public StorageNamed::CXmlStruct
	{
		_array_CProphession prophessions;
		CString sOfficeMapDescription;
		CString sOfficeNamedLocations;
		CString sOfficeMapNavi;
		CString sOfficeActionDsc;
		CString sOfficeDefaultState;
		CString sOfficeItemsDsc;
		float fBody2SpeedRate;
		float fMove2SpeedRate;
		void ApplySerialization();
		MAP2XML_BEGIN("LevelInfo")
			MAP2XML (&sOfficeMapDescription, "OfficeTexture")
			MAP2XML (&sOfficeNamedLocations, "NamedLocations")
			MAP2XML (&sOfficeMapNavi, "NavigationMap")
			MAP2XML (&sOfficeActionDsc, "ActionsDescription")
			MAP2XML (&sOfficeDefaultState, "DefaultState")
			MAP2XML (&sOfficeItemsDsc, "ItemDescriptions")
			MAP2XML_DEF (&fBody2SpeedRate, "Body2SpeedRate", 550.0f)
			MAP2XML_DEF (&fMove2SpeedRate, "Move2SpeedRate", 270.0f)
			
			MAP2XML (&prophessions, "Prophessions")
			
		MAP2XML_END()
	} dataLevelInfo;

	CSpriteNode* levelRoot;
	CSpriteNode* floor;
	DWORD floorNodeID;
	DWORD dwLevelState;
	u32 dwLevelStateSetTime;
	BOOL bLoadingFromDefaultState;
	BOOL bRecalConnections;
	void ScheduleRecalcConnections(){bRecalConnections=TRUE;};
	CLevel();
	~CLevel();
	void Think(u32 timeMs,BOOL bFastForward=0);
	static void LoadLevelFromDescription(CSpriteNode* out,const char* sz);
	s32 cursorOnScreenSide;
	void OnChangePauseState();
	void CheckLokReashability();
	void UpdateDynLocations();
	// Хинты
	struct CSingleHint
	{
		char szText[MAX_PATH];
		f32 fTime;
		f32 fTimeTo;
		int iPriority;
		DWORD dwColor;
		BOOL bCloseBaton;
		CSingleHint()
		{
			memset(this,0,sizeof(CSingleHint));
		}
	};
	typedef core::array<CSingleHint> _array_Hints;
	_array_Hints aHints;
	void AddHint(CString sText="", f32 fTime=0.0f, int iPriority=0, DWORD dwColor=0, BOOL bCloseBaton=FALSE);
	void HideHint(BOOL bLastOnly, int iPriority);
	void ActualizeHint();
	// Актеры
	CArray< CActor*, CActor* > actorsToDelete;
	CActor* CreateActor(const char* szTemplate,const char* szNameOverload,const char* sOptions,const char* szLoc);
	CActor* CreateActor(CActor* actor);
	BOOL DeleteActor(CActor* actor);
};

class CLevelThinker:public INodeAnimator
{
	CLevel* lvl;
public:
	u32 levelInitTime;
	static CLevelThinker*& getThinker()
	{
		static CLevelThinker* lf=0;
		return lf;
	}
	int pauseInitOk;
	CLevelThinker(CLevel* _lvl);
	~CLevelThinker();
	void animateNodeX(CNodeBasement* node, f32 delta, u32 timeMs);
	void finalize();
	u32 getGameTimer();
};

f32 getTime();
f32 getIngameTimeAbs();
CLevel*& getLevel();
CString GetStringIncEd(CString sCurrent, int* iWithIndex=NULL);
#endif