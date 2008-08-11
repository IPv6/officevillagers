#include "StdAfx.h"
#include "level.h"
#include "actor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

CLevel::CLevel()
{
	floor=0;
	bIsCutscene=0;
	bCutsceneType=0;
	timeMsPrevious=-1;
	cursorOnScreenSide=0;
	data.pMainOfficeActor=0;
	bLoadingFromDefaultState=0;
	bRecalConnections=0;
	progressCurSlider=0;
	progressCruSlider=0;
	progressTimSlider=0;
	fProgressTime=0;
	fProgressCur=0;
	isCrunch=0;
	data.lProgress_TimeMax=120;//2 min
	fHintTimeTo=0.0f;
	iHintProirity=0;
	levelNavigator=new CLocationManagerNavigatorY();
}

CLevel::~CLevel()
{
#ifdef _DEBUG
	CLocationManager& mn=CLocationManager::getInstance();
	mn.Save();
#endif
#ifndef _DEBUG
	DEBUG_ASSERT(mn.bChanged==0,"Error! some location modifications detected!!!");
#endif
	for(u32 i=0;i<dataLevelInfo.prophessions.size();i++)
	{
		if(dataLevelInfo.prophessions[i])
		{
			delete dataLevelInfo.prophessions[i];
		}
	}
	for(u32 i=0;i<actions.size();i++)
	{
		if(actions[i])
		{
			delete actions[i];
		}
	}
	data.ClearActors();
	// Это ПОСЛЕ актеров, так как в актерах итемы могут юать...
	for(u32 i=0;i<items.itemDsc.size();i++)
	{
		if(items.itemDsc[i])
		{
			delete items.itemDsc[i];
		}
	}
	delete levelNavigator;
	CLocationManager::getInstance().ClearAll();
	CLocationManager::getInstance(1).ClearAll();
}

void CLevel::CLevelSaveData::ClearActors()
{
	pMainOfficeActor=0;
	for(int i=0;i<actors.GetSize();i++)
	{
		CActor* act=actors[i];
		if(act)
		{
			act->StopBodyMotion(TRUE);
			delete act;
		}
	}
	actors.RemoveAll();
}

BOOL& isLevelLoading()
{
	static BOOL b=0;
	return b;
}

CLevel*& getLevel()
{
	static CLevel* lvl=0;
	return lvl;
}

CLevelThinker::CLevelThinker(CLevel* _lvl)
{
	lvl=_lvl;
	levelInitTime=0;
	if(getLevel()==0)
	{
		getLevel()=lvl;
	}
	if(getThinker()==0)
	{
		getThinker()=this;
	}
	pauseInitOk=0;
	setDiscretion(THINKING_LEVEL_DISCRETION);
	attachToTimer(1);
}

BOOL CLevelThinker::haltAnimator()
{
	if(__super::haltAnimator()){
		if(lvl)
		{
			if(getLevel()==lvl)
			{
				getLevel()->OnLevelClose();
				getLevel()=0;
			}
			lvl->drop();
			lvl=0;
		}
		if(getThinker()==this)
		{
			getThinker()=0;
		}
	}
	return FALSE;
}

u32 CLevelThinker::getGameTimer()
{
	u32 timeSpan=u32(runningTime+(getTick()-lastTimeAbsolute)*getGame().GetTimerSpeed(1));
	return timeSpan;
}

CLevelThinker::~CLevelThinker()
{
	//DEBUG_ASSERT(lvl==0,"WTF????");
}

void CLevelThinker::animateNodeX(CNodeBasement* node, f32 delta, u32 timeMs)
{
	if(lvl->dwLevelState<=LEVELSTATE_BEFORGM){
		lvl->dwLevelState=LEVELSTATE_INGAME;
		lvl->dwLevelStateSetTime=0;
		// Инициализации...
		// Прогресс бар
		lvl->UpdateProgress();
		levelInitTime=timeMs;
	}
	lvl->Think(timeMs);
	if(!pauseInitOk)
	{
		pauseInitOk=1;
		lvl->OnChangePauseState();
	}
	if(lvl->dwLevelState==LEVELSTATE_INGAME && (timeMs-levelInitTime)>THINKING_DISCRETION*3+1)
	{
		lvl->dwLevelState=LEVELSTATE_INGAME_ACTIVE;
	}
}

