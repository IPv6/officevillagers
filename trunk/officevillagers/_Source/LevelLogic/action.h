#ifndef _H_PROTECTOR_ACTION
#define _H_PROTECTOR_ACTION
#include "../engineBindings.h"
#include "navigation.h"
#include "actor.h"

class CActionLookupParams
{
public:
	CActionLookupParams()
	{
		memset(this,0,sizeof(CActionLookupParams));
	}
	int iMinPriority;
};

class CAction;
typedef core::array<CAction*> _array_CActionsP;
class CAction
{
public:
	// Данные. Коприуютс оператором присваивания!
	CString p_sActionName;// Уникальное имя, ключ
	CString p_sNameForHUD;// имя, которое пишется когд перс выполняет это действие
	_array_CProphession p_PrqProphessions;
	_array_CLocationsP p_PrqLocations;
	_array_sc p_PrqActionsNot;
	_array_sc p_PrqActionsOk;
	_array_sc p_PrqOfficeAttrsOk;

	BOOL p_PrqCanAffectBusyActors;
	int p_PrqIncutscene;
	CString sScriptBegin;
	CString sScriptIn;
	CString sScriptIn2;
	CString sScriptEnd;
	CAction* m_parentAction;
	BOOL p_bProtected;
	BOOL p_bDisableUI;
	BOOL p_iClearOnSave;
	int p_iPriority;
	int p_iThinkSpeed;
	int p_UniqueUsage;
	float p_WalkSpeed;

	int iActionUsageCount;
	u32 lastActivationTime;
	SquirrelObject actionScriptMirror;
	// Конструктор
	CAction();
	~CAction();
	// Методы
	void CopyFrom(CAction& other,BOOL bNonUniqOnly);
	static CAction* Add2LevelFromDescription(const CString& sDsc, const char* szFile);
	// Ивент и несколько степов
	static CAction* Add2LevelFromDescriptionSingleSet(const CString& sDsc, const char* szFile, int iStepNum);
	void UpdateScriptMirror();
	BOOL OnDetach(CActor* who);
	BOOL OnAttach(CActor* who);
	BOOL OnDuring(CActor* who);
	static CAction* FindBestActionForNow(CActor* who);
	static CAction* FindBestActionForNow(CActor* who, const CActionLookupParams& params);
private:
	CAction& operator=(const CAction& other);// Блокируем копирование
};

#endif