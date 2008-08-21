#include "StdAfx.h"
#include "level.h"
#include "actor.h"
#include "../Stuff/animators.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

#define MOVE_ACCURACY		0.3f
BOOL& isLevelLoading();

//CActor::CActor
void CActor::Init()
{
	isError=0;
	p_Profession=0;
	lastThinkTime=0;
	bNodeSpawned=0;
	node=0;
	_autoClone=0;
	bMovementsIsPausedToTime=0;
	bMovementsIsPaused=0;
	isInteractive=true;
	p_CurrentAction=0;
	data.p_speedRate=1.0f;
	data.p_CurrentActionName="";
	data.p_CurrentFaceDirection=0;
	data.p_CurrentBodyDirection=_v2(1,0);
	data.fActionBeginTime=1.0f;
	data.p_fCreationTime=-1;
	actorLocationLocator=0;
	actionAttachDone=0;
	scriptMirror=SquirrelVM::CreateTable();
	static int iMaxCnt=0;
	lastHeadYawTime=0;
	lastInPocketActor=0;
	bHasBodyAndCanMove=-1;
	iActorID=ACTORNODEW_ID_MIN+(iMaxCnt%(ACTORNODEW_ID_MAX-ACTORNODEW_ID_MIN));
	iMaxCnt++;
}

CWalkData::CWalkData()
{
	lWalkCount=0;
	bDebugPath=0;
	activeNavigationDotTime=0;
	activeNavigationDotTimeStart=0;
	activeNavigationMapStepNum=-1;
	bMarkVisitedFloors=0;
}

void CActor::PrepareSerialization()
{
	data.p_pos=getPosition();
	data.p_AttributesAsStr=data.p_Attributes.PackString(NULL,"__*");
	data.p_posLocator="";
	data.p_ValidProphession.TrimLeft(" ,");
	data.p_ValidProphession.TrimRight(" ,");
	if(data.p_actorLocationEntrance.GetLength() && getLevel()->getLocationByName(data.p_actorLocationEntrance)->bNotSavable){
		data.p_actorLocationEntrance="";
	}
};

void CActor::ApplySerialization()
{
	// Профессия. ДО всего так как содержит некторые дефолты
	SetProf(data.p_ProfessionName,-1);
	data.p_ValidProphession=CString(",")+data.p_ValidProphession+CString(",");
	scriptMirror.SetValue("ValidProfessions",data.p_ValidProphession.GetBuffer(0));
	// Время
	if(data.p_fCreationTime<=0.0f){
		data.p_fCreationTime=getLevel()->data.lProgress_TimeAbs;
	}
	scriptMirror.SetValue("Location","");
	scriptMirror.SetValue("LocationEntrance","");
	if(data.p_posLocator.GetLength()>0){
		// Если указан начальный локатор, ставим на него
		// может быть и не указан - загрузка из сейв файла
		if(data.p_actorLocationInitial.GetLength()==0){
			data.p_actorLocationInitial=data.p_posLocator;
		}
		actorLocationLocator=getLevel()->getLocationByName(data.p_posLocator);
#ifdef _DEBUG
		if(Debug_ShowInfo() && data.p_Enabled){
			VisualizeRCTF(actorLocationLocator->Convert2rect(),"gui\\editor\\square.png",FALSE);
		}
#endif
		if(actorLocationLocator){//&& (data.p_iActorType <=0 || getLevel()->bLoadingFromDefaultState || )){
			data.p_pos=_v2(actorLocationLocator->x,actorLocationLocator->y);
		}
		if(data.p_actorLocationEntrance.GetLength()==0){
			CString sle=data.p_posLocator+"!";
			if(CLocationManager::getInstance().getLocationByNameSoft(sle)){
				data.p_actorLocationEntrance=sle;
			}
		}
	}
	{
		// Создаем DYNAMIC-локатор на данного актера. Динамический,несохраняемый
		actorLocationLocator=CLocationManager::getInstance().CreateDynLocator(data.p_sName,actorLocationLocator);
		if(actorLocationLocator->w<0.01f && actorLocationLocator->h<0.01f){
			if(p_Profession && p_Profession->w >= 0.01f && p_Profession->h >= 0.01){
				actorLocationLocator->w=p_Profession->w;
				actorLocationLocator->h=p_Profession->h;
			}else{
				actorLocationLocator->w=2.5f;
				actorLocationLocator->h=2.0f;
			}
		}
		// Чтобы позиция "задалась" даже у мебели и недумающих актеров
		actorLocationLocator->x=data.p_pos.X;
		actorLocationLocator->y=data.p_pos.Y;
		// Локатор входа
		if(p_Profession->bChangeNavigation && 
			(data.p_actorLocationEntrance.GetLength()==0 || CLocationManager::getInstance().getLocationByNameSoft(data.p_actorLocationEntrance)==NULL))// Локатора может не быть - если сохранился создававшийся динамически
		{// Влияет на навигацию - подход должен быть. создаем свое!!!
			if(p_Profession->bChangeNavigation<2){
				CString sDynEntrName=data.p_sName+"!";
				CLocation* entLoc=CLocationManager::getInstance().CreateDynLocator(sDynEntrName,actorLocationLocator);
				data.p_actorLocationEntrance=entLoc->szName;
				const float defDEH=1.0f;
				entLoc->y=actorLocationLocator->y-actorLocationLocator->h*0.5f-defDEH*0.5f;
				entLoc->h=defDEH;
				entLoc->w=min(entLoc->w,5.0f);
			}
		}
		// Сохраняем для скрипта
		scriptMirror.SetValue("Location",data.p_actorLocationInitial.GetBuffer(0));
		scriptMirror.SetValue("LocationEntrance",data.p_actorLocationEntrance.GetBuffer(0));
	}
	if(data.p_sNameLocalized.GetLength()==0){
		if(data.p_iActorType==1){
			data.p_sNameLocalized=_ll(data.p_sName,data.p_sName,"actors\\templates\\names.lng");
		}else{
			data.p_sNameLocalized=data.p_sName;
		}
	}
	scriptMirror.SetValue("Name",data.p_sName.GetBuffer(0));
	scriptMirror.SetValue("NameLocalized",data.p_sNameLocalized.GetBuffer(0));
	scriptMirror.SetValue("ActorType",data.p_iActorType);
	scriptMirror.SetValue("CreationTime",data.p_fCreationTime);
	scriptMirror.SetValue("BodyLock",data.p_iBodyLock);
	SetInteractive(isInteractive);
	scriptMirror.SetValue("Action","");
	scriptMirror.SetValue("ActorID",iActorID);
	scriptMirror.SetValue("pauseMovements",bMovementsIsPaused);
	{// Атрибуты
		CString sAttrs=data.p_AttributesAsStr;
		if(data.p_AttributesOverload.GetLength()>0){
			CString sAddAttr;
			ReadFile(getDataPath(data.p_AttributesOverload),sAddAttr);
			if(sAddAttr.GetLength()>0){
				sAttrs+="\n";
				sAttrs+=sAddAttr;
			}
		}
		data.p_Attributes.UnpackString(sAttrs,TRUE);
	}
	if(getLevel()->dwLevelState>=LEVELSTATE_INGAME)
	{// Итемы и анимации
		ReattachItems();
	}
	// Текущее действие
	if(data.p_CurrentActionName.GetLength()){
		AI_SwitchToAction(data.p_CurrentActionName,FALSE);
	}
	// Вызываем чтобы обновить скрипт миррор и состояние ноды
	SetEnable(data.p_Enabled, TRUE);
	// Автоклоны
	if(_autoClone>0){
		// Клонируем
		for(int i=0;i<_autoClone;i++)
		{
			CActor* newActor=new CActor();
			newActor->data.CopyFrom(*this);
			// Имя...
			newActor->data.p_sName=incInString(newActor->data.p_sName,i+1);
			newActor->data.p_sNameLocalized=incInString(newActor->data.p_sNameLocalized,i+1);
			// Локация...
			CString sLocator=incInString(newActor->data.p_posLocator,i+1);
			CLocation* loca=CLocationManager::getInstance().getLocationByNameSoft(sLocator);
			if(loca){
				// Такой локатор есть!
				newActor->data.p_posLocator=sLocator;
			}
			// Применяем все...
			newActor->ApplySerialization();
			getLevel()->data.actors.Add(newActor);
		}
	}
};