CAction*& CLevel::getCurrentAction()
{
	static CAction* act=0;
	return act;
}

void CLevel::UpdateDynLocations()
{
	for(int i=0;i<data.actors.GetSize();i++)
	{
		CActor* actor=data.actors[i];
		if(actor)
		{
			actor->AI_SyncPosition(0);
		}
	}
}

void CLevel::Think(u32 timeMs,BOOL bFastForward)
{
	if(timeMsPrevious<0){
		timeMsPrevious=timeMs;
	}
	float lProgress_TimeNowBefore=data.lProgress_TimeNow;
	f32 delta=f32(timeMs-timeMsPrevious)*0.001f;
	data.lProgress_TimeNow+=delta;
	data.lProgress_TimeAbs+=delta;// Всегда!!!!
	fProgressTime=data.lProgress_TimeNow/data.lProgress_TimeMax;
	if(fProgressTime>1.0f){
		fProgressTime=1.0f;
	}
	if(fHintTimeTo>0.0f && getTime()>fHintTimeTo){
		ShowHint("",0.0f,iHintProirity);
		fHintTimeTo=0.0f;
	}
	static u32 callCnt=0;
	UpdateDynLocations();
	for(int i=0;i<data.actors.GetSize();i++)
	{
		CActor* actor=data.actors[i];
		if(actor)
		{
			int iThinkSpeed=0;
			if(actor->p_CurrentAction){
				iThinkSpeed=actor->p_CurrentAction->p_iThinkSpeed;
			}
			if(iThinkSpeed<0){
				// Эти не думают
				continue;
			}
			if(!iThinkSpeed){
				iThinkSpeed=(THINKING_DISCRETION/THINKING_LEVEL_DISCRETION);
			}
			if((callCnt%iThinkSpeed)==0)
			{
				actor->Think(timeMs);
			}
		}
	}
	callCnt++;
	{//Clearing actors
		if(actorsToDelete.GetSize()>0){
			// Удаляем актеров
			for(int i=0;i<actorsToDelete.GetSize();i++){
				CActor* ttd=actorsToDelete[i];
				//DEBUG_ASSERT(ttd->scriptMirror);
				for(int k=0;k<data.actors.GetSize();k++){
					if(data.actors[k]==ttd){
						// пРОВЕРЯЕМ КАРМАНЫ
						ttd->MakePocketEmpty();
						data.actors.RemoveAt(k);
						delete ttd;
						break;
					}
				}
			}
			actorsToDelete.RemoveAll();
		}
	}
	if(bRecalConnections){
		bRecalConnections=0;
		levelNavigator->RecalcConnections();
	}
	timeMsPrevious=timeMs;
}

void CLevel::LoadLevelFromDescription(CSpriteNode* out,const char* sDlgContent)
{
	CLevel* plvl=new CLevel();
	CLevel& lvl=(*plvl);
	lvl.dwLevelState=LEVELSTATE_INIT;
	lvl.dwLevelStateSetTime=0;
	lvl.levelRoot=out;
	attachAnimator(out, new CLevelThinker(plvl));
	CString s;
	if(isParam(sDlgContent,"-OfficeDescription:",s)){
		// Читаем файл с поддержкой слития нескольких кусков
		//StorageNamed::CIrrXmlStorage storage(getDataPath(s),true);
		CString sContent;
		ReadFileTxt(getDataPath(s),sContent);
		StorageNamed::CIrrXmlStorage storage;
		storage.LoadFromString(sContent);
		lvl.dataLevelInfo.DoSerialization(&storage);
	}
	lvl.dwLevelState=LEVELSTATE_BEFORGM;
}

