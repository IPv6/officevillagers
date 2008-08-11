#include "StdAfx.h"
#include "spriteBindings.h"
#include "LevelLogic/level.h"
#include "LevelLogic/actor.h"
#include "Stuff/animators.h"
//#include "../../_Source/_gameEngine/Module_MouseKeyb.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

void CSpriteParser::ParseSprFile(CString& sContentRaw,_array_sc* params, CSprCreation* data)
{
	// Мои аниматоры
	CString s;
	if(isParam(sContentRaw,"-Animation.ConnectSprites:",s)){
		lSprConnectFrom=getS32(s,'f');
		lSprConnectTo=getS32(s,'t');
	}
	if(isParam(sContentRaw,"-ActorBodySync:",s)){
		CString sPathes=GetInstringPart("path[","]",s);
		syncPathes=Str2IArray(sPathes,',',FALSE,FALSE);
	}
	if(isParam(sContentRaw,"-ActorHeadSync:",s)){
		bSyncHead=getU32(s,0);
	}
	/*if(isParam(sContentRaw,"-Animation.GeometryWarp:",s)){
		lGWarpTime=getS32(s,'t');
		lGWarpAmpl=getF32(s,'a');
	}
	if(isParam(sContentRaw,"-Animation.Align2Path:",s)){
		align2path=getS32(s,0);
	}
	if(isParam(sContentRaw,"-Animation.Physics2D:",s)){
		sPhysics=s;
	}

	if(sContentRaw.Find("-GameLogic-")==-1){
		return;
	}
	if(isParam(sContentRaw,"-GameLogic-ObjectInfo:",s)){
		objinHP=getS32(s,'h');
		objinAttack=getS32(s,'a');
		objinHlBar=getS32(s,'b');
		objinType=getSTR(s,'t');
	}
	if(isParam(sContentRaw,"-GameLogic-FollowCursor:",s)){
		followCursor=1;
		followCursorSpeed=getF32(s,'s');
	}*/
}

void CSpriteParser::ApplySprFile(CSpriteNode* out, CSprCreation* data)
{
	/*if(lGWarpTime>0){
		attachAnimator(out,new CGeometryWarp(lGWarpTime,lGWarpAmpl));
	}
	if(followCursor){
		attachAnimator(out,new CFollowCursor(followCursorSpeed));
	}
	if(align2path){
		attachAnimator(out,new CRotate2Direction());
	}
	if(objinHP){
		attachAnimator(out,new CGameObjectHandler(objinHP,objinAttack,objinType,objinHlBar));
	}
	if(sPhysics.GetLength())
	{
		//CPhysics2d::getInstance()->;
		attachAnimator(out,new CPhysicsAnimator(sPhysics));
	}*/
	if(lSprConnectFrom)
	{
		attachAnimator(out,new CSpriteConnector(lSprConnectFrom,lSprConnectTo));
	}
	if(syncPathes.size())
	{
		attachAnimator(out,new CSpriteBodySync(syncPathes));
	}
	if(bSyncHead)
	{
		attachAnimator(out,new CSpriteHeadSync());
	}
}


CDlgSpriteParser::CDlgSpriteParser()
{
}

BOOL& isLevelLoading();
void CDlgSpriteParser::InitializeDlgFile(CString& sDlgContent,_array_sc* aParams)
{
	if(sDlgContent.Find("-Office")==-1){
		// Это не описание уровня
		return;
	}
	isLevelLoading()=TRUE;
}

void CDlgSpriteParser::FinalizeDlgFile(CSpriteNode* out, CString& sDlgContent,_array_sc* aParams)
{
	if(sDlgContent.Find("-Office")==-1){
		// Это не описание уровня
		return;
	}
	isLevelLoading()=FALSE;
}

void CDlgSpriteParser::ParseDlgFile(CSpriteNode* out, CString& sDlgContent, _array_sc* aParams)
{
	CString s;
	if(isParam(sDlgContent,"-PauseGame:",s)){
		int isPause=getU32(s,0);
		if(isPause){
			attachSysAnimator(out,new CDialogPauseGuard());
		}
	}
	if(sDlgContent.Find("-Office")==-1){
		// Это не описание уровня
		return;
	}
	#ifdef _DEBUG
	getGame().ClearAllCaches(TRUE);
	#endif
	CLevel::LoadLevelFromDescription(out,sDlgContent);
}

CDialogPauseGuard::CDialogPauseGuard()
{
	makeSystem();
	if(getLevel() && getLevel()->dwLevelState>=LEVELSTATE_INGAME_ACTIVE && getLevel()->dwLevelState<LEVELSTATE_PAUSE)
	{
		getLevel()->dwLevelState=LEVELSTATE_PAUSE;
		getLevel()->dwLevelStateSetTime=getTick();
	}
}

CDialogPauseGuard::~CDialogPauseGuard()
{
	if(getLevel() && getLevel()->dwLevelState==LEVELSTATE_PAUSE)
	{
		getLevel()->dwLevelState=LEVELSTATE_INGAME_ACTIVE;
		getLevel()->dwLevelStateSetTime=getTick();
	}
}