void CActor::ReattachItems()
{
	u32 i=0;
	for(i=0;i<data.p_items.size();i++)
	{
		data.p_items[i]->OnItemAttach(this);
	}
}

CActor::~CActor()
{
	for(u32 i=0;i<data.p_items.size();i++)
	{
		if(data.p_items[i])
		{
			delete data.p_items[i];
		}
	}
	if(node)
	{
		removeNode(node);
		bNodeSpawned=0;
		node=0;
	}
}

#define ACTOR_THINMOVEMENT_FLYANIMATOR	"HNOFL"
#define ACTOR_THINMOVEMENT_WALKANIM		"HNOWAL"
DWORD WINAPI SetupActorMovementer(LPVOID p1, LPVOID p2, CNodeBasement* pDeleteNode)
{
	if(isLevelLoading()){
		return 0;
	}
	CLevel* lvl=getLevel();
	if(getGame().isPaused()>=PAUSELEVEL_EXIT || !lvl || lvl->dwLevelState<=LEVELSTATE_BEFORGM){
		// Игра выключется... или перезагрузка уровня
		return 0;
	}
	// До лока - актер может быть неактуален уже!
	CFlyAnimatorX* fa=(CFlyAnimatorX*)p2;
	if(fa && fa->iRemoveReason==BY_STOP_LEVELEND){
		// Все хуже... уровень выгружается
		return 0;
	}
	// Уровень не выключается... см
	BOOL bStop=FALSE;
	CActor* actor=(CActor*)p1;
	CSmartLock cs(&actor->csactiveNavigationMap);
	if(fa && fa->iDedicatedWalkNumber!=actor->walkData.lWalkCount){
		return 0;
	}
	if(fa && fa->iRemoveReason==BY_STOP_MOTION){
		//Warning("Jump by stop_mot!");
		//получить по атрибуту причину выключения....
		//сохранят последний номер кадра анимации тела... чтобы не другать
		bStop=1;
	}
	int i=actor->walkData.activeNavigationMapStepNum;
	if(i<0 || i+1 >= int(actor->walkData.activeNavigationMap.size())){
		bStop=2;
		//Warning("Jump by amapsize!");
	}
	if(bStop){
		actor->AI_FinishWalkablePathTo();
		return 0;
	}
	actor->walkData.activeNavigationMapStepNum-=1;
	f32 zPos=DEF_ACTOR_Z+actor->posOffset.Z;
	_v3 frompos=_v3(actor->walkData.activeNavigationMap[i+1].X,actor->walkData.activeNavigationMap[i+1].Y,zPos);//_v3(p_pos.X,p_pos.Y,curpos.Z);//node?node->getPosition():_v3(data.p_pos.X,data.p_pos.Y,zPos);
	_v3 posCur=actor->node->getPosition();
	if(fabs(posCur.X-frompos.X)>0.2f || fabs(posCur.Y-frompos.Y)>0.2f) 
	{
		frompos=posCur;
	}

	_v2 topos2=actor->walkData.activeNavigationMap[i];
	BOOL bAdjusted=actor->AI_TuneCrowdBehaviour(_v2(frompos.X,frompos.Y),topos2,actor->walkData.activeNavigationMapZones[i]);
	if(bAdjusted){
		// Переносим обратно в карту...
		actor->walkData.activeNavigationMap[i]=topos2;
	}
	_v3 topos=_v3(topos2.X,topos2.Y,zPos);//_v3(p_pos.X,p_pos.Y,curpos.Z);

	_v3 newDirection=topos-frompos;
	actor->data.p_CurrentBodyDirection=_v2(newDirection.X,newDirection.Y);
	actor->data.p_CurrentFaceDirection=0;
	u32 stepTime=u32((newDirection).getLength()*actor->GetWalkSpeed()*getLevel()->dataLevelInfo.fMove2SpeedRate);
	if(stepTime==0){
		SetupActorMovementer(p1,0,0);
		return 0;
	}
	// Корректируем чтобы уложится в подсчитанное время...
	if(actor->lastThinkTime+stepTime+THINKING_DISCRETION > actor->walkData.activeNavigationDotTime){
		actor->walkData.activeNavigationDotTime=actor->lastThinkTime+stepTime+THINKING_DISCRETION;
	}
	fa=new CFlyAnimatorX(frompos,topos,stepTime);
	fa->attachToTimer(1);
	fa->setAnimatorName(ACTOR_THINMOVEMENT_FLYANIMATOR);
	fa->setOnDestroyCallback(SetupActorMovementer,p1,fa,FALSE);
	fa->iDedicatedWalkNumber=actor->walkData.lWalkCount;
	attachAnimator(actor->node,fa);
	actor->RefreshBodyAccordingDirection(actor->data.p_CurrentBodyDirection,TRUE);

#ifdef _DEBUG
	if(Debug_ShowInfo())
	{
		CTextNode* t;
		t=addText("-",frompos,getNode("officeFloor"));
		attachAnimator(t, new CSelfdestruction(2000));
		t=addText("+",topos,getNode("officeFloor"));
		attachAnimator(t, new CSelfdestruction(2000));
		//addBillboard("gui\\editor\\editor_navimap_dar.spr",_d2(1.0f,1.0f),_v3(pathDot.X,pathDot.Y,DEF_ACTOR_Z),getNode("officeFloor"));
	}
#endif
	return 0;
}