void CLevel::CheckLokReashability()
{
	// Сверяем все ли точки находятся в путях проходимости
	CString sErrs;
	CLocationManager& mn=CLocationManager::getInstance(CurrentLocManager());
	for(u32 i=0;i<mn.locations.size();i++){
		CString sLocName=mn.locations[i]->szName;
		if(sLocName.Find(DYNLOCATOR_PREFIX)!=-1){
			continue;
		}
		if(sLocName.Find("FurniWALL")!=-1){
			continue;
		}
		_v2 dot(mn.locations[i]->x,mn.locations[i]->y);
		int iRes=getLevel()->levelNavigator->EnsureWalkablePos(dot);
		if(iRes!=WALKSTATE_IN)
		{
			sErrs+=sLocName;
			sErrs+=" ";
		}
	}
	if(sErrs.GetLength()){
		GfxMessageBox(toString("Unreachable locations found!\n%s",sErrs));
	}else{
		GfxMessageBox("All ok!");
	}
}

void CLevel::CSerializableLevelInfo::ApplySerialization()
{
	CLevel& lvl=(*getLevel());
	{// Карта офиса
		lvl.floor=addBillboard3(sOfficeMapDescription,_v3(0.0f,0.0f,1.0f),lvl.levelRoot);
		lvl.floorNodeID=lvl.floor->getID();
	}
	{
		MEASURE_THIS;
		_d2 navidotsScale=getNode("officeFloor")->getSize();

		// Навигационные круги
		CLocationManager& mnlevelNavigator=CLocationManager::getInstance(1);
		mnlevelNavigator.Load(sOfficeMapNavi);
		mnlevelNavigator.CheckOffscreen();
		mnlevelNavigator.ScaleWith(navidotsScale);

		// Именованные локации
		CString sNavidots=sOfficeNamedLocations;
		CurrentLocManager()=0;// Важно!!!
		CLocationManager& mnLocs=CLocationManager::getInstance(CurrentLocManager());
		mnLocs.Load(sNavidots);
		mnLocs.ScaleWith(navidotsScale);
		lvl.levelNavigator->InitFrom(&mnlevelNavigator);
	}
	CString sItemsDsc=isValidPath(sOfficeItemsDsc);
	if(sItemsDsc.GetLength()){// Загрузка описаний итемов
		StorageNamed::CIrrXmlStorage storage(sItemsDsc,true);
		lvl.items.DoSerialization(&storage);
	}
	{// Действия
		CString sAcstion=sOfficeActionDsc,sAction;
		sAcstion=getDataPath(sAcstion);
		CStringArray arrayOI;
		CString sPath=GetPathPart(sAcstion,1,1,0,0);
		CString sMask=GetPathPart(sAcstion,0,0,1,1);
		GetFolderFileListing(arrayOI,sPath,sMask);
		for(int kl=0;kl<arrayOI.GetSize();kl++)
		{
			ReadFileTxt(arrayOI[kl],sAction);
			PreParse(sAction,0,0);
			CAction* act=CAction::Add2LevelFromDescription(sAction,arrayOI[kl]);
		}
	}
	{// загрузка сохраненной игры
		lvl.data.ClearActors();
		CString sFile=getGame().getProfile().uo.getString("SaveFile");
		if(sFile.GetLength()==0 || !isFileExist(getDataPath(sFile)) || getDataPath(sFile)==getDataPath(sOfficeDefaultState))
		{
			sFile=sOfficeDefaultState;
			lvl.bLoadingFromDefaultState=TRUE;
			DEBUG_LASTACTION(toString("Loading save from %s",sFile));
			//StorageNamed::CIrrXmlStorage storage(getDataPath(sFile),true);lvl.data.DoSerialization(&storage);
			CString sContent;
			StorageNamed::CIrrXmlStorage storage;
			ReadFileTxt(getDataPath(sFile),sContent);
			storage.LoadFromString(sContent);
			lvl.data.DoSerialization(&storage);
		}else{
			DEBUG_LASTACTION(toString("Loading unicode save from %s",sFile));
			StorageNamed::CIrrXmlStorage storage(getDataPath(sFile),true);
			lvl.data.DoSerialization(&storage);
		}

		// Переинициализируем итемы у актеров
		for(int i=0;i<lvl.data.actors.GetSize();i++){
			if(lvl.data.actors[i]){
				lvl.data.actors[i]->ReattachItems();
			}
		}
		// Инициализируем пути проходимости... возможно где-то чтото енаблилось пачкой
		lvl.levelNavigator->RecalcConnections();
		// Все, закончилось
		lvl.bLoadingFromDefaultState=FALSE;
	}
	// Симуляция пройденного времени
	// Скрипт-привязка
	getGame().scripter.CallPrecompiledMethod("event_OnLoadGame",lvl.data.scriptSafe);
}

