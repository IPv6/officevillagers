#include "StdAfx.h"
#include "engineBindings.h"
#include "spriteBindings.h"
#include "scriptBindings.h"
#include "LevelLogic/level.h"
#include "LevelLogic/actor.h"
#include "Stuff/animators.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

int g_maxpathFindingsPerFrm=0;
int pathFindingsPerFrm=0;
CGameImplementation* CGameImplementation::inst=0;
CGameImplementation& CGameImplementation::getInst()
{
	return *inst;
}

void CGameImplementation::HandleCommonDescriptionMacros(CString& sContent,_array_sc* params, CSprCreation* data)
{
	if(sContent.Find("${SUBID")!=-1){
		sContent.Replace("${SUBID_BODY}",toString(SUBID_BODY));
		sContent.Replace("${SUBID_HEAD}",toString(SUBID_HEAD));
		sContent.Replace("${SUBID_LABELS}",toString(SUBID_LABELS));
		sContent.Replace("${SUBID_LB_NAME}",toString(SUBID_LB_NAME));
		sContent.Replace("${SUBID_ITEMATTACH}",toString(SUBID_ITEMATTACH));
		sContent.Replace("${SUBID_SHADOW}",toString(SUBID_SHADOW));
		

		sContent.Replace("${SUBID_SITAVATAR}",toString(SUBID_SIT_AVATAR));
		sContent.Replace("${SUBID_SIT}",toString(SUBID_SIT_0));
		sContent.Replace("${SUBID_SITX}",toString(SUBID_SITX_0));
		sContent.Replace("${SUBID_DEFDROP}",toString(SUBID_DEFDROP));
		
	}
}

#define PAUSE_EFFECT_TIME	1000
f32 CGameImplementation::GetTimerSpeed(DWORD dwTimerId)
{
	if(dwTimerId==1)
	{
		f32 res=fGameSpeed;
		if(getLevel() && getLevel()->dwLevelState==LEVELSTATE_PAUSE)
		{
			u32 pauseSetTime=getLevel()->dwLevelStateSetTime;
			u32 tickTime=getTick();
			if(pauseSetTime && tickTime-pauseSetTime<PAUSE_EFFECT_TIME){
				f32 fned=(1.0f-f32(tickTime-pauseSetTime)/f32(PAUSE_EFFECT_TIME));
				res=fGameSpeed*fned;
				if(res<0.2f){
					res=0.0f;
				}
			}else{
				res=0.0f;
			}
		}
		static f32 prev=-1.0f;
		if(prev!=res)
		{
			prev=res;
			if(getLevel())
			{
				getLevel()->OnChangePauseState();
			}
		}
		return res;
	}
	return 1.0f;
}

void CGameImplementation::PrepareScripting()
{
	RegisterGameScriptMethods();
}

CGameImplementation::CGameImplementation()
{
	bRegistered=0;
	fGameSpeed=1.0f;
	inst=this;
}

BOOL CGameImplementation::InitializeRegistrations()
{
	if(getGame().getOptions().lSpecBuildType==0){// Не дебаг и не паблишер
		if(!DRMClient::askServer_canContinue()){
			Alert("You should not run this file. Use links created by installer");
			return FALSE;
		}
		bRegistered=DRMClient::askServer_isRegistered();
	}else{
		bRegistered=true;
	}
	return TRUE;
}

CCustomProfile* CGameImplementation::createCustomProfile()
{
	// Создаем, об удалении заботится движок
	return new CUserStats();
}

CDlgParser* CGameImplementation::createCustomDialogDescriptionParser()
{
	return new CDlgSpriteParser();
}

CAniParser* CGameImplementation::createCustomNodeDescriptionParser()
{
	// Создаем, об удалении заботится движок
	return new CSpriteParser();
}
	
void CGameImplementation::CreateEnvironment()
{
}

void CGameImplementation::DeleteEnvironment()
{
}

CCharPerturbator* CGameImplementation::GetFontEffect(const char* szFontEffectCode)
{
	if(isStringBeginsWith(szFontEffectCode,"typer")){
		CFontEffectTyper* fnt=new CFontEffectTyper();
		fnt->sAutoMarker=GetInstringPart("[marker:", "]", szFontEffectCode);
		return fnt;
	}
	/*if(isStringBeginsWith(szFontEffectCode,"brake")){
		return new CFontEffectMoveBrake();
	}
	if(isStringBeginsWith(szFontEffectCode,"mistiqueIn")){
		return new CFontEffectMistiqueIn();
	}
	if(isStringBeginsWith(szFontEffectCode,"jumper")){
		return new CFontEffectJumper();
	}*/
	return 0;
}

void CGameImplementation::OnUpdateFrame()
{
#ifdef _DEBUG
	static BOOL b=0;
	if(!b)
	{
		b=1;
		AddDebugScreenLine(toString("Early prealpha. Build '%s'",__TIMESTAMP__));
	}
#endif
	BOOL bClickMode=getGame().eventHandler.bMouseState_OnClick;
	int iPlayAnimation=bClickMode;
	if(getUserFocus().getLastNodeMouseOver())
	{
		//if(!getLevel() || getUserFocus().getLastNodeMouseOver()!=getLevel()->floorNodeID)
		s32 dwMO=getUserFocus().getLastNodeMouseOver();
		int iNum=CMouseEventBlocker::meBlockerIDs.linear_reverse_search(dwMO);
		if(iNum==-1)
		{
			// Игрок держит курсор над активной кнопкой
			iPlayAnimation=1;
		}
	}
	getGame().setCursor(bClickMode+1,iPlayAnimation?TRUE:FALSE);

	if(pathFindingsPerFrm>g_maxpathFindingsPerFrm){
		g_maxpathFindingsPerFrm=pathFindingsPerFrm;
	}
	pathFindingsPerFrm=0;
	return;
}

bool CGameImplementation::HandleEvent(const SEvent& event)
{
	return false;
}

void CGameImplementation::HandleGameStateChange(int iNewState)
{
	if(iNewState==GAMESTATE_DYING){
#ifdef _DEBUG
		CFlowTimer::AddCustomInfo(">>> MaxPathsearches per frame",toString(g_maxpathFindingsPerFrm));
#endif
	}
}
