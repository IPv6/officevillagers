#include "StdAfx.h"
#include "scriptBindings.h"
#include "LevelLogic/navigation.h"
#include "LevelLogic/level.h"
#include "LevelLogic/actor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

CLocation* getActorLocation(const char* sByName)
{
	CActor* pActor=getLevel()->getActorByName(sByName);
	if(pActor){
		pActor->AI_SyncPosition(0);
		if(pActor->data.p_actorLocationEntrance.GetLength()){
			return getLocationPrefixed(pActor->data.p_actorLocationEntrance);
		}else{
			return pActor->actorLocationLocator;
		}
	}
	return NULL;
}

CLocation* getLocation(StackHandler& sa, int iParaNum)
{
	CString sByName;
	int iType=sa.GetType(iParaNum);
	if(iType==OT_STRING){// по имени
		sByName=sa.GetString(iParaNum);
	}
	if(iType==OT_TABLE){// по имени
		SquirrelObject sqo=sa.GetObjectHandle(iParaNum);
		CString sName=sqo.GetString(DEPRECATION1);
		if(sName.GetLength()){
			CString sPrefix=getCurrentLocationPrefix();
			float fx=sqo.GetFloat("_x");
			float fy=sqo.GetFloat("_y");
			CLocation* newLoc=new CLocation();
			newLoc->szName=sName;
			if(sPrefix.GetLength() && sByName.Find("::")==-1){
				newLoc->szName=sPrefix+"::"+newLoc->szName;
			}
			CString sNamePers=sqo.GetString("_relative");
			if(sNamePers.GetLength()){
				CActor* act= getLevel()->getActorByName(sNamePers);
				if(act){
					fx+=act->data.p_pos.X;
					fy+=act->data.p_pos.Y;
				}else{
					CLocation* loc=getLocationPrefixed(sNamePers);
					if(loc){
						fx+=loc->x;
						fy+=loc->y;
					}
				}
			}
			newLoc->x=fx;
			newLoc->y=fy;
			newLoc->w=1;
			newLoc->h=1;
			newLoc->bScaled=1;
			sByName=newLoc->szName;
			CLocationManager::getInstance(CurrentLocManager()).AddLocator(newLoc);
		}else{
			// Честное
			SquirrelObject sqo=sa.GetObjectHandle(iParaNum);
			// Если это перс... то к нему!
			CString sIsProf=sqo.GetString("Profession");
			if(sIsProf.GetLength()){
				sByName=sqo.GetString("Name");
				return getActorLocation(sByName);
			}else{
				sByName=sqo.GetString("_name");
			}
		}
	}
	return getLocationPrefixed(sByName);
}

int nloc_Load(HSQUIRRELVM v)
{
	StackHandler sa(v);
	int iType=sa.GetInt(SQ_PARAM1);
	CurrentLocManager()=iType;
	CString sFile=sa.GetString(SQ_PARAM2);
	CLocationManager& mn=CLocationManager::getInstance(iType);
	mn.Load(sFile);
	SquirrelObject result=SquirrelVM::CreateArray(mn.locations.size());
	for(u32 i=0;i<mn.locations.size();i++)
	{
		SquirrelObject item=SquirrelVM::CreateTable();
		item.SetValue("_isdot",true);
		item.SetValue("_x",mn.locations[i]->x);
		item.SetValue("_y",mn.locations[i]->y);
		item.SetValue("_w",mn.locations[i]->w);
		item.SetValue("_h",mn.locations[i]->h);
		item.SetValue("_name",mn.locations[i]->szName.GetBuffer(0));
		result.SetValue(i,item);
	}
	return sa.Return(result);
}

int nloc_Remove(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CString sName=sa.GetString(SQ_PARAM1);
	CLocationManager& mn=CLocationManager::getInstance(CurrentLocManager());
	while(1){
		if(!mn.Remove(sName)){
			break;
		}
	}
	return sa.Return(true);
}