void CLevel::CLevelSaveData::PrepareSerialization()
{
	sBaseSaveFile="";// Не балуемся...
	for(int i=0;i<actors.GetSize();i++)
	{
		CActor* actor=actors[i];
		if(actor && actor->p_CurrentAction && actor->p_CurrentAction->p_iClearOnSave)
		{
			actor->AI_SwitchToAction("",TRUE,TRUE);
		}
	}
}

void CLevel::CLevelSaveData::ApplySerialization()
{
	if(sBaseSaveFile.GetLength()){
		CLevel::CLevelSaveData baseOne;
		baseOne.bFaked=1;
		{// Грузим...
			//StorageNamed::CIrrXmlStorage storage(getDataPath(sBaseSaveFile),true);
			CString sContent;
			ReadFileTxt(getDataPath(sBaseSaveFile),sContent);
			StorageNamed::CIrrXmlStorage storage;
			storage.LoadFromString(sContent);
			baseOne.DoSerialization(&storage);
		}
		// Какие новые - копируем сюда
		for(s32 i=0;i<baseOne.actors.GetSize();i++){
			CActor* actNew=baseOne.actors[i];
			if(getLevel()->getActorByName(actNew->data.p_sName)==0){
				actors.Add(actNew);
				baseOne.actors[i]=NULL;
			}/*else{
				ThrowCriticalError(toString("Error loading %s\n duplicated actor %s found!!!",sBaseSavedState,actNew->data.p_sName));
			}*/
		}
		baseOne.ClearActors();
	}
	if(!bFaked){
		for(s32 i=0;i<actors.GetSize();i++){
			if(actors[i]->data.p_iActorType==-1){
				DEBUG_ASSERT(pMainOfficeActor==0,"More than one 'office' found in save file!!!");
				pMainOfficeActor=actors[i];
				DEBUG_ASSERT(pMainOfficeActor->data.p_sName==OFFICE_ACTOR,"Actor 'office' has wrong name!");
				break;
			}
		}
		DEBUG_ASSERT(pMainOfficeActor!=0,"No 'office' found in save file!!!");
		getLevel()->InitOfficeObject();
		// Восстанавливаем сейф
		BOOL bReadSafe=FALSE;
		if(sSafeFile.GetLength()>0){
			long lTracker = 0;
			//CString sSafeCnt;ReadFile(sSafeFile,sSafeCnt);
			CString sSafeCnt=sSafeFile;//DeHtmlize(sSafeCnt);
			if(Str2SQObject(sSafeCnt,scriptSafe,lTracker))
			{
				bReadSafe=TRUE;
			}
		}
		if(!bReadSafe){
			scriptSafe=SquirrelVM::CreateTable();
		}
	}
};