void CActor::StopBodyMotionUI(BOOL bLevelEnd)
{
	if(node){
		int iRR=bLevelEnd?BY_STOP_LEVELEND:BY_STOP_MOTION;
		CAtlasAnimation* wa=(CAtlasAnimation*)node->getAnimatorByName(ACTOR_THINMOVEMENT_WALKANIM,TRUE);
		if(wa)
		{
			// Запоминаем позицию в атласе!!!
			node->setAttrib("BODY_WALK_FRAME",long(wa->currentFrame));
			wa->removeThisAnimator();
		}
		CFlyAnimatorX* fa=(CFlyAnimatorX*)node->getAnimatorByName(ACTOR_THINMOVEMENT_FLYANIMATOR);
		if(fa)
		{
			fa->removeThisAnimator();
			fa->iRemoveReason=iRR;
		}
	}
}

void CActor::StopBodyMotion(BOOL bLevelEnd)
{
	CSmartLock cs(&csactiveNavigationMap);
	StopBodyMotionUI(bLevelEnd);
	if(walkData.activeNavigationMapStepNum==-1){
		// Уже не идем!
		return;
	}
	walkData.activeNavigationMapStepNum=-1;
	walkData.activeNavigationDotTime=0;
	walkData.activeNavigationMap.clear();
	walkData.activeNavigationMapZones.clear();
}

void CActor::RefreshHeadAccordingDirection(float fBodyAddonAngle,int iAngleShift, BOOL bRelative, BOOL bTrim)
{
	if(!bHasBodyAndCanMove){
		return;
	}
	// Голова
	CSpriteNode* nodeHead=(CSpriteNode*)getNode(SUBID_HEAD,node);
	if(!nodeHead){
		return;
	}
	f32 fBodyAngle=f32(getXYRotationFromDirection(_v3(data.p_CurrentBodyDirection.X,data.p_CurrentBodyDirection.Y,0.0f),0.0f))*180.0f/PI;
	if(!bRelative){
		fBodyAddonAngle=fBodyAddonAngle-fBodyAngle;
	}
	if(bTrim){
		fBodyAddonAngle=max(fBodyAddonAngle,-90.0f);
		fBodyAddonAngle=min(fBodyAddonAngle,90.0f);
	}
	data.p_CurrentFaceDirection=fBodyAddonAngle;
	f32 fAngleNeedle=(fBodyAngle+fBodyAddonAngle)/360.0f;
	// Двойной размер!!!
	#define RSIZ_A	14
	int recalca[RSIZ_A]={0,0,0,0,3,3,3,3,3,2,6,5,4,1};
	int iHeadNumber=(RSIZ_A*50+iAngleShift+int(fAngleNeedle*f32(RSIZ_A)))%RSIZ_A;
	nodeHead->SetAtlasSprite((f32)recalca[iHeadNumber],(f32)data.p_iHead,nodeHead->atlas_xtotal,nodeHead->atlas_ytotal);
	_v3 headPos=nodeHead->getPosition();
	if(data.p_CurrentBodyDirection.X>0){
		headPos.X=-_m::fabs(headPos.X);
	}else{
		headPos.X=_m::fabs(headPos.X);
	}
	nodeHead->setPosition(headPos);
}

void CActor::SetBodyZeroSprite()
{
	if(!bHasBodyAndCanMove || !node)
	{
		return;
	}
	static long iZeroSpr=getManifest().Settings.getLong("PersonBodyZeroSprite");
	CSpriteNode* nodeBody=(CSpriteNode*)getNode(SUBID_BODY,node);
	if(nodeBody)
	{
		f32 newSpr=f32(iZeroSpr);
		if(nodeBody->atlas_x >= nodeBody->atlas_xtotal/2)
		{
			newSpr += float(int(nodeBody->atlas_xtotal/2));
		}
		nodeBody->SetAtlasSpriteX(newSpr);
	}
}

void CActor::RefreshBodyAccordingDirection(_v2 direction,BOOL bWalking)
{
	if(!bHasBodyAndCanMove || !node)
	{
		return;
	}
	data.p_CurrentBodyDirection=direction;
	// Тулово
	CAtlasAnimation* wa=(CAtlasAnimation*)node->getAnimatorByName(ACTOR_THINMOVEMENT_WALKANIM,TRUE);
	if(wa)
	{
		node->setAttrib("BODY_WALK_FRAME",long(wa->currentFrame));
		wa->removeThisAnimator();
	}
	CSpriteNode* nodeBody=(CSpriteNode*)getNode(SUBID_BODY,node);
	CAnimationMarshrut* marshrut=new CAnimationMarshrut();
	_d2 sizerer=nodeBody->getSize();
	if(direction.Y>=0){
		marshrut->init(0,int(nodeBody->atlas_xtotal/2-1),data.p_iBody);
	}else{
		marshrut->init(int(nodeBody->atlas_xtotal/2),int(nodeBody->atlas_xtotal-1),data.p_iBody);
	}
	if(direction.X<0){
		sizerer.Width=-_m::fabs(sizerer.Width);
	}else{
		sizerer.Width=_m::fabs(sizerer.Width);
	}
	nodeBody->setSize(sizerer);
	long startFrame=node->getAttribL("BODY_WALK_FRAME")+1;
	if(startFrame>=marshrut->size()){
		startFrame=0;
	}
	wa=new CAtlasAnimation(int(GetWalkSpeed()*getLevel()->dataLevelInfo.fBody2SpeedRate),_v2(nodeBody->atlas_xtotal,nodeBody->atlas_ytotal), marshrut, startFrame);
	attachAnimator(nodeBody,wa);
	wa->setAnimatorName(ACTOR_THINMOVEMENT_WALKANIM);// После attachAnimator!!! // Хотя оно там и так установится
	wa->attachToTimer(1);
	wa->updateFrame();
	if(!bWalking)
	{
		wa->removeThisAnimator();
	}
	RefreshHeadAccordingDirection(0,0);
}

float CActor::GetWalkSpeed()
{
	float f=data.p_speedRate;
	if(p_CurrentAction){
		f/=p_CurrentAction->p_WalkSpeed;
	}
	return f;
}

#define MAX_FOLLOWER_DISTANCE	2.0f
#define MAX_FOLLOWER_DISTANCESQ	MAX_FOLLOWER_DISTANCE*MAX_FOLLOWER_DISTANCE

#define MAX_FOLLOWER_TRAGET_DISTANCE	1.4f
#define MAX_FOLLOWER_TRAGET_DISTANCESQ	MAX_FOLLOWER_TRAGET_DISTANCE*MAX_FOLLOWER_TRAGET_DISTANCE

