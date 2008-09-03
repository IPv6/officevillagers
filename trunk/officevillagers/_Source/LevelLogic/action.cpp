#include "StdAfx.h"
#include "level.h"
#include "actor.h"
#include "action.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

#define STEPON_CLASS "StepOn"
#define STEPIN_CLASS "StepIn"
#define STEPOFF_CLASS "StepOff"
#define ENABLON_CLASS "EnablOn"
#define ENABLOFF_CLASS "EnablOff"
#define STEP_SECTION_NAME	"\n[Step]"
#define EVENT_SECTION_NAME	"\n[Event]"
#define SAVESAFERESET	"#"
CAction::CAction()
{
	actionScriptMirror=SquirrelVM::CreateTable();
	lastActivationTime=0;
	m_parentAction=0;
	p_bProtected=0;
	p_bDisableUI=0;
	p_sSaveSafe="";
	p_iThinkSpeed=0;
	p_WalkSpeed=1.0f;
	p_iPriority=0;
	p_PrqCanAffectBusyActors=0;
	iActionUsageCount=0;
	p_UniqueUsage=0;
	p_PrqIncutscene=0;
};

CAction::~CAction()
{
	actionScriptMirror.Reset();
}

void CAction::CopyFrom(CAction& other,BOOL bNonUniqOnly)
{
	if(!bNonUniqOnly){
		p_sActionName=other.p_sActionName;// Уникальное имя, ключ
		p_sNameForHUD=other.p_sNameForHUD;// имя, которое пишется когд перс выполняет это действие
	}
	p_PrqProphessions=other.p_PrqProphessions;
	p_PrqLocations=other.p_PrqLocations;
	p_PrqActionsNot=other.p_PrqActionsNot;
	p_PrqActionsOk=other.p_PrqActionsOk;
	p_PrqOfficeAttrsOk=other.p_PrqOfficeAttrsOk;
	p_PrqIncutscene=other.p_PrqIncutscene;
	p_PrqCanAffectBusyActors=other.p_PrqCanAffectBusyActors;
	sScriptBegin=other.sScriptBegin;
	sScriptIn=other.sScriptIn;
	sScriptIn2=other.sScriptIn2;
	sScriptEnd=other.sScriptEnd;
	m_parentAction=other.m_parentAction;
	p_bProtected=other.p_bProtected;
	p_bDisableUI=other.p_bDisableUI;
	p_sSaveSafe=other.p_sSaveSafe;
	p_iPriority=other.p_iPriority;
	p_iThinkSpeed=other.p_iThinkSpeed;
	p_UniqueUsage=other.p_UniqueUsage;
	iActionUsageCount=other.iActionUsageCount;
	p_WalkSpeed=other.p_WalkSpeed;
	actionScriptMirror=other.actionScriptMirror.Clone();
}

CAction* CAction::Add2LevelFromDescription(const CString &sDsc, const char* szFile)
{
	_array_sc events=Inifile2StrArray(sDsc,EVENT_SECTION_NAME);
	for(u32 i=0;i<events.size();i++)
	{
		CString sActionStep=events[i].c_str();
		CAction* stepAction=Add2LevelFromDescriptionSingleSet(sActionStep,szFile, 0);
		stepAction->m_parentAction=NULL;
	}
	return 0;
}