BOOL CLevel::SaveGame(const char* szPostfix)
{
	if(bIsCutscene){
		// Во время катсцены сохранцки не пашут... по идее сохранка должна быть в момент включения катсцены
		return FALSE;
	}
	// Скрипт-привязка
	getGame().scripter.CallPrecompiledMethod("event_OnSaveGame",data.scriptSafe);
	// Сохраняем сейф
	CString sSafeCnt;
	long lTracker = 0;
	SQObject2Str(data.scriptSafe,sSafeCnt,lTracker,SQSERIALIZEOPT_IGNORESYSSLOTS|SQSERIALIZEOPT_IGNOREUNKNOWNS);
	//data.sSafeFile=getGame().getProfile().getProfileFolder()+getGame().getProfile().getProfileFilename("",FALSE)+"_safe.ini";SaveFile(data.sSafeFile,sSafeCnt);
	data.sSafeFile=sSafeCnt;//Htmlize(data.sSafeFile);
	// Сохраняем данные
	CString sPostfix=(szPostfix && szPostfix[0])?(szPostfix):(getGame().getProfile().getProfileFilename("",FALSE)+"_level");
	CString sLevelFile=getGame().getProfile().getProfileFolder()+sPostfix+".sav";
	StorageNamed::CIrrXmlStorage storage(sLevelFile,false);
	data.DoSerialization(&storage);
	// Сохраняем еще что?
#ifndef _DEBUG
	getGame().getProfile().uo.setString("SaveFile",sLevelFile);
#endif
	return TRUE;
}

BOOL CLevel::OnLevelClose()
{
#ifndef _DEBUG
	SaveGame();
#else
	getGame().addLogLine("Warning: Cutscene Skipped game save!!!",ELL_WARNING);
#endif
	return TRUE;
}

void CLevel::InitOfficeObject()
{
	ResetOfficeObject();
}

void CLevel::ResetOfficeObject()
{
	if(data.lProgress_TimeNow>data.lProgress_TimeMax){
		//data.lProgress_TimeNow-=data.lProgress_TimeMax;
		data.lProgress_TimeNow=0;
	}
	data.pMainOfficeActor->scriptMirror.SetValue("LevelFinish",0);// Спецдля него
	data.pMainOfficeActor->scriptMirror.SetValue("LevelCrunch",0);
}

void CLevel::OnChangePauseState()
{
	CNodeBasement* nd=getNode("pauseMarker");
	if(nd)
	{
		if(getGame().getClient().GetTimerSpeed(1)<0.1f)//Берем некешированную инфу!
		{
			nd->setVisible(true);
		}else{
			nd->setVisible(false);
		}
	}
}

CProphession* CLevel::getProphessionByName(const char* szName)
{
	for(u32 i=0;i<dataLevelInfo.prophessions.size();i++)
	{
		if(dataLevelInfo.prophessions[i]->sName==szName){
			return dataLevelInfo.prophessions[i];
		}
	}
	ThrowCriticalError(toString("Undeclared prophession '%s' used",szName));
	return 0;
}

CLocation* CLevel::getLocationByName(const char* szName)
{
	/*CLocationManager& mn=CLocationManager::getInstance();
	return mn.getLocationByName(szName);*/
	return getLocationPrefixed(szName);
}

CActor* CLevel::getActorByID(int iID)
{
	for(int i=0;i<data.actors.GetSize();i++)
	{
		if(data.actors[i] && data.actors[i]->iActorID==iID)
		{
			return data.actors[i];
		}
	}
	return 0;
}

CActor* CLevel::getActorByName(const char* szName)
{
	for(int i=0;i<data.actors.GetSize();i++)
	{
		if(data.actors[i] && data.actors[i]->data.p_sName==szName)
		{
			return data.actors[i];
		}
	}
	return 0;
}

CAction* CLevel::getActionByName(const char* szName)
{
	// Warning! даже если делать на мапы, пробег нужно оставить для -CopyFrom
	if(!szName || strcmp(szName,NO_ACTION)==0){
		return 0;
	}
	for(u32 i=0;i<actions.size();i++)
	{
		if(actions[i]->p_sActionName==szName){
			return actions[i];
		}
	}
	ThrowCriticalError(toString("Undeclared action '%s' used",szName));
	return 0;
}

CActorItemDesc* CLevel::getItemDscByName(const char* szName)
{
	for(u32 i=0;i<items.itemDsc.size();i++)
	{
		if(items.itemDsc[i]->sName==szName){
			return items.itemDsc[i];
		}
	}
	return NULL;
}

