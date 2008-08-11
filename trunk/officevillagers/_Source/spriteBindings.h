#ifndef _H_PROTECTOR_SPRBD
#define _H_PROTECTOR_SPRBD
#include "engineBindings.h"

class CSpriteParser: public CAniParser
{
public:
	s32 lSprConnectFrom;
	s32 lSprConnectTo;
	_array_i syncPathes;
	s32 bSyncHead;

	/*s32 lGWarpTime;
	f32 lGWarpAmpl;

	s32 followCursor;
	f32 followCursorSpeed;
	CString sPhysics;

	s32 objinHP;
	s32 objinAttack;
	s32 objinHlBar;
	CString objinType;
	s32 objinBullet;

	s32 align2path;*/

	CSpriteParser()
	{
		lSprConnectFrom=lSprConnectTo=0;
		bSyncHead=0;
		/*align2path=0;
		lGWarpTime=0;
		lGWarpAmpl=0.0f;
		followCursor=0;
		objinHP=0;*/
	};
	~CSpriteParser(){};
	void ParseSprFile(CString& sContentRaw,_array_sc* params, CSprCreation* data);
	void ApplySprFile(CSpriteNode* out, CSprCreation* data);
};

class CDlgSpriteParser: public CDlgParser
{
public:
	CDlgSpriteParser();
	/*_array_v3i bhMarks;
	f32 energyLevel;
	BOOL bShowTutOn1stFire;
	CString sItemsFolder;
	CString sNextLevel;
	_array_v2 mainPath;
	_array_fs fireSpots;
	_array_sc wavesFileList;
	int iCurrentWave;
	DWORD dwLevelState;
	CLevelDescription();
	void OnLevelFinished();
	void OnLevelFailed();
	BOOL isEnoughEnergy();
	BOOL UpdateLevelPaths(int iWave);*/
	virtual void ParseDlgFile(CSpriteNode* out, CString& sDlgContent,_array_sc* aParams);
	virtual void InitializeDlgFile(CString& sDlgContent,_array_sc* aParams);
	virtual void FinalizeDlgFile(CSpriteNode* out, CString& sDlgContent,_array_sc* aParams);
};


class CDialogPauseGuard:public INodeAnimator
{
public:
	CDialogPauseGuard();
	~CDialogPauseGuard();
};

#endif