CAction* CAction::Add2LevelFromDescriptionSingleSet(const CString &sDsc, const char* szFile, int iStepNum)
{
	DEBUG_LASTACTION(toString("Loading action from %s",szFile));
	CAction* newOne=new CAction();
	getLevel()->getCurrentAction()=newOne;
	CString s;
	CString sDscInit=sDsc;
	int iLeft=sDscInit.Find(STEP_SECTION_NAME);
	if(iLeft>0){
		// У нас ивент, берем начало...
		sDscInit=sDscInit.Left(iLeft);
	}
	if(isParam(sDscInit,"-Copy.From:",s)){
		CString sBaseAction=getSTR(s,0);
		CAction* act=getLevel()->getActionByName(sBaseAction);
		if(act){
			newOne->CopyFrom(*act,TRUE);
		}
	}
	if(isParam(sDscInit,"-Name:",s)){
		newOne->p_sActionName=getSTR(s,0);
	}else{
		CString sFilRel=szFile;
		RemoveAbsPath(sFilRel);
		newOne->p_sActionName=toString("%s_%i",sFilRel,iStepNum);
	}
	{// Текст для подсказки
		if(isParam(sDscInit,"-HUDHint:",s)){
			newOne->p_sNameForHUD=getSTR(s,0);
		}
		if(newOne->p_sNameForHUD.GetLength()==0){
			newOne->p_sNameForHUD=CString("ACTIONLABEL.")+newOne->p_sActionName;
		}
		if(newOne->p_sNameForHUD.GetLength()){
			newOne->p_sNameForHUD=_ll(newOne->p_sNameForHUD,"","text\\action_names.lng");//_l(newOne->p_sNameForHUD);
		}
	}
	if(isParam(sDscInit,"-Prq.Prophessions:",s)){
		_array_sc validProfs=Str2Array(getSTR(s,0),',',false);
		for(u32 i=0;i<validProfs.size();i++)
		{
			newOne->p_PrqProphessions.push_back(getLevel()->getProphessionByName(validProfs[i].c_str()));
		}
	}
	if(isParam(sDscInit,"-Prq.Locations:",s)){
		_array_sc listing=Str2Array(getSTR(s,0),',',false);
		for(u32 i=0;i<listing.size();i++)
		{
			newOne->p_PrqLocations.push_back(getLevel()->getLocationByName(listing[i].c_str()));
		}
	}
	if(isParam(sDscInit,"-Prq.ActionsNot:",s)){
		newOne->p_PrqActionsNot=Str2Array(getSTR(s,0),',',false);
	}
	if(isParam(sDscInit,"-Prq.Actions:",s)){
		newOne->p_PrqActionsOk=Str2Array(getSTR(s,0),',',false);
	}
	if(isParam(sDscInit,"-Prq.IsGlobalEvent:",s)){
		newOne->p_PrqCanAffectBusyActors=getS32(s,0);
	}
	if(isParam(sDscInit,"-Prq.OfficeAttribute:",s)){
		newOne->p_PrqOfficeAttrsOk=Str2Array(getSTR(s,0),',',false);
	}
	if(isParam(sDscInit,"-Prq.InCutscene:",s)){
		newOne->p_PrqIncutscene=getS32(s,0);
	}
	if(isParam(sDscInit,"-Protected:",s)){
		newOne->p_bProtected=getS32(s,0);
	}
	if(isParam(sDscInit,"-Disable.Person.UI:",s)){
		newOne->p_bDisableUI=getS32(s,0);
	}
	if(isParam(sDscInit,"-Think.Speed:",s)){
		newOne->p_iThinkSpeed=getS32(s,0);
	}
	if(isParam(sDscInit,"-SaveSafe:",s)){
		newOne->p_sSaveSafe=getS32(s,0);
	}
	if(isParam(sDscInit,"-Walk.Speed:",s)){
		newOne->p_WalkSpeed=getF32(s,0);
	}
	if(isParam(sDscInit,"-Priority:",s)){
		newOne->p_iPriority=getS32(s,0);
	}
	if(isParam(sDscInit,"-UniqueUsage:",s)){
		newOne->p_UniqueUsage=getS32(s,0);
	}
	CString sFN=CamelString(newOne->p_sActionName,TRUE);
	if(isParam(sDscInit,"-Script.On:",s)){
		newOne->sScriptBegin=getGame().scripter.TurnCodeIntoFunction(s,sFN,STEPON_CLASS,"thisActor");
	}
	if(isParam(sDscInit,"-Script.In:",s)){
		newOne->sScriptIn=getGame().scripter.TurnCodeIntoFunction(s,sFN,STEPIN_CLASS,"thisActor");
	}
	if(isParam(sDscInit,"-Script.In.Alt:",s)){
		newOne->sScriptIn2=getGame().scripter.TurnCodeIntoFunction(s,sFN+"_Alt",STEPIN_CLASS,"thisActor");
	}
	if(isParam(sDscInit,"-Script.Off:",s)){
		newOne->sScriptEnd=getGame().scripter.TurnCodeIntoFunction(s,sFN,STEPOFF_CLASS,"thisActor");
	}
	if(isParam(sDscInit,"-Script.Restore:",s)){
		newOne->sScriptRestore=getGame().scripter.TurnCodeIntoFunction(s,sFN+"_Rst",STEPON_CLASS,"thisActor");
	}
	
	if(iStepNum==0){
		_array_sc steps=Inifile2StrArray(sDsc,STEP_SECTION_NAME);
		for(u32 i=0;i<steps.size();i++)
		{
			CString sActionStep=steps[i].c_str();
			CAction* stepAction=Add2LevelFromDescriptionSingleSet(sActionStep,szFile, iStepNum+i+1);
			stepAction->m_parentAction=newOne;
		}
	}
	newOne->actionScriptMirror.SetValue("Name",newOne->p_sActionName.GetBuffer(0));
	newOne->actionScriptMirror.SetValue("Priority",newOne->p_iPriority);
	newOne->actionScriptMirror.SetValue("UniqueUsage",newOne->p_UniqueUsage);
	getLevel()->actions.push_back(newOne);
	// А также сразу кладем по картам!!! //TODO
	getLevel()->getCurrentAction()=0;
	return newOne;
}

