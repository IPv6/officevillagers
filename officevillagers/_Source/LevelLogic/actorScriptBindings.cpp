#include "StdAfx.h"
#include "actorScriptBindings.h"
#include "../scriptBindings.h"
#include "actor.h"
#include "action.h"
#include "level.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

#ifdef _DEBUG
#define DEBUG_ASSERT_SCRIPT(X,Y){if(!(X)){static CString s=(toString("Script Assert!\n%s: %i\n\n%s\n\nScriptStep: %s",__FILE__,__LINE__,(Y),getGame().scripter.sLastScriptDebug));Alert(s);};};
#define CHECK_GET_ACTOR(actor) \
	if(!actor){ \
		if(IS_SHIFT_PRESSED()){	SquirrelObject res=SquirrelVM::CreateTable();res.SetValue("Ololo","1");	return sa.Return(res);} \
		return sa.Return(false);\
	}

#else
#define DEBUG_ASSERT_SCRIPT(X,Y)
#define CHECK_GET_ACTOR(actor) \
	if(!actor){ \
		return sa.Return(false);\
	}
#endif

CNodeBasement* getNodeFromParam(StackHandler& sa,int sqParam);
void CheckScriptDeprecations(const char* szContent)
{
#ifdef _DEBUG
	return;
#endif
	if(strstr(szContent,DEPRECATION1)!=0){
		ThrowCriticalError(DEPRECATION1 " used! This feature is not for release build!");
	}
	if(strstr(szContent,DEPRECATION2)!=0){
		ThrowCriticalError(DEPRECATION2 " used! This feature is not for release build!");
	}
	return;
}

CActor* getActor(StackHandler& sa, int iParaNum,BOOL bSilently=0)
{
	MEASURE_THIS;
	DEBUG_ASSERT_SCRIPT(getLevel()!=0,"WTF???");
	BOOL bByName=0;
	CString sByName;
	int iType=sa.GetType(iParaNum);
	if(iType==OT_NULL || iType==-1){
		return getLevel()->data.pMainOfficeActor;
	}
	if(iType==OT_STRING){// по имени
		sByName=sa.GetString(iParaNum);
		bByName=1;
	}
	if(iType==OT_TABLE){// по имени
		SquirrelObject sqo=sa.GetObjectHandle(iParaNum);
		sByName=sqo.GetString("Name");
		//ACTOR_AVATAR_ID
		bByName=1;
	}
	if(bByName){
		if(sByName==OFFICE_ACTOR){
			return getLevel()->data.pMainOfficeActor;
		}
		CActor* actor=getLevel()->getActorByName(sByName);
		if(!bSilently){
			DEBUG_ASSERT_SCRIPT(actor!=0,toString("Actor '%s' does not exist",sByName));
			DEBUG_ASSERT_SCRIPT(!actor || actor->data.p_sName.Find(DELETEDACTOR_MARK)==-1,toString("Actor '%s' already deleted!",actor->data.p_sName));
		}
		return actor;
	}

	int iID=-1;
	if(iType==OT_INTEGER){// по ID ноды
		iID=sa.GetInt(iParaNum);
	}
	CArray< CActor*,CActor* >& actors=getLevel()->data.actors;
	for(int i=0;i<actors.GetSize();i++)
	{
		CActor* actor=actors[i];
		if(actor && actor->iActorID==iID)
		{
			DEBUG_ASSERT_SCRIPT(actor->data.p_sName.Find(DELETEDACTOR_MARK)==-1,toString("Actor '%s' already deleted!",actor->data.p_sName));
			return actor;
		}
	}
	//При наведении на сидящего идет проверка по коду
	// Вполне может и не найти актера по ID!
	//DEBUG_ASSERT_SCRIPT(0,"Actor can not be found!");
	return 0;
}


int actor_SwitchToAction(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CActor* actor=getActor(sa,SQ_PARAM1);
	CHECK_GET_ACTOR(actor);
	CString sAction=sa.GetString(SQ_PARAM2);
	if(!sAction.GetLength()){
		sAction=NO_ACTION;
	}
	BOOL bNowSet=FALSE;
	if(sa.GetType(SQ_PARAM3)!=-1){
		BOOL bNow=sa.GetBool(SQ_PARAM3);
		if(bNow){
			actor->AI_SwitchToAction(sAction,TRUE,TRUE);
			bNowSet=TRUE;
		}
	}
	if(!bNowSet){
		actor->AI_SwitchToAction(sAction,TRUE,FALSE);
	}
	return sa.Return(true);
}