int nloc_Add(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CString sName=sa.GetString(SQ_PARAM1);
	CLocationManager& mn=CLocationManager::getInstance(CurrentLocManager());
	CLocation* newOne=new CLocation();
	newOne->szName=sName;
	newOne->x=sa.GetFloat(SQ_PARAM2);
	newOne->y=sa.GetFloat(SQ_PARAM3);
	newOne->w=sa.GetFloat(SQ_PARAM4);
	newOne->h=sa.GetFloat(SQ_PARAM5);
	newOne->bScaled=0;
	mn.AddLocator(newOne);
	return sa.Return(true);
}


int nloc_Save(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CLocationManager& mn=CLocationManager::getInstance(CurrentLocManager());
	CString sFile=sa.GetString(SQ_PARAM1);
	mn.sOriginalFile=getDataPath(sFile);
	mn.Save();
	return sa.Return(true);
}


int game_SetGameSpeed(HSQUIRRELVM v)
{
	StackHandler sa(v); 
	f32 f1=sa.GetFloat(SQ_PARAM1);
	CGameImplementation::getInst().fGameSpeed=f1;
	if(CGameImplementation::getInst().fGameSpeed<0.0f)
	{
		CGameImplementation::getInst().fGameSpeed=0.0f;
	}
	if(CGameImplementation::getInst().fGameSpeed>500.0f)
	{
		CGameImplementation::getInst().fGameSpeed=500.0f;
	}
	return sa.Return(CGameImplementation::getInst().fGameSpeed);
}

int game_GetGameSpeed(HSQUIRRELVM v)
{
	StackHandler sa(v);
	return sa.Return(CGameImplementation::getInst().fGameSpeed);
}

int nloc_SetAutodrop(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CString sMask=sa.GetString(SQ_PARAM1);
	CString sTo=sa.GetString(SQ_PARAM2);
	if(sTo.GetLength()){
		for(UINT i=0;i<CLocationManager::getInstance().autoMoverRules.size();i++)
		{
			if(sMask==CLocationManager::getInstance().autoMoverRules[i].szNameFrom){
				// Новое значение
				strcpy(CLocationManager::getInstance().autoMoverRules[i].szNameTo,sTo);
				return sa.Return(true);
			}
		}
		CLocationConnector conn;
		strcpy(conn.szNameFrom,sMask);
		strcpy(conn.szNameTo,sTo);
		CLocationManager::getInstance().autoMoverRules.push_back(conn);
		return sa.Return(true);
	}else{
		for(UINT i=0;i<CLocationManager::getInstance().autoMoverRules.size();i++)
		{
			if(sMask==CLocationManager::getInstance().autoMoverRules[i].szNameFrom){
				CLocationManager::getInstance().autoMoverRules.erase(i);
				return sa.Return(true);
			}
		}
	}
	return sa.Return(false);
}

int nloc_GetLocations(HSQUIRRELVM v)
{
	StackHandler sa(v);
	SquirrelObject result=SquirrelVM::CreateArray(0);
	if(sa.GetType(SQ_PARAM1)==OT_TABLE){// по имени
		SquirrelObject sqo=sa.GetObjectHandle(SQ_PARAM1);
		CString sByNameMask=sqo.GetString("_byName");
		BOOL bByPosition=FALSE;
		_v2 checkPosition;
		if(sqo.Exists("_byPosition")){
			bByPosition=TRUE;
			SquirrelObject sqPos=sqo.GetValue("_byPosition");
			checkPosition.X=sqPos.GetFloat("_x");
			checkPosition.Y=sqPos.GetFloat("_y");
		}
		int iResulActorI=0;
		_array_CLocationsP& locs=CLocationManager::getInstance(CurrentLocManager()).locations;
		for(u32 i=0;i<locs.size();i++)
		{
			CLocation* actorLocationLocator=locs[i];
			if(bByPosition){
				if(actorLocationLocator && !actorLocationLocator->IsInLocation(checkPosition)){
					continue;
				}
			}
			if(sByNameMask.GetLength()>0){
				if(!patternMatch(actorLocationLocator->szName,sByNameMask)){
					continue;
				}
			}
			result.ArrayAppend(actorLocationLocator->szName.GetBuffer(0));
			iResulActorI++;
		}
	}
	return sa.Return(result);
}