void CLevel::UpdateProgress()
{
	if(data.lProgress_TimeNow>data.lProgress_TimeMax){
		data.pMainOfficeActor->scriptMirror.SetValue("LevelFinish",1);
	}
	fProgressCur=f32(data.lProgress_Cur)/f32(data.lProgress_Max);
	fProgressTime=data.lProgress_TimeNow/data.lProgress_TimeMax;
	isCrunch=0;
	if(fProgressTime>data.fProgress_Crunch){
		isCrunch=1;
		data.pMainOfficeActor->scriptMirror.SetValue("LevelCrunch",1);
	}
	if(!progressCurSlider){
		progressCurSlider=new CSliderControl(_v3(0.0f, 0.0f, 0.0f), &fProgressCur, NULL, getDataPath("gui\\level_interface_progress_cur.spr"), -1.0f, -1.0f, getNode("progress_slider"));
		progressCurSlider->SetFidelity(500.0f);
		progressCurSlider->SetDisretion(50);
		progressCurSlider->AddBBToTheMiddle((CSpriteNode*)getNode("gui_progr_curstopper"));
	}
	if(!progressTimSlider){
		progressTimSlider=new CSliderControl(_v3(0.0f, 0.0f, 0.0f), &fProgressTime, NULL, getDataPath("gui\\level_interface_progress_tim.spr"), -1.0f, -1.0f, getNode("progress_slider"));
		progressTimSlider->SetFidelity(10000.0f);
		progressTimSlider->SetDisretion(200);
		progressTimSlider->AddBBToTheMiddle((CSpriteNode*)getNode("gui_progr_timstopper"));
	}
	if(!progressCruSlider){
		progressCruSlider=new CSliderControl(_v3(0.0f, 0.0f, 0.0f), &data.fProgress_Crunch, NULL, getDataPath("gui\\level_interface_progress_cru.spr"), -1.0f, -1.0f, getNode("progress_slider"));
		progressCruSlider->SetFidelity(10000.0f);
		progressCruSlider->SetDisretion(200);
	}
}

BOOL CLevel::EnableCutscene(BOOL b)
{
	if(b){
		ShowHint("",0,100);
	}
	if(b){
		bIsCutscene++;
	}else{
		bIsCutscene--;
	}
	bCutsceneType=b;
	DEBUG_ASSERT(bIsCutscene>=0,"Bad cutscene counter!!!");
	return bIsCutscene;
}

void CLevel::ShowHint(CString sHudText, f32 fTime, int iPriority, DWORD dwColor)
{
	if(getTime()>fHintTimeTo || iPriority>=iHintProirity)
	{
		iHintProirity = iPriority;
		fHintTimeTo = getTime()+fTime;

		CSpriteNode* hudLabels=(CSpriteNode*)getNode("hudWTDMarker");
		bool b=(sHudText.GetLength()>0?true:false);
		if(bIsCutscene){
			b=false;
		}
		hudLabels->setVisible(b?true:false);
		if(b && sHudText.GetLength()){
			CTextNode* tn=(CTextNode*)getSpriteTextNode("hudWTDMarker");
			if(tn){
				if(dwColor!=0){
					tn->setTextColor(SColor(dwColor));
				}
				tn->setText(sHudText);
			}
		}
	}
	return;
}

f32 getTime()
{
	f32 f=f32(CLevelThinker::getThinker()->getGameTimer());
	f=f/1000.0f;
	return f;
}

f32 getIngameTimeAbs()
{
	return getLevel()->data.lProgress_TimeAbs;
}

CString GetStringIncEd(CString sCurrent, int* iWithIndex)
{
	int iActorNum=0;
	if(iWithIndex && (*iWithIndex)>=0){
		iActorNum=(*iWithIndex);
	}else{
		iActorNum=getGame().getProfile().uo.getLong("CreatedActorsCount");
		iActorNum++;
		getGame().getProfile().uo.setLong("CreatedActorsCount",iActorNum);
		if(iWithIndex){
			(*iWithIndex)=iActorNum;
		}
	}
	return toString("%s%i",sCurrent,iActorNum);
}