// Посылает в локацтю... динамические координаты локации!
int actor_SendToLocation(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CActor* actor=getActor(sa,SQ_PARAM1);
	CHECK_GET_ACTOR(actor);
	actor->StopBodyMotionUI();
	if(sa.GetType(SQ_PARAM2)==OT_FLOAT){
		f32 fx=sa.GetFloat(SQ_PARAM2);
		f32 fy=sa.GetFloat(SQ_PARAM3);
		actor->activeNavigationDotLocationName="";
		if(!actor->AI_SetMove2TargetPos(_v2(fx,fy))){
			return sa.Return(false);
		}
	}else{
		CLocation* loc=getLocation(sa,SQ_PARAM2);
		if(!loc){
			return sa.Return(false);
		}
		if(!actor->AI_SetMove2TargetPos(loc->szName)){
			return sa.Return(false);
		}
	}
	return sa.Return(true);
}

int actor_IsInLocation(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CActor* actor=getActor(sa,SQ_PARAM1);
	CHECK_GET_ACTOR(actor);
	CLocation* location=getLocation(sa,SQ_PARAM2);
	if(!location){
		return sa.Return(false);
	}
	if(location->IsInLocation(actor)){
		return sa.Return(true);
	}
	return sa.Return(false);
}

int actor_IsMovingToLocation(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CActor* actor=getActor(sa,SQ_PARAM1);
	CHECK_GET_ACTOR(actor);
	CLocation* loc=getLocation(sa,SQ_PARAM2);
	if(!loc){
		return sa.Return(false);
	}
	if(actor->activeNavigationDotLocationName==loc->szName){
		return sa.Return(true);
	}
	return sa.Return(false);
}


int getTime(HSQUIRRELVM v)
{
	StackHandler sa(v);
	// ИГРОВОЕ время!!!
	// Оно кладется в атрибуты и т.п.
	f32 res=getIngameTimeAbs();
	return sa.Return(res);
}


int game_GetTickCount(HSQUIRRELVM v)
{
	// Время с начала работы уровня (текущая сессия)
	StackHandler sa(v);
	return sa.Return(int(CLevelThinker::getThinker()->getGameTimer()));
}

int actor_ShowLabels(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CActor* actor=getActor(sa,SQ_PARAM1);
	CHECK_GET_ACTOR(actor);
	int iEnable=0;
	if(sa.GetType(SQ_PARAM2)==OT_BOOL){
		iEnable=sa.GetBool(SQ_PARAM2);
	}else{
		iEnable=sa.GetInt(SQ_PARAM2);
	}
	CNodeBasement* tn=getNodeFromParam(sa,SQ_PARAM3);
	actor->ShowActorLabels(iEnable,tn);
	return sa.Return(true);
}

int actor_PauseMovements(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CActor* actor=getActor(sa,SQ_PARAM1);
	CHECK_GET_ACTOR(actor);
	int iEnable=sa.GetBool(SQ_PARAM2);
	int iCurPaus=actor->AI_PauseMovements(iEnable);
	return sa.Return(iCurPaus);
}

int actor_PauseInteractivity(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CActor* actor=getActor(sa,SQ_PARAM1);
	CHECK_GET_ACTOR(actor);
	int iEnablePause=sa.GetBool(SQ_PARAM2);
	actor->SetInteractive(iEnablePause?FALSE:TRUE);
	return sa.Return(true);
}

int actor_IsInteractive(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CActor* actor=getActor(sa,SQ_PARAM1);
	CHECK_GET_ACTOR(actor);
	return sa.Return(actor->GetInteractive()?true:false);
}

int actor_GetProfessionStats(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CProphession* prof=0;
	if(sa.GetType(SQ_PARAM1)==OT_STRING){
		CString sProf=sa.GetString(SQ_PARAM1);
		prof=getLevel()->getProphessionByName(sProf);
	}
	if(sa.GetType(SQ_PARAM1)==OT_INTEGER){
		int iProf=sa.GetInt(SQ_PARAM1);
		if(iProf<int(getLevel()->dataLevelInfo.prophessions.size())){
			prof=getLevel()->dataLevelInfo.prophessions[iProf];
		}
	}
	if(prof){
		SquirrelObject res=SquirrelVM::CreateTable();
		res.SetValue("Name",(prof->sName).GetBuffer(0));
		res.SetValue("BaseName",(prof->sBaseName).GetBuffer(0));
		res.SetValue("NoThink",(prof->lNoThink));
		res.SetValue("ActorsCount",(prof->lCounter));
		return sa.Return(res);
	}else{
		return sa.Return(false);
	}
}

int actor_SetProf(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CActor* actor=getActor(sa,SQ_PARAM1);
	CHECK_GET_ACTOR(actor);
	CString sProf=sa.GetString(SQ_PARAM2);
	BOOL bPseudo=sa.GetBool(SQ_PARAM3);
	actor->SetProf(sProf,bPseudo?1:0);
	return sa.Return(true);
}