void CAction::UpdateScriptMirror()
{
	actionScriptMirror.SetValue("LastActivationTime",int(lastActivationTime));
	actionScriptMirror.SetValue("ActionUsageCount",iActionUsageCount);
}

BOOL CAction::OnAttach(CActor* who)
{
	DEBUG_ASSERT(!getLevel()->getCurrentAction(),toString("Action cross-sync error attach! actor=%s, action=%s",who->data.p_sName,p_sActionName));

	DEBUG_LASTACTION(toString("Actor '%s' Script.On method, action=%s",who->data.p_sName,this->p_sActionName));
	getLevel()->getCurrentAction()=this;
	iActionUsageCount++;
	who->actionAttachDone=0;
	who->scriptMirror.SetValue("_StepInThreadMethod",sScriptIn.GetBuffer(0));
	who->scriptMirror.SetValue("_StepInThreadMethodAlt",sScriptIn2.GetBuffer(0));
	who->scriptMirror.SetValue("_StepInThreadType",0);
	who->scriptMirror.SetValue("_StepInThread",0);
	who->scriptMirror.SetValue("Action",p_sActionName.GetBuffer(0));
	who->data.p_CurrentActionName=p_sActionName;
	getLevel()->getCurrentAction()=0;
	DEBUG_LASTACTION("");
	return 0;
}

BOOL CAction::OnDuring(CActor* who)
{
	DEBUG_ASSERT(!getLevel()->getCurrentAction(),toString("Action cross-sync error during! actor=%s, action=%s",who->data.p_sName,p_sActionName));

	getLevel()->getCurrentAction()=this;
	lastActivationTime=CLevelThinker::getThinker()->getGameTimer();//game_GetTickCount
	DEBUG_LASTACTION(toString("Actor '%s' Script.On method, action=%s",who->data.p_sName,this->p_sActionName));
	if(who->actionAttachDone==-1){
		who->actionAttachDone=0;
		if(sScriptRestore.GetLength()){
			who->scriptMirror.SetValue("__action_step",-1);
			getGame().scripter.CallPrecompiledMethod(sScriptRestore,who->scriptMirror,STEPON_CLASS);
		}
	}
	if(who->actionAttachDone==0){
		who->actionAttachDone=1;
		if(sScriptBegin.GetLength()){
			who->scriptMirror.SetValue("__action_step",1);
			getGame().scripter.CallPrecompiledMethod(sScriptBegin,who->scriptMirror,STEPON_CLASS);
		}
	}
	DEBUG_LASTACTION(toString("Actor '%s' Script.In method, action=%s",who->data.p_sName,this->p_sActionName));
	if(sScriptIn.GetLength()){
		who->scriptMirror.SetValue("__action_step",2);
		getGame().scripter.CallPrecompiledMethod("runActorStepInThread",who->scriptMirror);
	}
	getLevel()->getCurrentAction()=0;
	DEBUG_LASTACTION("");
	return 0;
}

BOOL CAction::OnDetach(CActor* who)
{
	DEBUG_ASSERT(!getLevel()->getCurrentAction(),toString("Action cross-sync error detach! actor=%s, action=%s",who->data.p_sName,p_sActionName));

	getLevel()->getCurrentAction()=this;
	iActionUsageCount--;
	DEBUG_LASTACTION(toString("Actor '%s' Script.Off method, action=%s",who->data.p_sName,this->p_sActionName));
	if(sScriptEnd.GetLength()){
		who->scriptMirror.SetValue("__action_step",3);
		getGame().scripter.CallPrecompiledMethod(sScriptEnd,who->scriptMirror,STEPOFF_CLASS);
	}
	//who->SetStatusTextOverload("");
	who->scriptMirror.SetValue("Action","");
	getLevel()->getCurrentAction()=0;
	DEBUG_LASTACTION("");
	return 0;
}

CAction* CAction::FindBestActionForNow(CActor* who)
{
	CActionLookupParams lkp;
	return FindBestActionForNow(who,lkp);
}