CActor* CLevel::CreateActor(CActor* actor)
{
	CActor* newOne=new CActor();
	newOne->data.CopyFrom(*actor);
	CString sIniName=actor->data.p_sName;
	while(true){
		newOne->data.p_sName=incInString(sIniName);
		if(getLevel()->getActorByName(newOne->data.p_sName)==0){
			break;
		}
		sIniName=newOne->data.p_sName;
	}
	newOne->data.p_sNameLocalized=incInString(actor->data.p_sNameLocalized);
	newOne->ApplySerialization();// Только теперь, так как у нас все поменялось
	getLevel()->data.actors.Add(newOne);
	return newOne;
}

CActor* CLevel::CreateActor(const char* szTemplate,const char* szNameOverload,const char* sOptions,const char* szLoc)
{
	CActor* newOne=new CActor();
	CString sFile=getDataPath(toString("actors\\templates\\%s.txt",szTemplate));
	StorageNamed::CIrrXmlStorage storage(sFile,true);
	storage.DelayApply(true);
	newOne->DoSerialization(&storage);
	_array_sc params;
	if(sOptions && sOptions[0]!=0){
		params=Str2Array(sOptions,'&',FALSE);
	}
	//Раздельно имя фио и генератор нелокализованного имени
	//http://www.assembla.com/wiki/show/wpg_game_ov/IiI%D0%98%D0%BC%D1%8F%D0%9F%D0%B5%D1%80%D1%81%D0%BE%D0%BD%D0%B0%D0%B6%D0%B0
	PreParse(newOne->data.p_sName, &params);// Резолвим имя
	PreParse(newOne->data.p_Sprite, &params);// Резолвим спрайт
	if(szLoc && szLoc[0]){
		newOne->data.p_posLocator=szLoc;
	}
	PreParse(newOne->data.p_posLocator, &params);// Резолвим локацию
	PreParse(newOne->data.p_CurrentActionName, &params);// Резолвим действие
	if(newOne->data.p_sName.Find("$")!=-1){
		_array_sc sNameBase=Str2Array(newOne->data.p_sName,' ',FALSE);
		newOne->data.p_sNameLocalized=newOne->data.p_sName;
		for(u32 i=0;i<sNameBase.size();i++){
			CString sNamePart=GetInstringPart("${", "}", sNameBase[i].c_str());
			CString sAllNames=_ll(sNamePart,sNamePart,"actors\\templates\\names.lng");
			_array_sc Items=Str2Array(sAllNames,' ',FALSE);
			newOne->data.p_sNameLocalized.Replace(sNameBase[i].c_str(),Items[rnd(0,Items.size())].c_str());
		}
	}
	if(!szNameOverload || szNameOverload[0]==0){
		szNameOverload="ACTOR";
	}
	newOne->data.p_sName=GetStringIncEd(szNameOverload);
	newOne->data.p_iBody=rnd(0,newOne->data.p_iBody);// Резолвим тело
	newOne->data.p_iHead=rnd(0,newOne->data.p_iHead);// Резолвим голову

	newOne->ApplySerialization();// Только теперь, так как у нас все поменялось
	getLevel()->data.actors.Add(newOne);
	// Скрипт-привязка
	if(newOne->data.p_sScriptOnCreate.GetLength()){
		getGame().scripter.CallPrecompiledMethod(newOne->data.p_sScriptOnCreate,newOne->scriptMirror);
	}
	return newOne;
}

BOOL CLevel::DeleteActor(CActor* actor)
{
	DEBUG_ASSERT(actor!=data.pMainOfficeActor,"Attempt to delete office actor detected!");
	// Скрипт-привязка
	if(actor->data.p_sScriptOnDelete.GetLength()){
		getGame().scripter.CallPrecompiledMethod(actor->data.p_sScriptOnDelete,actor->scriptMirror);
	}
	// Отложенное удаление
	actor->data.p_sName+=DELETEDACTOR_MARK;
	actor->scriptMirror.SetValue("Name",actor->data.p_sName.GetBuffer(0));
	actor->SetEnable(FALSE,TRUE);// Нужно чтобы как минимум пересчитать проходимость
	actorsToDelete.Add(actor);
	return TRUE;
}