void CActor::ThinkFollower()
{
	activeNavigationDotLocationName="";// Не можем бегать по целям...
	// Во время катсцены стоим столбом и не бегаем за учителем
	if(getLevel()->bIsCutscene){
		return;
	}
	CActor* targetActor=getLevel()->getActorByName(data.p_FollowToActor);
	if(!targetActor){
		data.p_FollowToActor="";
	}else{
		DEBUG_ASSERT(data.p_sInPocket_Actor.GetLength()==0,"Cant follow in pocket!");
		if(targetActor->bMovementsIsPaused==0){
			_v2 nowPos=getPosition();
			_v2 targetPos=targetActor->getPosition();
			targetPos+=clampByLength(nowPos-targetPos,0.01f,1.0f);
			if(
				nowPos.getDistanceFromSQ(targetPos)>MAX_FOLLOWER_DISTANCESQ
				&& targetPos.getDistanceFromSQ(lastFollowerToPosition)>MAX_FOLLOWER_TRAGET_DISTANCESQ
				//&& targetPos!=lastFollowerToPosition
				){
					lastFollowerToPosition=targetPos;
					AI_SetMove2TargetPos(targetPos);
			}
		}
	}
}

void CActor::PauseMovementsForSec(int iSecs)
{
	StopBodyMotion();
	bMovementsIsPausedToTime=-iSecs;
}

void CActor::ThinkMovements(u32 timeMs)
{
	// Использование timeMs обязательно! для быстрой прокрутки времени
	if(!bHasBodyAndCanMove)
	{
		return;
	}
	if(bMovementsIsPausedToTime!=0)
	{
		if(bMovementsIsPausedToTime<0){
			bMovementsIsPausedToTime=timeMs+(-bMovementsIsPausedToTime);
		}
		if(bMovementsIsPausedToTime>long(timeMs)){
			return;
		}
		bMovementsIsPausedToTime=0;
	}
	if(bMovementsIsPaused)
	{
		walkData.activeNavigationDotTime=0;
		StopBodyMotion();
	}
	if(!bMovementsIsPaused){
		if(data.p_FollowToActor.GetLength())
		{
			ThinkFollower();
		}
		if(walkData.activeNavigationMap.size()==0 && activeNavigationDotLocationName.GetLength()){
			// Инициируем беготню к цели
			walkData.activeNavigationDotTime=0;
			CLocation* loc=getLevel()->getLocationByName(activeNavigationDotLocationName);
			if(loc){
				if(loc->IsInLocation(getPosition())){
					// А мы уже здесь!
					activeNavigationDotLocationName="";
				}else{
					DEBUG_ASSERT(getLevel()->levelNavigator->EnsureWalkablePos(_v2(loc->x,loc->y))!=WALKSTATE_NONE,"Error: sending actor to unwalkable location!");
					BOOL bOk=AI_SetMove2TargetPos(_v2(loc->x,loc->y));
					if(walkData.activeNavigationMapZones.size())
					{// В 0(целевая позиция) заносим сейф зону - таргетный локатор!
						walkData.activeNavigationMapZones[0]=loc->Convert2rect();
					}
#ifdef _DEBUG
					if(!bOk){
						AddDebugScreenLine(toString("Debug warning: путь '%s' к %s не найден!!!",data.p_sName,activeNavigationDotLocationName),5000);
						if(Debug_ShowInfo()){
							VisualizeLINE(data.p_pos, _v2(loc->x,loc->y));
						}
					}
#endif
				}
			}else{
				activeNavigationDotLocationName="";
			}
		}
		if(walkData.activeNavigationMap.size()){
			CSmartLock cs(&csactiveNavigationMap);
			// двигаемся по навпоинтам
			_v2 currentTargetPos=walkData.activeNavigationMap[0];
			_v2 direction=(currentTargetPos-data.p_pos);
			if(direction.getLengthSQ()<MOVE_ACCURACY*MOVE_ACCURACY)
			{
				//Warning("Jump by dist!");
				AI_FinishWalkablePathTo();
				activeNavigationDotLocationName="";
			}
			if(walkData.activeNavigationDotTime && timeMs>=walkData.activeNavigationDotTime)
			{
				//Warning("Jump by time!");
				AI_FinishWalkablePathTo();
				activeNavigationDotLocationName="";
			}
			if(walkData.activeNavigationDotTime==0 && walkData.activeNavigationMap.size())
			{// движение не запрограммировано пока
				// Добавляем аниматор плавного движения
				u32 timeSpan=0;
				StopBodyMotion();
				for(int i=(walkData.activeNavigationMap.size()-1)-1;i>=0;i--)
				{
					_v2 direction=walkData.activeNavigationMap[i]-walkData.activeNavigationMap[i+1];
					f32 directionLen=(direction).getLength();
					u32 stepTime=u32(directionLen*GetWalkSpeed()*getLevel()->dataLevelInfo.fMove2SpeedRate);
					timeSpan+=stepTime;
				}
				// Округляем по частоте думания! Чтобы перс не толокся на месте
				timeSpan+=THINKING_DISCRETION-(timeSpan%THINKING_DISCRETION);
				walkData.activeNavigationDotTimeStart=timeMs;
				walkData.activeNavigationDotTime=timeMs+timeSpan;
				// Добавляем в цепочку аниматор движения
				walkData.activeNavigationMapStepNum=(walkData.activeNavigationMap.size()-1)-1;
				SetupActorMovementer(this,0,0);
			}
		}
	}
	/*// Если вычислять направление на лету...
	_v2 newDirection=data.p_pos-prevPos;
	fastNormalize(newDirection);
	if(fabs(newDirection.dotProduct(data.p_CurrentFaceDirection))<0.8f)
	{
		RefreshBodyAccordingDirection(newDirection,activeNavigationDotTime?TRUE:FALSE);
	}
	data.p_CurrentFaceDirection=newDirection;*/
	if(isInteractive){
		if(lastHeadYawTime==0 || timeMs>lastHeadYawTime)
		{
			lastHeadYawTime=timeMs+rnd(2000,6000);
			if(data.p_CurrentBodyDirection.Y<0){
				// Только если идем вниз!
				RefreshHeadAccordingDirection(rndf(-90,90));
			}
		}
	}
}

BOOL CActor::AI_SetMove2TargetPos(const char* szLocation)
{
	walkData.activeNavigationMap.clear();
	activeNavigationDotLocationName=szLocation;
	CLocation* loc=getLevel()->getLocationByName(activeNavigationDotLocationName);
	if(!loc){
		return FALSE;
	}
	_v2 posInQuestion(loc->x,loc->y);
	int iRes=getLevel()->levelNavigator->EnsureWalkablePos(posInQuestion);
	DEBUG_ASSERT(iRes==WALKSTATE_IN,toString("Actor sent to unreachable loc:\n%s (%.02f:%.02f) -> %s (%.02f:%.02f)",data.p_sName,data.p_pos.X,data.p_pos.Y,szLocation,loc->x,loc->y));
	if(iRes!=WALKSTATE_IN){
		AI_SwitchToAction(NO_ACTION,TRUE);
		return FALSE;
	}
	return TRUE;
}

BOOL CActor::AI_SetMove2TargetPos(_v2 newpos)
{
	CSmartLock cs(&csactiveNavigationMap);
	return AI_MakeWalkablePathTo(newpos);
}