CAction* CAction::FindBestActionForNow(CActor* who, const CActionLookupParams& lkp)
{
	_array_CActionsP res;
	CAction* rqParent=who->p_CurrentAction? who->p_CurrentAction->m_parentAction:NULL;
	for(u32 i=0;i<getLevel()->actions.size();i++)
	{
		CAction* ac=getLevel()->actions[i];
		// Все действия предназначенные для переключения через скрипт
		if(ac->p_iPriority < 0){// Таких у нс непониманиями Вадима большинство... увы :(
			continue;
		}
		// Отсечка по занятости
		if(!ac->p_PrqCanAffectBusyActors && who->p_CurrentAction){
			continue;
		}

		/*if(ac->p_PrqCanAffectBusyActors && rqParent==ac){
			// Глобальное событие стоящее в корне цепи не должно красть задачу при выполнении действий из своей цепи
			continue;
		}*/

		// Отсечка по паренту. Смотрим либо глобальные либо действия в тойже цепи
		if(ac->m_parentAction && ac->m_parentAction!=rqParent){
			continue;
		}

		// Отсечка по приоритету
		if(ac->p_iPriority < lkp.iMinPriority){
			continue;
		}
		
		// Отсечка по уникальности использования
		if(ac->p_UniqueUsage && ac->iActionUsageCount >= ac->p_UniqueUsage){
			continue;
		}

		// Отсечка по профессии
		if(ac->p_PrqProphessions.size()){
			if(ac->p_PrqProphessions.linear_search(who->p_Profession)==-1){
				continue;
			}
		}

		// Отсечка по местоположению
		if(ac->p_PrqLocations.size()){
			BOOL bOk=FALSE;
			for(u32 i=0;i<ac->p_PrqLocations.size();i++){
				if(ac->p_PrqLocations[i]->IsInLocation(who)){
					bOk=TRUE;
					break;
				}
			}
			if(!bOk){
				continue;
			}
		}
		// По действию
		if(ac->p_PrqActionsOk.size() && who->p_CurrentAction){
			BOOL bOk=FALSE;
			for(u32 i=0;i<ac->p_PrqActionsOk.size();i++){
				if(strcmp(ac->p_PrqActionsOk[i].c_str(),who->p_CurrentAction->p_sActionName)==0){
					bOk=TRUE;
					break;
				}
			}
			if(!bOk){
				continue;
			}
		}
		// По НЕ действию
		if(ac->p_PrqActionsNot.size() && who->p_CurrentAction){
			BOOL bOk=TRUE;
			for(u32 i=0;i<ac->p_PrqActionsNot.size();i++){
				if(strcmp(ac->p_PrqActionsNot[i].c_str(),who->p_CurrentAction->p_sActionName)==0){
					bOk=FALSE;
					break;
				}
			}
			if(!bOk){
				continue;
			}
		}
		{// Доступность в катсцене
			if(ac->p_PrqIncutscene==0 && getLevel()->bIsCutscene){
				continue;
			}
			if(ac->p_PrqIncutscene>0 && !getLevel()->bIsCutscene){
				continue;
			}
		}
		// По наличию атрибута в оффисе
		if(ac->p_PrqOfficeAttrsOk.size() && getLevel()->data.pMainOfficeActor){
			BOOL bOk=TRUE;
			for(u32 i=0;i<ac->p_PrqOfficeAttrsOk.size();i++){
				BOOL bInv=0;
				const char* szAttr=ac->p_PrqOfficeAttrsOk[i].c_str();
				if(szAttr[0]=='!'){
					szAttr++;
					bInv=1;
				}
				int val=getLevel()->data.pMainOfficeActor->data.p_Attributes.getAsBool(szAttr);
				if(!val && !bInv){
					bOk=FALSE;
					break;
				}
				if(val && bInv){
					bOk=FALSE;
					break;
				}
			}
			if(!bOk){
				continue;
			}
		}
		res.push_back(ac);
	}
	if(!res.size()){
		return 0;
	}
	//return res.size()?res[rnd(0,res.size())]:0;
	//Ищем с максимальным приоритетом
	CAction* retVal=res[0];
	if(res.size()>1){
		int iMinPr=res[0]->p_iPriority;
		for(u32 i=0;i<res.size();i++){
			if((res[i]->p_iPriority>iMinPr) || (res[i]->p_iPriority==iMinPr && rnd(0,100)>50)){
				iMinPr=res[i]->p_iPriority;
				retVal=res[i];
			}
		}
	}
	return retVal;
}