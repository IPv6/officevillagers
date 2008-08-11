#ifndef _H_PROTECTOR_GAMEIMPL
#define _H_PROTECTOR_GAMEIMPL
#include "../../_Source/_gameEngine/_irrDefines.h"
#include "../../_Source/_gameEngine/_drmclient.h"
#include "../../_Source/_gameEngine/_utils/_utils.h"
#include "../../_Source/_gameEngine/_irrExtensions/_irrExtensions.h"
#include "../../_Source/_gameEngine/_basicAnimators/_basicAnimators.h"
#include "../../_Source/_gameEngine/Server_Game.h"

#define ACTORNODEW_ID_MIN	USERDEFINEDID1_MIN
#define ACTORNODEW_ID_MAX	USERDEFINEDID1_MIN+500

class CUserStats: public CCustomProfile
{
public:
	//CString sCurrentLevel;
	void InitializeProfile(CString& sIniFile,CUserProfile* profile){};
	void LoadProfile(CString& sIniFile,CUserProfile* profile){};
	void SaveProfile(CString& sIniFile,CUserProfile* profile){};
	void NotificationProfileSwitched(){};
	//SaveLoad
};

class CGameImplementation: public CGameRotorClient
{
public:
	static CGameImplementation* inst;
	static CGameImplementation& getInst();
	f32 fGameSpeed;

	BOOL bRegistered;
	CGameImplementation();
	//////////////
	// Перегрузки
	///////////////
	// Клиентский обработчик профиля игрока
	virtual CCustomProfile* createCustomProfile();
	// Клиентский обработчик описания диалога (можно использовать для описания уровня игры например и т.п.)
	virtual CDlgParser* createCustomDialogDescriptionParser();
	// Клиентский обработчик описания ноды сцены (можно использовать для описания итемов уровня и т.п.)
	virtual CAniParser* createCustomNodeDescriptionParser();
	// Обработчик описаний: клиентские макросподстановки
	virtual void HandleCommonDescriptionMacros(CString& sContent,_array_sc* params, CSprCreation* data);
	// Таймеры
	virtual f32 GetTimerSpeed(DWORD dwTimerId);
	// Обработка DRM
	virtual BOOL InitializeRegistrations();
	// Запуск скриптового движка. Добавление своим методов в него и т.д.
	virtual void PrepareScripting();
	
	virtual void CreateEnvironment();
	virtual void DeleteEnvironment();
	
	virtual void OnUpdateFrame();
	
	virtual bool HandleEvent(const SEvent& event);
	virtual CCharPerturbator* GetFontEffect(const char* cFontCode=0);
};


#endif