int actor_RandomizePos(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CActor* actor=getActor(sa,SQ_PARAM1);
	CHECK_GET_ACTOR(actor);
	CLocation* location=getLocation(sa,SQ_PARAM2);
	if(!location){
		return sa.Return(false);
	}
	_rctf rr=location->Convert2rect();
	f32 x = frand(rr.DOWN_LEFT.X,rr.UP_RIGHT.X);
	f32 y = frand(rr.DOWN_LEFT.Y,rr.UP_RIGHT.Y);
	actor->SetLocation(_v2(x,y));
	return sa.Return(true);
}

int actor_SetActorPos(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CActor* actor=getActor(sa,SQ_PARAM1);
	CHECK_GET_ACTOR(actor);
	if(sa.GetType(SQ_PARAM2)==OT_TABLE){
		SquirrelObject sqo=sa.GetObjectHandle(SQ_PARAM2);
		if(sqo.GetBool("_raw")){
			_v2 target;
			target.X=sqo.GetFloat("_x");
			target.Y=sqo.GetFloat("_y");
			actor->SetLocation(target);
			return sa.Return(true);
		}
	}

	CLocation* location=getLocation(sa,SQ_PARAM2);
	if(!location){
		return sa.Return(false);
	}
	actor->SetLocation(location);
	return sa.Return(true);
}

int actor_GetActorPos(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CActor* actor=getActor(sa,SQ_PARAM1);
	CHECK_GET_ACTOR(actor);
	actor->scriptMirror.SetValue("_x",actor->data.p_pos.X);
	actor->scriptMirror.SetValue("_y",actor->data.p_pos.Y);
	return sa.Return(actor->scriptMirror);
}


int actor_IsActorExist(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CActor* actor=getActor(sa,SQ_PARAM1,true);
	CHECK_GET_ACTOR(actor);
	return sa.Return(actor->scriptMirror);
}

int actor_GetActor(HSQUIRRELVM v)
{
	MEASURE_THIS;
	StackHandler sa(v);
	BOOL bSilent=sa.GetBool(SQ_PARAM2);
	CActor* actor=getActor(sa,SQ_PARAM1,bSilent);
	CHECK_GET_ACTOR(actor);
	return sa.Return(actor->scriptMirror);
}