int nloc_GetLocation(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CLocation* loc=getLocation(sa,SQ_PARAM1);
	if(!loc){
		return sa.Return(false);
	}
	SquirrelObject sqo=SquirrelVM::CreateTable();
	sqo.SetValue("_name",loc->szName.GetBuffer(0));
	sqo.SetValue("_x",loc->x);
	sqo.SetValue("_y",loc->y);
	sqo.SetValue("_w",loc->w);
	sqo.SetValue("_h",loc->h);
	return sa.Return(sqo);
}

int nloc_GetWalkableState(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CLocation* loc=getLocation(sa,SQ_PARAM1);
	if(!loc){
		return sa.Return(false);
	}
	bool bRes=true;
	_v2 posInQuestion(loc->x,loc->y);
	int iRes=getLevel()->levelNavigator->EnsureWalkablePos(posInQuestion,TRUE);
	//if(iRes!=WALKSTATE_IN){bRes=false;}
	return sa.Return(iRes);
}

int nloc_CheckWalkablePath(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CLocation* loc1=getLocation(sa,SQ_PARAM1);
	if(!loc1){
		return sa.Return(false);
	}
	CLocation* loc2=getLocation(sa,SQ_PARAM2);
	if(!loc2){
		return sa.Return(false);
	}
	bool bRes=true;
	_v2 posInQuestion(loc1->x,loc1->y);
	_v2 posInQuestion2(loc2->x,loc2->y);
	CWalkData emptyData;
	int iRes=getLevel()->levelNavigator->GetWalkablePath(posInQuestion,posInQuestion2,emptyData,TRUE);
	TRACE(toString(">>> %s->%s: путь %s\n",loc1->szName,loc2->szName,iRes?"есть":"нет"));
	return sa.Return(iRes?true:false);
}

int game_GetState(HSQUIRRELVM v)
{
	StackHandler sa(v);
	if(!getLevel()){
		return sa.Return((int)LEVELSTATE_INIT);
	}
	return sa.Return((int)getLevel()->dwLevelState);
}

int game_Pause(HSQUIRRELVM v)
{
	StackHandler sa(v);
	BOOL bPause=sa.GetBool(SQ_PARAM1);
	if(bPause){
		if(getLevel() && getLevel()->dwLevelState>=LEVELSTATE_INGAME_ACTIVE && getLevel()->dwLevelState<LEVELSTATE_PAUSE)
		{
			getLevel()->dwLevelState=LEVELSTATE_PAUSE;
			getLevel()->dwLevelStateSetTime=getTick();
			return sa.Return(true);
		}
	}else{
		if(getLevel() && getLevel()->dwLevelState==LEVELSTATE_PAUSE)
		{
			getLevel()->dwLevelState=LEVELSTATE_INGAME_ACTIVE;
			getLevel()->dwLevelStateSetTime=getTick();
			return sa.Return(true);
		}
	}
	return sa.Return(false);
}

int event_GetEvent(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CAction* action=0;
	if(sa.GetType(SQ_PARAM1)==OT_STRING){
		CString sEventName=sa.GetString(SQ_PARAM1);
		action=getLevel()->getActionByName(sEventName);
	}
	if(sa.GetType(SQ_PARAM1)==OT_INTEGER){
		int iEventName=sa.GetInt(SQ_PARAM1);
		if(iEventName<int(getLevel()->actions.size())){
			action=getLevel()->actions[iEventName];
		}
	}
	if(!action){
		return sa.Return(false);
	}
	action->UpdateScriptMirror();
	return sa.Return(action->actionScriptMirror);
}