void CActor::SetEnable(BOOL bEnable,BOOL bForceScriptUpdate)
{
	BOOL bWasSwitch=((bEnable && !data.p_Enabled)||(!bEnable && data.p_Enabled));
	BOOL bPrevEnable=data.p_Enabled;
	data.p_Enabled=bEnable;
	scriptMirror.SetValue("Enabled",data.p_Enabled?true:false);
	if(bEnable){
		if(bWasSwitch || bForceScriptUpdate){
			if(data.p_sScriptOnEnable.GetLength()){
				getGame().scripter.CallPrecompiledMethod(data.p_sScriptOnEnable,scriptMirror);
			}
		}
	}
	if(!bEnable){
		if(bWasSwitch || bForceScriptUpdate){
			if(data.p_sScriptOnDisable.GetLength()){
				getGame().scripter.CallPrecompiledMethod(data.p_sScriptOnDisable,scriptMirror);
			}
		}
	}
	if(!node && data.p_Enabled && !bPrevEnable){
		// Чтобы небыло блинка когда енаблящийся актер еще небыл создан - спауним его
		SpawnPersUI();
	}
	if(node && (bPrevEnable!=bEnable)){
		// Меняем видимость только если сменился енабле статус... иногда мы скриваем сами енабленных персов тоже - при посадке к примеру
		node->setVisible(data.p_Enabled?true:false);
	}
	if(getLevel()->dwLevelState>=LEVELSTATE_INGAME){
		if(bWasSwitch && p_Profession && p_Profession->bChangeNavigation)
		{
			getLevel()->ScheduleRecalcConnections();//getLevel()->levelNavigator->RecalcConnections();
		}
	}
}

void CActor::SetLocation(const _v2& newPos)
{
	data.p_pos.X=newPos.X;
	data.p_pos.Y=newPos.Y;
	getLevel()->levelNavigator->EnsureWalkablePos(data.p_pos);
	AI_SyncPosition(2);
	StopBodyMotion();
}

void CActor::SetLocation(CLocation* loc)
{
	SetLocation(_v2(loc->x,loc->y));
}

void CActor::SetProf(const char* szProf,BOOL bUIOnly)
{
	LOG_DEBUG(toString("|%s: Set prof %s",data.p_sName,szProf));
	CProphession* newOne=getLevel()->getProphessionByName(szProf);
	if(bUIOnly!=1){
		if(p_Profession){
			p_Profession->lCounter--;
		}
		p_Profession=newOne;
		p_Profession->lCounter++;
		data.p_ProfessionName=szProf;
		scriptMirror.SetValue("Profession",data.p_ProfessionName.GetBuffer(0));
		CString sProfN=newOne->sBaseName;
		if(sProfN.GetLength()==0){
			sProfN=szProf;
		}
		scriptMirror.SetValue("ProfessionN",sProfN.GetBuffer(0));
	}
	// Меняем боди
	if(!data.p_iBodyLock && bUIOnly!=-1){
		data.p_iBody=newOne->iBodySpr;
		RefreshBodyAccordingDirection(data.p_CurrentBodyDirection,FALSE);
	}
}

BOOL CActor::GetInteractive()
{
	if(getLevel()->bIsCutscene && getLevel()->bCutsceneType!=2)
	{
		return 0;
	}
	if(p_CurrentAction && (p_CurrentAction->p_bDisableUI || (p_CurrentAction->m_parentAction && p_CurrentAction->m_parentAction->p_bDisableUI)))
	{
		return 0;
	}
	return isInteractive;
}

void CActor::SetInteractive(BOOL bValue)
{
	if(bValue<0){
		if(p_CurrentAction){
			bValue=(p_CurrentAction->p_bDisableUI || (p_CurrentAction->m_parentAction && p_CurrentAction->m_parentAction->p_bDisableUI))?false:true;
		}else{
			bValue=TRUE;
		}
	}
	isInteractive=bValue;
	//scriptMirror.SetValue("Interactive",isInteractive?true:false);
}

int CActor::AI_PauseMovements(BOOL bPauseAI)
{
	//AddDebugScreenLine(toString("Pausing movements on %s - %i",data.p_sName,bPauseAI));
	BOOL bWasPaused=bMovementsIsPaused;
	if(bPauseAI){
		bMovementsIsPaused++;
	}else{
		bMovementsIsPaused--;
	}
	DEBUG_ASSERT(bMovementsIsPaused>=0,toString("Bad bAiIsPaused!!! %i",bMovementsIsPaused));
	if(bMovementsIsPaused)
	{
		StopBodyMotion();
		if(node){
			_v3 pos3=node->getPosition();
			data.p_pos=_v2(pos3.X,pos3.Y);// Запоминаем!
			AI_SyncPosition(1);
		}
	}
	if(!bPauseAI)
	{// При возвращении из паузы, даже если общая пауза пока есть, перемещаем на ближайшую точку
		if(node)
		{
			_v3 pos3=node->getPosition();
			data.p_pos=_v2(pos3.X,pos3.Y);
		}
		// Проверяем позицию с учетом доступности оной из реперной точки!!!
		getLevel()->levelNavigator->EnsureWalkablePos(data.p_pos,FALSE,TRUE);
		AI_SyncPosition(1);
		SetBodyZeroSprite();
	}

	if(bWasPaused && !bMovementsIsPaused)
	{// При возвращении из паузы оживляем путь
		{// Так как это может вызываться в контексте другого экшна обнуляем контекст
			CAction* nowAction=getLevel()->getCurrentAction();
			getLevel()->getCurrentAction()=0;
			ThinkActions();// Подбор нового действия!
			getLevel()->getCurrentAction()=nowAction;
		}
		if(activeNavigationDotLocationName.GetLength())
		{// Перестраиваемся...
			AI_SetMove2TargetPos(activeNavigationDotLocationName);
		}
	}
	scriptMirror.SetValue("pauseMovements",bMovementsIsPaused);
	LOG_DEBUG(toString("|%s: Set pauseMov %i, res %i",data.p_sName,bPauseAI,bMovementsIsPaused));
	//AddDebugScreenLine(toString(toString("%s: Set pauseMov %i, res %i",data.p_sName,bPauseAI,bMovementsIsPaused)));
	return bMovementsIsPaused;
}

BOOL CActor::AI_FinishWalkablePathTo()
{
	if(walkData.activeNavigationMap.size()){
#ifdef _DEBUG
		if(data.p_pos.getDistanceFrom(walkData.activeNavigationMap[0])>3.0f){
			int a=0;
		}
#endif
		data.p_pos=walkData.activeNavigationMap[0];//Обязательно присваиваем... Иначе триггеры в скрипте могут не отработать из-за изинга!!!
		AI_SyncPosition(1);
		walkData.activeNavigationMap.clear();// Можно очистить
	}
	StopBodyMotion();
	walkData.activeNavigationDotTime=0;
	RefreshBodyAccordingDirection(data.p_CurrentBodyDirection,FALSE);
	SetBodyZeroSprite();
	if(data.p_FollowToActor.GetLength())
	{// Сразу думаем куда дальше
		walkData.activeNavigationMap.clear();// Очищаем ДО
		ThinkFollower();
	}
	return TRUE;
}