int actor_GetActors(HSQUIRRELVM v)
{
	MEASURE_THIS;
	StackHandler sa(v);
	SquirrelObject resultActors=SquirrelVM::CreateArray(0);
	if(sa.GetType(SQ_PARAM1)==OT_TABLE){// по имени
		_array_i resI;
		SquirrelObject sqo=sa.GetObjectHandle(SQ_PARAM1);
		CString sByNameMask=sqo.GetString("_byName");
		BOOL bByType=0;
		int iType=-1;
		if(sqo.Exists("_byType")){
			bByType=TRUE;
			iType=sqo.GetInt("_byType");
		}
		BOOL bByPosition=FALSE;
		_v2 checkPosition;
		_rctf checkPositionZazor;
		if(sqo.Exists("_byLocation")){
			bByPosition=TRUE;
			CString sLoc=sqo.GetString("_byLocation");
			CLocation* loc=getLocationPrefixed(sLoc);
			checkPosition.X=loc->x;
			checkPosition.Y=loc->y;
			checkPositionZazor.UpperLeftCorner.Y=loc->h*0.5f;
			checkPositionZazor.UpperLeftCorner.X=-loc->w*0.5f;
			checkPositionZazor.LowerRightCorner.Y=-loc->h*0.5f;
			checkPositionZazor.LowerRightCorner.X=loc->w*0.5f;
			// НЕ Проверяем доступна ли локация... возможно нам надо как раз предметы внутри мебели
		}
		BOOL bClosest=sqo.GetBool("_closest");
		if(sqo.Exists("_byPosition")){
			bByPosition=TRUE;
			SquirrelObject sqPos=sqo.GetValue("_byPosition");
			checkPosition.X=sqPos.GetFloat("_x");
			checkPosition.Y=sqPos.GetFloat("_y");
			if(sqo.Exists("_byPositionLimits")){
				SquirrelObject sqLim=sqo.GetValue("_byPositionLimits");
				checkPositionZazor.UpperLeftCorner.Y=sqLim.GetFloat("_u");
				checkPositionZazor.UpperLeftCorner.X=-sqLim.GetFloat("_l");
				checkPositionZazor.LowerRightCorner.Y=-sqLim.GetFloat("_d");
				checkPositionZazor.LowerRightCorner.X=sqLim.GetFloat("_r");
			}
		}
		BOOL bIncludeDisabled=sqo.GetBool("_includeDisabled");
		CString sProfMask=sqo.GetString("_byProfession");
		CString sActionMask=sqo.GetString("_byAction");
		// Пробегаемся по актерам
		int iResulActorI=0;
		for(int i=0;i<getLevel()->data.actors.GetSize();i++)
		{
			CActor* actor=getLevel()->data.actors[i];
			if(!bIncludeDisabled){
				if(!actor->data.p_Enabled){
					continue;
				}
			}
			if(bByType){
				if(actor->data.p_iActorType!=iType){
					continue;
				}
			}
			if(bByPosition){
				if(actor->actorLocationLocator && !actor->actorLocationLocator->IsInLocation(checkPosition,&checkPositionZazor)){
					continue;
				}
			}
			if(sActionMask.GetLength()>0){
				if(!actor->p_CurrentAction || !patternMatch(actor->data.p_CurrentActionName,sActionMask)){
					continue;
				}
			}
			if(sProfMask.GetLength()>0){
				if(!actor->data.p_ProfessionName.GetLength() || !patternMatch(actor->data.p_ProfessionName,sProfMask)){
					continue;
				}
			}
			if(sByNameMask.GetLength()>0){
				if(!patternMatch(actor->data.p_sName,sByNameMask)){
					continue;
				}
			}
			resI.push_back(i);
			//result.ArrayAppend(actor->scriptMirror);
			iResulActorI++;
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
		if(bClosest)
		{
			for(u32 i=1;i<resI.size();i++)
			{
				for(u32 i2=0;i2<i;i2++)
				{
					CActor* actor1=getLevel()->data.actors[resI[i]];
					CActor* actor2=getLevel()->data.actors[resI[i2]];
					f32 dist1=checkPosition.getDistanceFromSQ(actor1->data.p_pos);
					f32 dist2=checkPosition.getDistanceFromSQ(actor2->data.p_pos);
					if(dist2>dist1){
						int tmp=resI[i];
						resI[i]=resI[i2];
						resI[i2]=tmp;
					}
				}
			}
		}
		for(u32 i=0;i<resI.size();i++)
		{
			CActor* actor=getLevel()->data.actors[resI[i]];
			resultActors.ArrayAppend(actor->scriptMirror);
		}
	}
	return sa.Return(resultActors);
}

int actor_CloneActor(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CActor* actor=getActor(sa,SQ_PARAM1);
	CHECK_GET_ACTOR(actor);
	CActor* actor2=getLevel()->CreateActor(actor);
	actor2->SetLocation(actor->getPosition());
	return sa.Return(actor2->scriptMirror);
}

int actor_CreateActor(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CString sTemplateName=sa.GetString(SQ_PARAM1);
	CString sNameOverload=sa.GetString(SQ_PARAM2);
	CString sOptions,sLoc;
	int iType=sa.GetType(SQ_PARAM3);
	if(iType==OT_STRING){
		sOptions=sa.GetString(SQ_PARAM3);
	}else if(iType==OT_TABLE){
		SquirrelObject sqo=sa.GetObjectHandle(SQ_PARAM3);
		sOptions=sqo.GetString("_options");
		sLoc=sqo.GetString("_location");
	}
	CActor* actor=getLevel()->CreateActor(sTemplateName,sNameOverload,sOptions,sLoc);
	return sa.Return(actor->scriptMirror);
}

int actor_DeleteActor(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CActor* actor=getActor(sa,SQ_PARAM1);
	CHECK_GET_ACTOR(actor);
	BOOL bRes=getLevel()->DeleteActor(actor);
	return sa.Return(bRes?true:false);
}

int actor_OrientBody(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CActor* actor=getActor(sa,SQ_PARAM1);
	CHECK_GET_ACTOR(actor);
	if(sa.GetType(SQ_PARAM2)==OT_FLOAT && sa.GetType(SQ_PARAM3)==OT_FLOAT){
		_v2 dir;
		dir.X=sa.GetFloat(SQ_PARAM2);
		dir.Y=sa.GetFloat(SQ_PARAM3);
		actor->RefreshBodyAccordingDirection(_v2(dir.X,dir.Y),FALSE);
		return sa.Return(true);
	}

	int iType=sa.GetType(SQ_PARAM2);
	if(iType!=OT_TABLE && iType!=OT_STRING){
		f32 angl=sa.GetFloat(SQ_PARAM2);
		_v3 dir(1.0f,0.0f,0.0f);
		rotateXYVectorSlow(dir,f32(GRAD_PI2*angl));
		actor->RefreshBodyAccordingDirection(_v2(dir.X,dir.Y),FALSE);
		return sa.Return(true);
	}
	CLocation* loc=getLocation(sa,SQ_PARAM2);
	if(!loc){
		return sa.Return(false);
	}
	_v2 dir=loc->getPosition()-actor->getPosition();
	actor->RefreshBodyAccordingDirection(_v2(dir.X,dir.Y),FALSE);
	return sa.Return(true);
}

int actor_StopWalking(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CActor* actor=getActor(sa,SQ_PARAM1);
	CHECK_GET_ACTOR(actor);
	f32 iSecs=sa.GetFloat(SQ_PARAM2);
	actor->PauseMovementsForSec(int(iSecs));
	return sa.Return(true);
}

int actor_CancelWalking(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CActor* actor=getActor(sa,SQ_PARAM1);
	CHECK_GET_ACTOR(actor);
	actor->activeNavigationDotLocationName="";
	actor->PauseMovementsForSec(0);
	return sa.Return(true);
}

int actor_EnsureDistance(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CActor* actor=getActor(sa,SQ_PARAM1);
	CHECK_GET_ACTOR(actor);
	CActor* actor2=getActor(sa,SQ_PARAM2);
	CHECK_GET_ACTOR(actor2);
	f32 fSpace=sa.GetFloat(SQ_PARAM3);
	bool forceDistAsNeared=sa.GetBool(SQ_PARAM4)?true:false;
	actor->AI_EnsureMinDist2Actor(actor2,fSpace,forceDistAsNeared);
	return sa.Return(true);
}

int actor_UpdatePosition(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CActor* actor=getActor(sa,SQ_PARAM1);
	CHECK_GET_ACTOR(actor);
	actor->AI_SyncPosition(0);
	return sa.Return(true);
}

int actor_OrientHead(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CActor* actor=getActor(sa,SQ_PARAM1);
	CHECK_GET_ACTOR(actor);
	int iType=sa.GetType(SQ_PARAM2);
	if(iType!=OT_TABLE && iType!=OT_STRING){
		f32 angl=sa.GetFloat(SQ_PARAM2);
		actor->RefreshHeadAccordingDirection(angl,FALSE,TRUE);
		return sa.Return(true);
	}
	CLocation* loc=getLocation(sa,SQ_PARAM2);
	if(!loc){
		return sa.Return(false);
	}
	_v2 dir=loc->getPosition()-actor->getPosition();
	f32 headRotation=getXYRotationFromDirection(_v3(dir.X,dir.Y,0.0f));
	actor->RefreshHeadAccordingDirection(headRotation/PI*180.0f,0,FALSE);
	return sa.Return(true);
}

int actor_AddAttribute(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CString sName=sa.GetString(SQ_PARAM2);
	int iType=sa.GetType(SQ_PARAM3);
	CActor* actor=getActor(sa,SQ_PARAM1);
	CHECK_GET_ACTOR(actor);
	if(iType==OT_STRING)
	{
		CString sVal=sa.GetString(SQ_PARAM3);
		actor->AddAttribute(sName,sVal);
		return sa.Return(true);
	}
	if(iType==OT_INTEGER)
	{
		int sVal=sa.GetInt(SQ_PARAM3);
		actor->AddAttribute(sName,(long)sVal);
		return sa.Return(true);
	}
	if(iType==OT_FLOAT)
	{
		f32 fVal=sa.GetFloat(SQ_PARAM3);
		actor->AddAttribute(sName,(f32)fVal);
		return sa.Return(true);
	}
	DEBUG_ASSERT_SCRIPT(0,"Wrong parameter type for attribute!");
	return sa.Return(false);
}

int actor_GetAttribute(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CActor* actor=getActor(sa,SQ_PARAM1);
	if(!actor){
		return SQ_OK;
	}
	CString s;
	long l=0;
	f32 f=0;
	CString sName;
	int iType=sa.GetType(SQ_PARAM2);
	if(iType==OT_INTEGER){
		actor->data.p_Attributes.getOptionNameByIndex(sa.GetInt(SQ_PARAM2),sName);
		iType=actor->data.p_Attributes.getOptionType(sName,&s,&l,&f);
		SquirrelObject res=SquirrelVM::CreateTable();
		res.SetValue("name",sName.GetBuffer(0));
		if(iType==1)
		{
			res.SetValue("value",s.GetBuffer(0));
			return sa.Return(res);
		}
		if(iType==2)
		{
			res.SetValue("value",l);
			return sa.Return(res);
		}
		if(iType==3)
		{
			res.SetValue("value",f);
			return sa.Return(res);
		}
		return SQ_OK;
	}
	sName=sa.GetString(SQ_PARAM2);
	if(sName.GetLength()){
		iType=actor->data.p_Attributes.getOptionType(sName,&s,&l,&f);
		if(iType==1)
		{
			return sa.Return(s);
		}
		if(iType==2)
		{
			return sa.Return(l);
		}
		if(iType==3)
		{
			return sa.Return(f);
		}
	}
	return SQ_OK;
}

int actor_GetAttributeN(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CString sName=sa.GetString(SQ_PARAM2);
	CActor* actor=getActor(sa,SQ_PARAM1);
	if(!actor){
		return SQ_OK;
	}
	CString s;
	long l=0;
	f32 f=0;
	if(sName.GetLength()){
		int iType=actor->data.p_Attributes.getOptionType(sName,&s,&l,&f);
		if(iType==1)
		{
			l=atol(s);
		}
		if(iType==3)
		{
			l=(long)f;
		}
	}
	return sa.Return(l);
}


int actor_GetAttributeS(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CActor* actor=getActor(sa,SQ_PARAM1);
	if(!actor){
		return SQ_OK;
	}
	CString sName=sa.GetString(SQ_PARAM2);
	CString s;
	long l=0;
	f32 f=0;
	if(sName.GetLength()){
		int iType=actor->data.p_Attributes.getOptionType(sName,&s,&l,&f);
		if(iType==2)
		{
			s=toString("%i",l);
		}
		if(iType==3)
		{
			s=toString("%04f",f);
		}
	}
	return sa.Return(s);
}

int actor_DelAttribute(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CActor* actor=getActor(sa,SQ_PARAM1);
	if(!actor){
		return SQ_OK;
	}
	CString sName=sa.GetString(SQ_PARAM2);
	return sa.Return(actor->DelAttribute(sName)?true:false);
}

int actor_PackAttributes(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CActor* actor=getActor(sa,SQ_PARAM1);
	if(!actor){
		return SQ_OK;
	}
	CString sMask=sa.GetString(SQ_PARAM2);
	CString aAttrs=actor->data.p_Attributes.PackString(sMask.GetLength()?sMask.GetBuffer(0):NULL);
	return sa.Return(aAttrs.GetBuffer(0));
}

int actor_AddItem(HSQUIRRELVM v)
{
	StackHandler sa(v);
	if(getLevel()==0){
		// У нас иконки добавляются иногда в отложенном режиме...
		// при выходе в меню уровня можут уже не быть! а потом добавления сработает
		return sa.Return(false);
	}
	CActor* actor=getActor(sa,SQ_PARAM1);
	CHECK_GET_ACTOR(actor);
	CString sName=sa.GetString(SQ_PARAM2);
	CString sParams,sCustomParam;
	int iType=sa.GetType(SQ_PARAM3);
	if(iType==OT_STRING){
		sParams=sa.GetString(SQ_PARAM3);
	}
	if(iType==OT_TABLE){
		SquirrelObject sqo=sa.GetObjectHandle(SQ_PARAM3);
		sParams=sqo.GetString("_spr");
		sCustomParam=sqo.GetString("_param");
	}
	return sa.Return(actor->AddItem(sName,sParams,sCustomParam)?true:false);
}

CActorItem* getActorItem(CActor* actor, StackHandler& sa, int iParamNum)
{
	int iParam2Type=sa.GetType(iParamNum);
	if(iParam2Type==OT_STRING){
		CString sName=sa.GetString(iParamNum);
		return actor->GetItemByName(sName);
	}
	if(iParam2Type==OT_INTEGER){
		int iIndex=sa.GetInt(iParamNum);
		return actor->GetItemByIndex(iIndex);
	}
	if(iParam2Type==OT_TABLE){
		SquirrelObject sqo=sa.GetObjectHandle(iParamNum);
		CString sByName=sqo.GetString("_name");
		return actor->GetItemByName(sByName);
	}
	return NULL;
}

int actor_DelItem(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CActor* actor=getActor(sa,SQ_PARAM1);
	CHECK_GET_ACTOR(actor);
	CActorItem* item=getActorItem(actor,sa,SQ_PARAM2);
	if(item){
		return sa.Return(actor->DelItem(item)?true:false);
	}
	return sa.Return(false);
}

int actor_HasItem(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CActor* actor=getActor(sa,SQ_PARAM1);
	CHECK_GET_ACTOR(actor);
	CActorItem* item=getActorItem(actor,sa,SQ_PARAM2);
	if(item){
		return sa.Return(true);
	}
	return sa.Return(false);
}

int actor_GetItem(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CActor* actor=getActor(sa,SQ_PARAM1);
	CHECK_GET_ACTOR(actor);
	CActorItem* item=getActorItem(actor,sa,SQ_PARAM2);
	if(!item){
		return sa.Return(false);
	}
	return sa.Return(item->sqoItem);
}

int actor_GetItemAttribute(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CActor* actor=getActor(sa,SQ_PARAM1);
	CHECK_GET_ACTOR(actor);
	CActorItem* item=getActorItem(actor,sa,SQ_PARAM2);
	if(!item){
		return sa.Return(false);
	}
	CString s;
	long l=0;
	f32 f=0;
	CString sName;
	sName=sa.GetString(SQ_PARAM3);
	if(sName.GetLength()){
		int iType=item->attribs.getOptionType(sName,&s,&l,&f);
		if(iType==1)
		{
			return sa.Return(s);
		}
		if(iType==2)
		{
			return sa.Return(l);
		}
		if(iType==3)
		{
			return sa.Return(f);
		}
	}
	return sa.Return(false);
}

int actor_SetItemAttribute(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CActor* actor=getActor(sa,SQ_PARAM1);
	CHECK_GET_ACTOR(actor);
	CActorItem* item=getActorItem(actor,sa,SQ_PARAM2);
	if(item){
		CString sName=sa.GetString(SQ_PARAM3);
		int iType=sa.GetType(SQ_PARAM4);
		if(iType==OT_STRING)
		{
			CString sVal=sa.GetString(SQ_PARAM4);
			item->attribs.setString(sName,sVal);
			return sa.Return(true);
		}
		if(iType==OT_INTEGER)
		{
			int sVal=sa.GetInt(SQ_PARAM4);
			item->attribs.setLong(sName,(long)sVal);
			return sa.Return(true);
		}
		if(iType==OT_FLOAT)
		{
			f32 fVal=sa.GetFloat(SQ_PARAM4);
			item->attribs.setDouble(sName,(f32)fVal);
			return sa.Return(true);
		}
	}
	return sa.Return(false);
}

int actor_GetItemByMask(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CActor* actor=getActor(sa,SQ_PARAM1);
	CHECK_GET_ACTOR(actor);
	CActorItem* item=0;
	CString sName=sa.GetString(SQ_PARAM2);
	item=actor->GetItemByMask(sName);
	if(!item){
		return sa.Return(false);
	}
	return sa.Return(item->sqoItem);
}

int actor_PutIntoPocket(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CActor* actorTo=getActor(sa,SQ_PARAM1);
	CHECK_GET_ACTOR(actorTo);
	CActor* actorFrom=getActor(sa,SQ_PARAM2);
	CHECK_GET_ACTOR(actorFrom);
	CString sItem=sa.GetString(SQ_PARAM3);
	return sa.Return(actorFrom->PutInPocket(actorTo->data.p_sName,sItem)?true:false);
}

int actor_MakePocketEmpty(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CActor* actor=getActor(sa,SQ_PARAM1);
	CHECK_GET_ACTOR(actor);
	actor->MakePocketEmpty();
	return SQ_OK;
}

int actor_SetActorEnabled(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CActor* actor=getActor(sa,SQ_PARAM1);
	CHECK_GET_ACTOR(actor);
	BOOL bEnalbe=sa.GetBool(SQ_PARAM2);
	actor->SetEnable(bEnalbe,FALSE);
	return sa.Return(true);
}

int actor_SetFollowerMode(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CActor* actor=getActor(sa,SQ_PARAM1);
	CHECK_GET_ACTOR(actor);
	if(sa.GetType(SQ_PARAM2)==OT_NULL){
		actor->AI_SetFollowTo(NULL);
	}else{
		CActor* actor2=getActor(sa,SQ_PARAM2);
		DEBUG_ASSERT_SCRIPT(actor2 && actor2->data.p_iActorType==1,toString("Attempt to set Invalid teacher!!! %s",actor2?actor2->data.p_sName:"???"));
		actor->AI_SetFollowTo(actor2);
	}
	return sa.Return(true);
}

int actor_SetActionLabel(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CActor* actor=getActor(sa,SQ_PARAM1);
	CHECK_GET_ACTOR(actor);
	CString sLabel=sa.GetString(SQ_PARAM2);
	actor->SetStatusTextOverload(sLabel);
	return sa.Return(true);
}

int actor_GetSpriteData(HSQUIRRELVM v)
{
	StackHandler sa(v);
	CActor* actor=getActor(sa,SQ_PARAM1);
	CHECK_GET_ACTOR(actor);
	SquirrelObject res=SquirrelVM::CreateTable();
	res.SetValue("Sprite",actor->data.p_Sprite.GetBuffer(0));
	res.SetValue("Body",actor->data.p_iBody);
	res.SetValue("Head",actor->data.p_iHead);
	return sa.Return(res);
}

void RegisterActorScriptMethods()
{
	SquirrelVM::CreateFunctionGlobal(actor_SendToLocation,_T("actor_SendToLocation_i"),"*");
	SquirrelVM::CreateFunctionGlobal(actor_SetProf,_T("actor_SetProfession_i"),"*");
	SquirrelVM::CreateFunctionGlobal(actor_GetProfessionStats,_T("actor_GetProfessionStats"),"*");
	SquirrelVM::CreateFunctionGlobal(actor_IsInLocation,_T("actor_IsInLocation"),"*");
	SquirrelVM::CreateFunctionGlobal(actor_IsMovingToLocation,_T("actor_IsMovingToLocation"),"*");
	SquirrelVM::CreateFunctionGlobal(actor_PauseMovements,_T("actor_PauseMovements"),"*");
	SquirrelVM::CreateFunctionGlobal(actor_PauseInteractivity,_T("actor_PauseInteractivity"),"*");
	SquirrelVM::CreateFunctionGlobal(actor_IsInteractive,_T("actor_IsInteractive"),"*");
	SquirrelVM::CreateFunctionGlobal(actor_ShowLabels,_T("actor_ShowLabels"),"*");
	SquirrelVM::CreateFunctionGlobal(game_GetTickCount,_T("game_GetTickCount"),"*");
	SquirrelVM::CreateFunctionGlobal(getTime,_T("getTime"),"*");
	SquirrelVM::CreateFunctionGlobal(actor_SwitchToAction,_T("actor_SwitchToAction"),"*");
	SquirrelVM::CreateFunctionGlobal(actor_IsActorExist,_T("actor_IsActorExist"),"*");
	SquirrelVM::CreateFunctionGlobal(actor_GetActor,_T("actor_GetActor"),"*");
	SquirrelVM::CreateFunctionGlobal(actor_GetActors,_T("actor_GetActors"),"*");
	
	SquirrelVM::CreateFunctionGlobal(actor_OrientBody,_T("actor_OrientBody"),"*");
	SquirrelVM::CreateFunctionGlobal(actor_OrientHead,_T("actor_OrientHead"),"*");
	SquirrelVM::CreateFunctionGlobal(actor_UpdatePosition,_T("actor_UpdatePosition"),"*");
	SquirrelVM::CreateFunctionGlobal(actor_StopWalking,_T("actor_StopWalking"),"*");
	SquirrelVM::CreateFunctionGlobal(actor_CancelWalking,_T("actor_CancelWalking"),"*");
	SquirrelVM::CreateFunctionGlobal(actor_EnsureDistance,_T("actor_EnsureDistance"),"*");
	
	SquirrelVM::CreateFunctionGlobal(actor_AddAttribute,_T("actor_AddAttribute"),"*");
	SquirrelVM::CreateFunctionGlobal(actor_AddAttribute,_T("actor_SetAttribute"),"*");
	SquirrelVM::CreateFunctionGlobal(actor_GetAttribute,_T("actor_GetAttribute"),"*");
	SquirrelVM::CreateFunctionGlobal(actor_GetAttributeN,_T("actor_GetAttributeN"),"*");
	SquirrelVM::CreateFunctionGlobal(actor_GetAttributeS,_T("actor_GetAttributeS"),"*");
	SquirrelVM::CreateFunctionGlobal(actor_DelAttribute,_T("actor_DelAttribute"),"*");
	SquirrelVM::CreateFunctionGlobal(actor_PackAttributes,_T("actor_PackAttributes"),"*");

	SquirrelVM::CreateFunctionGlobal(actor_GetActorPos,_T("actor_GetActorPos"),"*");
	SquirrelVM::CreateFunctionGlobal(actor_SetActorPos,_T("actor_SetActorPos"),"*");
	SquirrelVM::CreateFunctionGlobal(actor_RandomizePos,_T("actor_RandomizePos"),"*");
	SquirrelVM::CreateFunctionGlobal(actor_CreateActor,_T("actor_CreateActor"),"*");
	SquirrelVM::CreateFunctionGlobal(actor_CloneActor,_T("actor_CloneActor"),"*");
	SquirrelVM::CreateFunctionGlobal(actor_DeleteActor,_T("actor_DeleteActor"),"*");
	
	SquirrelVM::CreateFunctionGlobal(actor_SetActorEnabled,_T("actor_SetActorEnabled"),"*");
	SquirrelVM::CreateFunctionGlobal(actor_SetFollowerMode,_T("actor_SetFollowerMode"),"*");
	SquirrelVM::CreateFunctionGlobal(actor_SetActionLabel,_T("actor_SetActionLabel"),"*");
	SquirrelVM::CreateFunctionGlobal(actor_GetSpriteData,_T("actor_GetSpriteData"),"*");
	

	SquirrelVM::CreateFunctionGlobal(actor_AddItem,_T("actor_AddItem"),"*");
	SquirrelVM::CreateFunctionGlobal(actor_DelItem,_T("actor_DelItem"),"*");
	SquirrelVM::CreateFunctionGlobal(actor_HasItem,_T("actor_HasItem"),"*");
	SquirrelVM::CreateFunctionGlobal(actor_GetItem,_T("actor_GetItem"),"*");
	SquirrelVM::CreateFunctionGlobal(actor_GetItemAttribute,_T("actor_GetItemAttribute"),"*");
	SquirrelVM::CreateFunctionGlobal(actor_SetItemAttribute,_T("actor_SetItemAttribute"),"*");
	SquirrelVM::CreateFunctionGlobal(actor_GetItemByMask,_T("actor_GetItemByMask"),"*");
	
	

	SquirrelVM::CreateFunctionGlobal(actor_PutIntoPocket,_T("actor_PutIntoPocket"),"*");
	SquirrelVM::CreateFunctionGlobal(actor_MakePocketEmpty,_T("actor_MakePocketEmpty"),"*");
}