int event_GetEvents(HSQUIRRELVM v)
{
	StackHandler sa(v);
	_array_i resI;
	SquirrelObject result=SquirrelVM::CreateArray(0);
	if(sa.GetType(SQ_PARAM1)==OT_TABLE){// по имени
		SquirrelObject sqo=sa.GetObjectHandle(SQ_PARAM1);
		CString sByNameMask=sqo.GetString("_byName");
		// Пробегаемся по актерам
		int iResulEventsI=0;
		for(u32 i=0;i<getLevel()->actions.size();i++)
		{
			CAction* action=getLevel()->actions[i];
			if(sByNameMask.GetLength()>0){
				if(!patternMatch(action->p_sActionName,sByNameMask)){
					continue;
				}
			}
			resI.push_back(i);
			iResulEventsI++;
		}
		BOOL bRandomize=sqo.GetBool("_randomize");
		if(bRandomize){
			for(u32 i=0;i<resI.size();i++)
			{
				int i2=rnd(0,resI.size());

				int tmp=resI[i];
				resI[i]=resI[i2];
				resI[i2]=tmp;
			}
		}
		for(u32 i=0;i<resI.size();i++)
		{
			CAction* action=getLevel()->actions[resI[i]];
			action->UpdateScriptMirror();
			result.ArrayAppend(action->actionScriptMirror);
		}
	}
	return sa.Return(result);
}

int level_GetTimeToNewIssue(HSQUIRRELVM v)
{
	StackHandler sa(v);
	return sa.Return(getLevel()->data.lProgress_TimeMax-getLevel()->data.lProgress_TimeNow);
}

int level_Time(HSQUIRRELVM v)
{
	StackHandler sa(v);
	return sa.Return(getLevel()->fProgressTime);
}

int level_TimeAbs(HSQUIRRELVM v)
{
	StackHandler sa(v);
	return sa.Return(getIngameTimeAbs());
}

int level_Progress(HSQUIRRELVM v)
{
	StackHandler sa(v);
	int iCur=-1;
	int iChanges=0;
	if(sa.GetType(SQ_PARAM1)!=OT_NULL){
		iCur=sa.GetInt(SQ_PARAM1);
		getLevel()->data.lProgress_Cur=iCur;
		iChanges++;
	}
	int iMax=-1;
	if(sa.GetType(SQ_PARAM2)!=OT_NULL){
		iMax=sa.GetInt(SQ_PARAM2);
		getLevel()->data.lProgress_Max=iMax;
		iChanges++;
	}
	if(iChanges){
		getLevel()->UpdateProgress();
	}
	return sa.Return(getLevel()->data.lProgress_Cur);
}

int level_CheckLocks(HSQUIRRELVM v)
{
	StackHandler sa(v);
	getLevel()->CheckLokReashability();
	return SQ_OK;
}

int level_Crunch(HSQUIRRELVM v)
{
	StackHandler sa(v);
	if(sa.GetType(SQ_PARAM1)==OT_BOOL){
		return sa.Return(getLevel()->isCrunch);
	}
	if(sa.GetType(SQ_PARAM1)!=OT_NULL){
		f32 fCrunch=sa.GetFloat(SQ_PARAM1);
		getLevel()->data.fProgress_Crunch=fCrunch;
		getLevel()->UpdateProgress();
	}
	return sa.Return(getLevel()->data.fProgress_Crunch);
}

int level_ResetTime(HSQUIRRELVM v)
{
	getLevel()->ResetOfficeObject();
	return SQ_OK;
}

int game_CutsceneBegin(HSQUIRRELVM v)
{
	StackHandler sa(v);
	int iCutSceneType=1;
	if(sa.GetType(SQ_PARAM1)!=-1){
		iCutSceneType=sa.GetInt(SQ_PARAM1);
	}
	getLevel()->EnableCutscene(iCutSceneType);
	return SQ_OK;
}

int game_CutsceneEnd(HSQUIRRELVM v)
{
	getLevel()->EnableCutscene(0);
	return SQ_OK;
}

int game_IsCutscene(HSQUIRRELVM v)
{
	StackHandler sa(v);
	return sa.Return(getLevel()->bIsCutscene?true:false);
}

int game_SaveGame(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CString sText=sa.GetString(SQ_PARAM1);
	return sa.Return(getLevel()->SaveGame(sText)?true:false);
}

int gui_ClearHint(HSQUIRRELVM v)
{
	StackHandler sa(v);
	getLevel()->HideHint(FALSE,-1);
	return SQ_OK;
}


int gui_ShowHint(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CString sText=sa.GetString(SQ_PARAM1);
	f32 fTime=sa.GetFloat(SQ_PARAM2);
	int iPriority=1;
	if(sa.GetType(SQ_PARAM3)!=OT_NULL){
		iPriority=sa.GetInt(SQ_PARAM3);
	}
	getLevel()->AddHint(sText,fTime,iPriority,0xFFFFFFFF,0);
	return SQ_OK;
}