BOOL CActor::AI_MakeWalkablePathTo(_v2 targetPosition)
{
	CSmartLock cs(&csactiveNavigationMap);
#ifdef _DEBUG
	if(targetPosition==UNDEFINED_2DPOS){if(walkData.activeNavigationMapStepNum!=-1){
		int a=0;
	}}
#endif
	StopBodyMotion();
	walkData.lWalkCount++;
	walkData.activeNavigationDotTime=0;
	walkData.activeNavigationMap.clear();
	if(targetPosition==UNDEFINED_2DPOS){
		activeNavigationDotLocationName="";
		return FALSE;
	}
	return getLevel()->levelNavigator->GetWalkablePath(data.p_pos, targetPosition, walkData);
}

BOOL CActor::SetActionAccordingOrder()
{
	CString sNewAction=nextAction.sActionToSwitchTo;
	BOOL bManual=nextAction.bManual;
	nextAction.sActionToSwitchTo="";// Зачищаем ДО перемены действия - так как оно может инициировать новую перемену

	if(p_CurrentAction){
		p_CurrentAction->OnDetach(this);
		p_CurrentAction=0;
		data.p_CurrentActionName="";
	}
	AI_SetMove2TargetPos(UNDEFINED_2DPOS);// Сбрасываем любые пути прицепленные к персу
	CAction* pNewAction=getLevel()->getActionByName(sNewAction);
	p_CurrentAction=pNewAction;
	data.fActionBeginTime=getIngameTimeAbs();
	if(p_CurrentAction){
		scriptMirror.SetValue("ManualEvent",bManual);
		p_CurrentAction->OnAttach(this);
	}else
	{
		data.p_CurrentActionName="";
		sNewAction="";// Для NO_ACTION
	}
	// История...
	AddAttribute("EventHistory_2",GetAttributeStr("EventHistory_1"));
	AddAttribute("EventHistory_1",GetAttributeStr("EventHistory_0"));
	AddAttribute("EventHistory_0",sNewAction);
	return p_CurrentAction?1:0;
}

BOOL CActor::AI_SwitchToAction(const char* szActionName,BOOL bManual, BOOL bNOW)
{
	if(bNOW)
	{
		AI_SwitchToAction(szActionName,bManual,FALSE);// Простая установка
		// И тутже делаем пуш простой установки!
		CAction* nowAction=getLevel()->getCurrentAction();
		getLevel()->getCurrentAction()=0;
		SetActionAccordingOrder();
		getLevel()->getCurrentAction()=nowAction;
	}else{
		LOG_DEBUG(toString("|%s: Set action %s, bManual %i",data.p_sName,szActionName,bManual));
#ifdef _DEBUG
		dbgActionHistory=toString("%.02f+",getIngameTimeAbs()-data.fActionBeginTime)+dbgActionHistory;
		dbgActionHistory=toString("%s[%s]\n",szActionName,bManual?"m":"a")+dbgActionHistory;
		if(dbgActionHistory.GetLength()>500)
		{
			dbgActionHistory=dbgActionHistory.Left(500)+"...";
		}
#endif
		nextAction.sActionToSwitchTo=szActionName;
		nextAction.bManual=bManual;
	}
	return 1;
}

void CActor::SpawnPersUI()
{
	if(!data.p_Enabled){// Неактивен
		return;
	}
	bNodeSpawned=1;
	//"actors\\bodies\\human.spr"
	if(data.p_Sprite.GetLength())
	{
		node=addBillboard2(data.p_Sprite,getNode("officeFloor"));
	}else{
		node=addEmptyNode(getNode("officeFloor"),DUMMYID);
	}
	node->setID(iActorID);
	posOffset=node->getPosition();// Простой способ сместить картинку относительно орижина
	//posOffset.Z=0.0f;
	AI_SyncPosition(1);
	scriptMirror.SetValue("ActorID",iActorID);
	ShowActorLabels(false);
	// Есть ли тело...
	if(bHasBodyAndCanMove==-1)
	{
		bHasBodyAndCanMove=0;
		CSpriteNode* nodeBody=(CSpriteNode*)getNode(SUBID_BODY,node);
		if(nodeBody)
		{
			bHasBodyAndCanMove=1;
		}
	}
	RefreshBodyAccordingDirection(data.p_CurrentBodyDirection,FALSE);
	// Итемы и анимации
	u32 i=0;
	for(i=0;i<data.p_items.size();i++)
	{
		CActorItem::OnItemSpawnUI(data.p_items[i],this);
	}
	// Продумываем покет - чтобы не мелькал
	ThinkPocket(TRUE);
	for(int i=0;i<getLevel()->data.actors.GetSize();i++){
		CActor* actorOther=getLevel()->data.actors[i];
		if(actorOther && actorOther!=this){
			if(actorOther->data.p_sInPocket_Actor==this->data.p_sName){
				actorOther->ThinkPocket(TRUE);
			}
		}
	}
}

void CActor::ThinkActions()
{
	if(nextAction.sActionToSwitchTo.GetLength()){
		SetActionAccordingOrder();
	}else if(!bMovementsIsPaused)
	{// Если смена задачи не запланирована - ищем подходящую
		BOOL bCanLookNewAction=TRUE;
		if(p_CurrentAction){
			if(p_CurrentAction->p_bProtected){
				bCanLookNewAction=FALSE;
			}
			if(p_CurrentAction->m_parentAction && p_CurrentAction->m_parentAction->p_bProtected > 1){
				bCanLookNewAction=FALSE;
			}
		}
		if(bCanLookNewAction){
			CActionLookupParams lkp;
			lkp.iMinPriority=p_CurrentAction?p_CurrentAction->p_iPriority:0;
			CAction* newAction=CAction::FindBestActionForNow(this,lkp);
			if(newAction && newAction!=p_CurrentAction){
				AI_SwitchToAction(newAction->p_sActionName,FALSE);
			}
		}
	}
	// Продумываем Action
	if(p_CurrentAction){
		p_CurrentAction->OnDuring(this);
	}
}

void CActor::Think(u32 timeMs,BOOL bFastForward)
{
	lastThinkTime=timeMs;
	if(!data.p_Enabled)
	{
		return;
	}
	if(!bFastForward){
		if(!bNodeSpawned){
			SpawnPersUI();
		}
	}
	DEBUG_ASSERT(data.p_fCreationTime>=-0.0001f,"Bad actor creation time!");
	if(!p_Profession || !p_Profession->lNoThink){
		//Порядок вызова:
		//Сначала обдумываем действия. Бывает что актеру назначают путь и действие и обработка действия после движения сбросит путь!
		ThinkActions();
		// Движения
		ThinkMovements(timeMs);
	}
	// Анимации - Думаем всегда! мусор и т.п
	ThinkItems();
	// Карман - даже у тех объектов что не думают! их то и таскают
	ThinkPocket();
}

