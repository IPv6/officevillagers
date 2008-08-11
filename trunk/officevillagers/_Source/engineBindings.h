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
	// ����������
	///////////////
	// ���������� ���������� ������� ������
	virtual CCustomProfile* createCustomProfile();
	// ���������� ���������� �������� ������� (����� ������������ ��� �������� ������ ���� �������� � �.�.)
	virtual CDlgParser* createCustomDialogDescriptionParser();
	// ���������� ���������� �������� ���� ����� (����� ������������ ��� �������� ������ ������ � �.�.)
	virtual CAniParser* createCustomNodeDescriptionParser();
	// ���������� ��������: ���������� �����������������
	virtual void HandleCommonDescriptionMacros(CString& sContent,_array_sc* params, CSprCreation* data);
	// �������
	virtual f32 GetTimerSpeed(DWORD dwTimerId);
	// ��������� DRM
	virtual BOOL InitializeRegistrations();
	// ������ ����������� ������. ���������� ����� ������� � ���� � �.�.
	virtual void PrepareScripting();
	
	virtual void CreateEnvironment();
	virtual void DeleteEnvironment();
	
	virtual void OnUpdateFrame();
	
	virtual bool HandleEvent(const SEvent& event);
	virtual CCharPerturbator* GetFontEffect(const char* cFontCode=0);
};


#endif