int gui_ShowAlarm(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CString sText=sa.GetString(SQ_PARAM1);
	f32 fTime=sa.GetFloat(SQ_PARAM2);
	getLevel()->AddHint(sText,fTime,2,0xFFFF0000,1);
	return SQ_OK;
}

int game_SetMagazinName(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CString sText=sa.GetString(SQ_PARAM1);
	if(sText.GetLength()>0){
		getLevel()->data.sMagazinName=sText;
	}
	return SQ_OK;
}

int game_GetMagazinName(HSQUIRRELVM v)
{
	StackHandler sa(v);
	return sa.Return(getLevel()->data.sMagazinName.GetBuffer(0));
}

int game_GetSafe(HSQUIRRELVM v)
{
	StackHandler sa(v);
	return sa.Return(getLevel()->data.scriptSafe);
}

int game_MarkSafe(HSQUIRRELVM v)
{
	StackHandler sa(v);
	//getGame().getProfile().Changed();
	return sa.Return(getLevel()->data.scriptSafe);
}

int game_VisualizeLocations(HSQUIRRELVM v)
{
	StackHandler sa(v);
	_v2 checkPosition;
	checkPosition.X=sa.GetFloat(SQ_PARAM1);
	checkPosition.Y=sa.GetFloat(SQ_PARAM2);
	int iResulActorI=0;
	_array_CLocationsP& locs=CLocationManager::getInstance(CurrentLocManager()).locations;
	for(u32 i=0;i<locs.size();i++)
	{
		CLocation* actorLocationLocator=locs[i];
		if(actorLocationLocator && !actorLocationLocator->IsInLocation(checkPosition)){
			continue;
		}
		CSpriteNode* nd=VisualizeRCTF(actorLocationLocator->Convert2rect(),getDataPath("\\gui\\editor\\square.png"));
		addText("<font-size:-2>\n"+actorLocationLocator->szName,_v3_NULL,nd);
		Warning(actorLocationLocator->szName);
	}
	return SQ_OK;
}

int game_VisualizeNavimap(HSQUIRRELVM v)
{
	StackHandler sa(v);
	_v2 checkPosition;
	checkPosition.X=sa.GetFloat(SQ_PARAM1);
	checkPosition.Y=sa.GetFloat(SQ_PARAM2);
	int iResulActorI=0;
	int iType=sa.GetInt(SQ_PARAM3);
	if(iType==0)
	{
		getLevel()->levelNavigator->VisualizeWall(checkPosition);
	}
	if(iType==2){

		static CString sReperPoint=getManifest().Settings.getString("NavigationReperPoint");
		CLocation* addw=CLocationManager::getInstance().getLocationByNameSoft(sReperPoint);
		_v2 startPosition=addw->getPosition();
		_v2 targetPosition;
		targetPosition=CLocationManager::getInstance().getLocationByNameSoft(sReperPoint+"_END1")->getPosition();
		getLevel()->levelNavigator->EnsureWalkablePos(targetPosition,999,FALSE);
		getLevel()->levelNavigator->EnsureWalkablePos(startPosition,FALSE,FALSE);
		getLevel()->levelNavigator->VisualizePath(startPosition,targetPosition);
		targetPosition=CLocationManager::getInstance().getLocationByNameSoft(sReperPoint+"_END2")->getPosition();
		getLevel()->levelNavigator->EnsureWalkablePos(targetPosition,999,FALSE);
		getLevel()->levelNavigator->EnsureWalkablePos(startPosition,FALSE,FALSE);
		getLevel()->levelNavigator->VisualizePath(startPosition,targetPosition);
	}
	if(iType==1){
		getLevel()->levelNavigator->VisualizeFloor(checkPosition);
	}
	return SQ_OK;
}