BOOL CActor::ThinkPocket(BOOL bForceUpdate)
{
	// Провыеряем карманность
	BOOL bInPocket=0;
	CActor* pocketActor=0;
	if(data.p_sInPocket_Actor.GetLength()){
		if(data.p_sInPocket_Actor!=DEF_NONEXISTABLEACTOR){
			pocketActor=getLevel()->getActorByName(data.p_sInPocket_Actor);
		}
		if(!pocketActor){
			sLastInPocketActor=data.p_sInPocket_Actor;
			bInPocket=0;
		}else{
			CActorItem* pocketItem=pocketActor->GetItemByName(data.p_sInPocket_Item);
			if(!pocketItem){
				sLastInPocketActor=data.p_sInPocket_Actor;
				bInPocket=0;
			}else{
				bInPocket=1;
			}
		}
		if(bInPocket){
			DEBUG_ASSERT(data.p_Enabled,"Can not pocketize disabled actor!");
			DEBUG_ASSERT(data.p_FollowToActor.GetLength()==0,"Can pocketize follower!");
			if(GetAttributeLong("__InPocket")==0 || bForceUpdate){
				AddAttribute("__InPocket",long(1));
				AddAttribute("__InPocket_Actor",data.p_sInPocket_Actor);
				AddAttribute("__InPocket_Item",data.p_sInPocket_Item);
			}
			if(node){
				node->setVisible(false);
			}
			if(!p_Profession || p_Profession->lNoPocketPosSync==0){
				data.p_pos=pocketActor->getPosition();
				AI_SyncPosition(2);
			}
		}
		if(!bInPocket){// But WAS in pocket last time!
			data.p_sInPocket_Item="";
			data.p_sInPocket_Actor="";
			if(GetAttributeLong("__InPocket")!=0 || bForceUpdate){
				AddAttribute("__InPocket",long(0));
				AddAttribute("__InPocket_Actor","");
				AddAttribute("__InPocket_Item","");
				getLevel()->levelNavigator->EnsureWalkablePos(data.p_pos);
				// Удаляем все итемы которые тоже должны очищаться
				CStringArray aListToDelete;
				for(u32 i=0;i<data.p_items.size();i++){
					if(data.p_items[i]->sCorrespondingSprFile.Find("__asPocket")!=-1 || (data.p_items[i]->itemDsc && data.p_items[i]->itemDsc->bAsPocket)){
						aListToDelete.Add(data.p_items[i]->sName);
					}
				}
				for(s32 j=0;j<aListToDelete.GetSize();j++){
					DelItem(aListToDelete[j]);
				}
			}
			if(node){
				node->setVisible(true);
				if(data.p_iActorType>0){
					// Позиция!
					// 1) SUBID_DEFDROP
					if(sLastInPocketActor.GetLength()>0){
						CActor* pocketActorLast=getLevel()->getActorByName(sLastInPocketActor);
						if(pocketActorLast && pocketActorLast->node){
							ISceneNode* dropNode=getNode(SUBID_DEFDROP,pocketActorLast->node);
							if(dropNode){
								_v3 newPosition=pocketActorLast->node->getPosition()+dropNode->getPosition();
								newPosition.Z=DEF_ACTOR_Z;
								node->setPosition(newPosition);
							}
						}
					}
					AI_SyncPosition(0);
					// 2) Валидация позиции
					// Проверяем позицию с учетом доступности оной из реперной точки!!!
					getLevel()->levelNavigator->EnsureWalkablePos(data.p_pos,FALSE,TRUE);
					AI_SyncPosition(1);
				}
			}
		}
	}
	// если выкинуло из кармана, то...
	BOOL bWasSwitch=(pocketActor!=lastInPocketActor);
	if(bWasSwitch){
		lastInPocketActor=pocketActor;
		if(getLevel()->dwLevelState>=LEVELSTATE_INGAME && p_Profession && p_Profession->bChangeNavigation){
			// Пересчитываем проходимости
			getLevel()->ScheduleRecalcConnections();//getLevel()->levelNavigator->RecalcConnections();
		}
	}
	return bInPocket;
}

void CActor::ThinkItems()
{
	for(u32 i=0;i<data.p_items.size();i++)
	{
		data.p_items[i]->Think(this);
	}
}

BOOL CActor::PutInPocket(const char* szToActor,const char* szToItem)
{
	if(szToActor && szToActor[0] && szToItem && szToItem[0]){
		sLastInPocketActor="";
		if(GetAttributeLong("__InPocket")!=0 && (data.p_sInPocket_Actor!=szToActor || data.p_sInPocket_Item!=szToItem)){
			data.p_sInPocket_Actor=DEF_NONEXISTABLEACTOR;
			ThinkPocket();// Сбрасываем
		}
		data.p_sInPocket_Actor=szToActor;
		data.p_sInPocket_Item=szToItem;
	}else{
		sLastInPocketActor=data.p_sInPocket_Actor;
		data.p_sInPocket_Actor=DEF_NONEXISTABLEACTOR;
	}
	BOOL bRes=ThinkPocket();
	return bRes;
}

BOOL CActor::MakePocketEmpty()
{
	for(int i=0;i<getLevel()->data.actors.GetSize();i++){
		CActor* actorOther=getLevel()->data.actors[i];
		if(actorOther && actorOther!=this){
			if(actorOther->data.p_sInPocket_Actor==this->data.p_sName){
				DelItem(actorOther->data.p_sInPocket_Item);
				actorOther->PutInPocket(NULL,NULL);
				actorOther->ThinkPocket(TRUE);
			}
			if(data.p_sInPocket_Actor==actorOther->data.p_sName){
				actorOther->DelItem(data.p_sInPocket_Item);
			}
		}
	}
	PutInPocket(NULL,NULL);
	return TRUE;
}

BOOL CActor::AI_SetFollowTo(CActor* targetAuthor)
{
	data.p_FollowToActor=targetAuthor?targetAuthor->data.p_sName:"";
	return FALSE;
}

void CActor::AI_SyncPosition(BOOL bForceUpdate)
{
	if(bForceUpdate){// Подхват перетаскивания?
		if(node){
			node->setPosition(_v3(data.p_pos.X+posOffset.X,data.p_pos.Y+posOffset.Y,DEF_ACTOR_Z+posOffset.Z));
		}
	}else if(node){
		data.p_pos=getPosition();
	}
	if(actorLocationLocator && (data.p_iActorType>0 || bForceUpdate>1)){
		{// Апдейтим... апдейтим всегда, даже в драге!!! даже над стенами и т.п.
			actorLocationLocator->x=data.p_pos.X;
			actorLocationLocator->y=data.p_pos.Y;
		}
	}
}

_v2 CActor::getPosition()
{
	if(node){
		_v3 gp=node->getPosition();
		data.p_pos=_v2(gp.X-posOffset.X,gp.Y-posOffset.Y);// Иначе логическая позиция сдвигается между сейв/лоадом!!!
	}
	return data.p_pos;
}