void RegisterActorScriptMethods();
void RegisterGameScriptMethods()
{
	SquirrelVM::CreateFunctionGlobal(game_GetGameSpeed,_T("game_GetGameSpeed"));
	SquirrelVM::CreateFunctionGlobal(game_SetGameSpeed,_T("game_SetGameSpeed"),"*");
	SquirrelVM::CreateFunctionGlobal(game_GetState,_T("game_GetState"));

	SquirrelVM::CreateFunctionGlobal(nloc_Remove,_T("nloc_Remove"),"*");
	SquirrelVM::CreateFunctionGlobal(nloc_Add,_T("nloc_Add"),"*");
	SquirrelVM::CreateFunctionGlobal(nloc_Save,_T("nloc_Save"),"*");
	SquirrelVM::CreateFunctionGlobal(nloc_Load,_T("nloc_Load"),"*");
	SquirrelVM::CreateFunctionGlobal(nloc_GetLocation,_T("nloc_GetLocation"),"*");
	SquirrelVM::CreateFunctionGlobal(nloc_SetAutodrop,_T("nloc_SetAutodrop"),"*");
	SquirrelVM::CreateFunctionGlobal(nloc_GetLocations,_T("nloc_GetLocations"),"*");
	SquirrelVM::CreateFunctionGlobal(nloc_GetWalkableState,_T("nloc_GetWalkableState"),"*");
	SquirrelVM::CreateFunctionGlobal(nloc_CheckWalkablePath,_T("nloc_CheckWalkablePath"),"*");
	
	SquirrelVM::CreateFunctionGlobal(game_Pause,_T("game_Pause"),"*");
	SquirrelVM::CreateFunctionGlobal(level_Progress,_T("level_Progress"),"*");
	SquirrelVM::CreateFunctionGlobal(level_Time,_T("level_Time"),"*");
	SquirrelVM::CreateFunctionGlobal(level_TimeAbs,_T("level_TimeAbs"),"*");
	SquirrelVM::CreateFunctionGlobal(level_GetTimeToNewIssue,_T("level_GetTimeToNewIssue"),"*");
	
	
	SquirrelVM::CreateFunctionGlobal(level_Crunch,_T("level_Crunch"),"*");
	SquirrelVM::CreateFunctionGlobal(level_CheckLocks,_T("level_CheckLocks"),"*");
	SquirrelVM::CreateFunctionGlobal(level_ResetTime,_T("level_ResetTime"),"*");
	

	SquirrelVM::CreateFunctionGlobal(event_GetEvent,_T("event_GetEvent"),"*");
	SquirrelVM::CreateFunctionGlobal(event_GetEvents,_T("event_GetEvents"),"*");
	SquirrelVM::CreateFunctionGlobal(game_VisualizeLocations,_T("game_VisualizeLocations"),"*");
	SquirrelVM::CreateFunctionGlobal(game_VisualizeNavimap,_T("game_VisualizeNavimap"),"*");

	SquirrelVM::CreateFunctionGlobal(game_CutsceneBegin,_T("game_CutsceneBegin_i"),"*");
	SquirrelVM::CreateFunctionGlobal(game_CutsceneEnd,_T("game_CutsceneEnd_i"),"*");
	SquirrelVM::CreateFunctionGlobal(game_IsCutscene,_T("game_IsCutscene"),"*");
	SquirrelVM::CreateFunctionGlobal(game_SaveGame,_T("game_SaveGame"),"*");
	SquirrelVM::CreateFunctionGlobal(gui_ShowHint,_T("gui_ShowHint"),"*");
	SquirrelVM::CreateFunctionGlobal(gui_ShowAlarm,_T("gui_ShowAlarm"),"*");
	SquirrelVM::CreateFunctionGlobal(gui_ClearHint,_T("gui_ClearHint"),"*");
	SquirrelVM::CreateFunctionGlobal(game_SetMagazinName,_T("game_SetMagazinName"),"*");
	SquirrelVM::CreateFunctionGlobal(game_GetMagazinName,_T("game_GetMagazinName"),"*");
	SquirrelVM::CreateFunctionGlobal(game_GetSafe,_T("game_GetSafe"),"*");
	SquirrelVM::CreateFunctionGlobal(game_MarkSafe,_T("game_MarkSafe"),"*");

	RegisterActorScriptMethods();
}