void CActor::ShowActorLabels(BOOL b, CNodeBasement* onNode)
{
	if(onNode==0){
		onNode=node;
	}
	CSpriteNode* nodeLabels=(CSpriteNode*)getNode(SUBID_LABELS,onNode);
	if(nodeLabels)
	{
		nodeLabels->setVisible(b?true:false);
		if(b)
		{
			CTextNode* tn=(CTextNode*)getNode(SUBID_LB_NAME,nodeLabels);
			if(tn){
				tn->setText(data.p_sNameLocalized);
			}
		}
	}
	{
		CString sHudText="";
		if(sActorStatusTextOverload.GetLength()>0){
			//CString sText=""
			sHudText=_ll(sActorStatusTextOverload,sActorStatusTextOverload,"actors\\actions\\action_names.lng");
		}else{
			sHudText+=data.p_ProfessionName;
			if(b && p_CurrentAction)
			{
				sHudText+=" ";
				sHudText+=p_CurrentAction->p_sNameForHUD;
			}
		}
		BOOL bColor=0;
		getLevel()->AddHint(b?sHudText:"",100.0f,0);
	}

#ifdef _DEBUG
		if(isDebugEnabled()){
			CString sHudText="";
			CTextNode* nodeLabels=(CTextNode*)getNode("debugLabels");
			if(!nodeLabels){
				nodeLabels=addText("???",_v3(0,10,-1),getNode("main_level_interface"));
				nodeLabels->setID("debugLabels");
				//addBillboard3("gui\\fade.spr",_v3(0,0,-0.0001f,nodeLabels);
			}
			if(b>100)
			{
				sHudText+="<font-size:-1><img src=gui\\fade.jpg w=80 h=130 z=0.001 alpha=0.5>\n";
				sHudText+=toString("name=%s, prof=%s, ",data.p_sName,data.p_ProfessionName);
				sHudText+=toString("action=%s\n",data.p_CurrentActionName);
				sHudText+=toString("tloc[%s] MP[%i] isI[%i] ",activeNavigationDotLocationName,bMovementsIsPaused,isInteractive);
				if(data.p_FollowToActor.GetLength()){
					sHudText+=toString("flw[%s]",data.p_FollowToActor);
				}
				sHudText+="\n";
				if(getKbState(VK_SHIFT)<0){
					CString sDbgh=dbgActionHistory;
					//sDbgh=CTextNode::JustifyText(sDbgh,6);
					sHudText+=toString("actionHistory=%s\n",sDbgh);
				}
				if(data.p_items.size()>0){
					sHudText+="items:";
					for(u32 i=0;i<data.p_items.size();i++){
						sHudText+=data.p_items[i]->sName;
						sHudText+=" ";
					}
					sHudText+="\n";
				}
				sHudText+=toString("attributes:%s\n",data.p_Attributes.PackString());
				sHudText+=toString("initLoc[%s] entLoc[%s]\n",data.p_actorLocationInitial,data.p_actorLocationEntrance);
				nodeLabels->setText(sHudText);
				nodeLabels->setVisible(true);
				VisualizeRCTF(actorLocationLocator->Convert2rect(),getDataPath("\\gui\\editor\\square.png"));
				CLocation* locEnt=CLocationManager::getInstance().getLocationByNameSoft(data.p_actorLocationEntrance);
				if(locEnt){
					VisualizeRCTF(locEnt->Convert2rect(),getDataPath("\\gui\\editor\\panels.png"));
				}
			}else{
				nodeLabels->setVisible(false);
			}
			
		}
#endif

	return;
}

void CActor::SetStatusTextOverload(const char* sz)
{
	sActorStatusTextOverload=sz;
}

#define ACTOR_NEAR_DISTANCE	2
BOOL CActor::AI_TuneCrowdBehaviour(_v2& fromp,_v2& point, _rctf& safeZone)
{
	BOOL bStnkes=0;
	_v2 cumulativep=point;
	for(int i=0;i<getLevel()->data.actors.GetSize();i++)
	{
		CActor* ac=getLevel()->data.actors[i];
		if(ac==this || !ac->data.p_Enabled || ac->data.p_iActorType!=1){
			continue;
		}
		f32 distSQ=(ac->getPosition()-cumulativep).getLengthSQ();
		if(distSQ<ACTOR_NEAR_DISTANCE*ACTOR_NEAR_DISTANCE){
			// Вычисялем оффсет!
			int cnt=3;
			f32 mindis=0;
			_v2 pnt(cumulativep);
			while(cnt>0){
				cnt--;
				pnt.X=rndf(safeZone.DOWN_LEFT.X,safeZone.UP_RIGHT.X);
				pnt.Y=rndf(safeZone.DOWN_LEFT.Y,safeZone.UP_RIGHT.Y);
				f32 sqq=(ac->getPosition()-pnt).getLengthSQ();
				if(sqq>mindis){
					cumulativep=pnt;
					mindis=sqq;
					bStnkes++;
				}
			}
		}
	}
	if(bStnkes){
		point=cumulativep;
		return TRUE;
	}
	return FALSE;
}

#define CHECKSTEPS	30
BOOL CActor::AI_EnsureMinDist2Actor(CActor* movableActor, f32 dist,bool forceDistAsNeared)
{
	_v2 thisPosition=getPosition();
	_v2 otherPosition=movableActor->getPosition();
	_v2 betweenVector=otherPosition-thisPosition;
	f32 realDistance=betweenVector.getLength();
	TRACE1(">> AI_EnsureMinDist2Actor init distance: %02f\n",realDistance);
	if(forceDistAsNeared){
		if(realDistance<0.1f){
			// Слишком близко!!!
			betweenVector.X=rndf(-1.0f,1.0f);
			betweenVector.Y=rndf(-1.0f,1.0f);
			// Подгоняем
			realDistance=dist;
		}
		if(realDistance>dist*1.1f)
		{
			// Слишком далеко!!!
			realDistance=dist;
		}
	}
	if(realDistance<dist*0.9f)
	{
		realDistance=dist;
	}
	betweenVector=(betweenVector.normalize())*realDistance;
	int iIterations=CHECKSTEPS;
	float fSingleStep = 2.0f*PI/float(CHECKSTEPS);
	while(iIterations>0)
	{
		_v2 otherPositionTest=thisPosition+betweenVector;
		if(getLevel()->levelNavigator->EnsureWalkablePos(otherPositionTest)==WALKSTATE_IN){
			TRACE2(">> AI_EnsureMinDist2Actor final vec found! %02f:%02f\n",betweenVector.X,betweenVector.Y);
			break;
		}
		// Поворачиваем вектор... ищем свободную площадку
		rotateXYVectorSlow(betweenVector,fSingleStep);
		iIterations--;
	}
	movableActor->SetLocation(thisPosition+betweenVector);
	TRACE3(">> AI_EnsureMinDist2Actor final vec : %02f:%02f, itr=%i\n",betweenVector.X,betweenVector.Y,iIterations);
	return TRUE;
}
