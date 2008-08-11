#include "StdAfx.h"
#include "navigation.h"
#include "actor.h"
#include "level.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

//#define FLOORPANEL_PREFIX		"FLOOR"
#define FLOORPANEL_PREFIX		"PANEL"
#define FLOORFIXED_PREFIX		"FIXED"
#define FLOORSMALLTESS_PREFIX	"[BR]"
#define FLOORSTRICTZONE_PREFIX	"STRICTZONE"
#define FLOOR_MAXNEARDIST		5.0f
#define	FLOOR_TESSEL_LIMIT		3.0f
#define	FLOOR_TESSELT_LIMIT		1.0f


void CLocationManagerNavigatorX::InitFrom(CLocationManager* mn)
{
	navifloor.clear();
	naviwalls.clear();
	naviconnections.clear();
	naviStrickZonesfloor.clear();
	UINT i=0;
	_rctf dot;
	for(i=0;i<mn->locations.size();i++)
	{
		CLocation* loc=mn->locations[i];
		dot=loc->Convert2rect();
		if(loc->szName.Find(FLOORSTRICTZONE_PREFIX)!=-1){
			rctFloor dot2(dot.DOWN_LEFT,dot.UP_RIGHT);
			naviStrickZonesfloor.push_back(dot2);
		}
	}
	for(i=0;i<mn->locations.size();i++)
	{
		CLocation* loc=mn->locations[i];
		dot=loc->Convert2rect();
		if(loc->szName.Find(FLOORPANEL_PREFIX)!=-1){
			BOOL bFixed=(loc->szName.Find(FLOORFIXED_PREFIX)!=-1)?(-1):(0);
			// Бъем большой кусок на много мелких
			_d2 nowSize=dot.getSize();
			f32 tightTesselate=FLOOR_TESSEL_LIMIT;
			if(loc->szName.Find(FLOORSMALLTESS_PREFIX)!=-1){
				tightTesselate=FLOOR_TESSELT_LIMIT;
			}
			int iXSize=nowSize.Width<tightTesselate?1:int(nowSize.Width/tightTesselate);
			int iYSize=nowSize.Height<tightTesselate?1:int(nowSize.Height/tightTesselate);
			if(iXSize<1){
				iXSize=1;
			}
			if(iYSize<1){
				iYSize=1;
			}
			nowSize.Width=nowSize.Width/float(iXSize);
			nowSize.Height=nowSize.Height/float(iYSize);
			for(int px=0;px<iXSize;px++){
				for(int py=0;py<iYSize;py++){
					rctFloor dot2(dot.DOWN_LEFT,dot.UP_RIGHT);
					dot2.bFixedAgainstWall=bFixed;
					dot2.DOWN_LEFT.X=dot.DOWN_LEFT.X+nowSize.Width*px;
					dot2.DOWN_LEFT.Y=dot.DOWN_LEFT.Y+nowSize.Height*py;
					dot2.UP_RIGHT.X=dot.DOWN_LEFT.X+nowSize.Width*(px+1);
					dot2.UP_RIGHT.Y=dot.DOWN_LEFT.Y+nowSize.Height*(py+1);
					// Проверяем не в стриктли он зоне
					for(UINT k=0;k<naviStrickZonesfloor.size();k++)
					{
						const rctFloor& wall=naviStrickZonesfloor[k];
						const rctFloor& floor=dot2;
						if(		wall.isPointInside(_p2(floor.LowerRightCorner.X,floor.LowerRightCorner.Y))
							||	wall.isPointInside(_p2(floor.LowerRightCorner.X,floor.UpperLeftCorner.Y))
							||	wall.isPointInside(_p2(floor.UpperLeftCorner.X,floor.UpperLeftCorner.Y))
							||	wall.isPointInside(_p2(floor.UpperLeftCorner.X,floor.LowerRightCorner.Y))
							){
								dot2.bStrictCheck=TRUE;
						}
					}
					navifloor.push_back(dot2);
				}
			}
		}
		if(loc->szName.Find(FLOORWALL_PREFIX)!=-1){
			naviwalls.push_back(dot);
		}
	}
	iBasicWallsSize=naviwalls.size();
	CopyNavs(navifloor,navifloorOrigin);
}


int CLocationManagerNavigatorX::WallIntersect(const f32& x1, const f32& y1, const f32& x2, const f32& y2)
{
	_v2 begin(x1,y1);
	_v2 end(x2,y2);
	BOOL b;
	for(UINT i=0;i<naviwalls.size();i++)
	{
		_rctf& wall=naviwalls[i];
		// Простые случаи
		if(wall.UP_RIGHT.X<x1 && wall.UP_RIGHT.X<x2)
		{
			continue;
		}
		if(wall.UP_RIGHT.Y<y1 && wall.UP_RIGHT.Y<y2)
		{
			continue;
		}
		if(wall.DOWN_LEFT.X>x1 && wall.DOWN_LEFT.X>x2)
		{
			continue;
		}
		if(wall.DOWN_LEFT.Y>y1 && wall.DOWN_LEFT.Y>y2)
		{
			continue;
		}
		// Честное пересечение
		b=isLinesCross(begin, end, _v2(wall.UP_RIGHT.X,wall.UP_RIGHT.Y), _v2(wall.UP_RIGHT.X,wall.DOWN_LEFT.Y));
		if(b){
			return i;
		}
		b=isLinesCross(begin, end, _v2(wall.UP_RIGHT.X,wall.DOWN_LEFT.Y), _v2(wall.DOWN_LEFT.X,wall.DOWN_LEFT.Y));
		if(b){
			return i;
		}
		b=isLinesCross(begin, end, _v2(wall.DOWN_LEFT.X,wall.DOWN_LEFT.Y),_v2(wall.DOWN_LEFT.X,wall.UP_RIGHT.Y));
		if(b){
			return i;
		}
		b=isLinesCross(begin, end, _v2(wall.DOWN_LEFT.X,wall.UP_RIGHT.Y),_v2(wall.UP_RIGHT.X,wall.UP_RIGHT.Y));
		if(b){
			return i;
		}
	}
	return -1;
}

BOOL CLocationManagerNavigatorX::IsDotsConnected(int iDot1, int iDot2)
{
	// Совсем далекие даже и не смотрим!
	if(fabs(navifloor[iDot1].UP_RIGHT.X-navifloor[iDot2].UP_RIGHT.X)>FLOOR_MAXNEARDIST){
		return FALSE;
	}
	if(fabs(navifloor[iDot1].UP_RIGHT.Y-navifloor[iDot2].UP_RIGHT.Y)>FLOOR_MAXNEARDIST){
		return FALSE;
	}
	int iWall=-1;
	if(navifloor[iDot1].bStrictCheck || navifloor[iDot2].bStrictCheck){
		// Проверяем пересекучки со стенами...
		iWall=WallIntersect(navifloor[iDot1].UP_RIGHT.X,navifloor[iDot1].UP_RIGHT.Y,navifloor[iDot2].UP_RIGHT.X,navifloor[iDot2].UP_RIGHT.Y);
		if(iWall!=-1){
			return FALSE;
		}
		iWall=WallIntersect(navifloor[iDot1].DOWN_LEFT.X,navifloor[iDot1].DOWN_LEFT.Y,navifloor[iDot2].DOWN_LEFT.X,navifloor[iDot2].DOWN_LEFT.Y);
		if(iWall!=-1){
			return FALSE;
		}
		iWall=WallIntersect(navifloor[iDot1].UP_RIGHT.X,navifloor[iDot1].DOWN_LEFT.Y,navifloor[iDot2].UP_RIGHT.X,navifloor[iDot2].DOWN_LEFT.Y);
		if(iWall!=-1){
			return FALSE;
		}
		iWall=WallIntersect(navifloor[iDot1].DOWN_LEFT.X,navifloor[iDot1].UP_RIGHT.Y,navifloor[iDot2].DOWN_LEFT.X,navifloor[iDot2].UP_RIGHT.Y);
		if(iWall!=-1){
			return FALSE;
		}
	}else{
		const _p2 c1=navifloor[iDot1].getCenter();
		const _p2 c2=navifloor[iDot2].getCenter();
		iWall=WallIntersect(c1.X,c1.Y,c2.X,c2.Y);
		if(iWall!=-1){
			return FALSE;
		}
	}
	return TRUE;
}

BOOL CLocationManagerNavigatorX::EnsureWalkablePos(_v2& posInQuestion, BOOL bCheck, BOOL bDoReperPointValidation)
{
	MEASURE_THIS;
	if(CLocationManager::getInstance().EnsureWalkablePos(posInQuestion,bCheck)){
		// Автоскок
		return WALKSTATE_JUMP;
	}
	int iClosestDot=0;
	BOOL bIn=GetClosestFloorDot(posInQuestion, &iClosestDot, bDoReperPointValidation);
	if(bIn){
		#ifdef _DEBUG
		if(Debug_ShowInfo()){
			VisualizeRCTF(navifloor[iClosestDot]);
		}
		#endif
		return WALKSTATE_IN;
	}
	if(!bIn && iClosestDot==-1){
#ifdef _DEBUG
		AddDebugScreenLine("Warning! No closest walkable dot found!");
#endif
		if(bDoReperPointValidation){
			EnsureWalkablePos(posInQuestion,FALSE,FALSE);// Без учета проходимости, любую ближайшую!
		}
		return WALKSTATE_NONE;
	}
	if(!bCheck){
	// Снеппим
#ifdef _DEBUG
		if(Debug_ShowInfo()){
			_rctf dot;
			dot.DOWN_LEFT.X=posInQuestion.X-1;
			dot.DOWN_LEFT.Y=posInQuestion.Y-1;
			dot.UP_RIGHT.X=posInQuestion.X+1;
			dot.UP_RIGHT.Y=posInQuestion.Y+1;
			AddDebugScreenLine("Warning! Position moved to closest walkable dot!");
			VisualizeRCTF(dot);
		}
#endif
		_rctf dot=navifloor[iClosestDot];
		if(posInQuestion.X<dot.DOWN_LEFT.X){
			posInQuestion.X=lerp(dot.DOWN_LEFT.X,dot.UP_RIGHT.X,0.3f);
		}
		if(posInQuestion.Y<dot.DOWN_LEFT.Y){
			posInQuestion.Y=lerp(dot.DOWN_LEFT.Y,dot.UP_RIGHT.Y,0.3f);
		}
		if(posInQuestion.X>dot.UP_RIGHT.X){
			posInQuestion.X=lerp(dot.DOWN_LEFT.X,dot.UP_RIGHT.X,0.7f);
		}
		if(posInQuestion.Y>dot.UP_RIGHT.Y){
			posInQuestion.Y=lerp(dot.DOWN_LEFT.Y,dot.UP_RIGHT.Y,0.7f);
		}
#ifdef _DEBUG
		if(Debug_ShowInfo()){
			AddDebugScreenLine("Warning! Position moved to closest walkable dot!");
			VisualizeRCTF(dot);
		}
#endif
	}
	return WALKSTATE_PROXI;
}

BOOL CLocationManagerNavigatorX::GetClosestFloorDot(_v2 posInQuestion, int* iClosestFloorDot, BOOL bDoReperPointValidation)
{
	f32 closest=100000.0f;
	(*iClosestFloorDot)=-1;
	if(bDoReperPointValidation)
	{
		static CString sReperPoint=getManifest().Settings.getString("NavigationReperPoint");
		CLocation* addw=CLocationManager::getInstance().getLocationByNameSoft(sReperPoint);
		_v2 startPosition=addw->getPosition();
		_v2 targetPosition=posInQuestion;
		EnsureWalkablePos(targetPosition,FALSE,FALSE);// Без учета реперной точки. GetWalkablePath плюс для тестового режима обязан иметь валидный конец
		// Обновляем инфу о проходимостях
		CWalkData walkData;
		walkData.bMarkVisitedFloors=1;
		for(u32 i=0;i<navifloor.size();i++)
		{
			navifloor[i].bWalkable = FALSE;
		}
		GetWalkablePath(startPosition, targetPosition, walkData, TRUE);
	}
	for(u32 i=0;i<navifloor.size();i++)
	{
		if(bDoReperPointValidation && !navifloor[i].bWalkable){
			continue;
		}
		_rctf dot=navifloor[i];
		if(isPointInside(dot,posInQuestion))
		{
			(*iClosestFloorDot)=i;
			return TRUE;
		}
		f32 dist=(getCenter(dot)-posInQuestion).getLengthSQ();
		if(dist<closest)
		{
			(*iClosestFloorDot)=i;
			closest=dist;
		}
	}
	return FALSE;
}


void CLocationManagerNavigatorX::RecalcConnections()
{
#ifdef _DEBUG
	BOOL DDD=0;
	if(getKbState(VK_DIVIDE)<0){
		DDD=1;
	}
#endif
	naviconnections.clear();
	{// разбираемся со стенами
		MEASURE_THIS2("RECALC_PREPARE");
		// Сбрасываем все что сверх...
		naviwalls.set_used(iBasicWallsSize);
		CopyNavs(navifloorOrigin,navifloor);
		// Добавляем динамические
		for(int i=0;i<getLevel()->data.actors.GetSize();i++){
			CActor* actor=getLevel()->data.actors[i];
			if(actor->data.p_Enabled && actor->data.p_sInPocket_Actor.GetLength()==0){
				if(actor->data.p_iActorType==0){
					CLocation* loc=actor->actorLocationLocator;
					if(actor->p_Profession && actor->p_Profession->bChangeNavigation && loc){
						_rctf dot=loc->Convert2rect();
						int bFixedAgainstWall=0;
						naviwalls.push_back(dot);
						bFixedAgainstWall=naviwalls.size()-1;
						// Ищем дополнительные локаторы
						if(actor->data.p_actorLocationInitial.GetLength()>0){
							for(int i=1;i<10;i++){
								CString sAdditionalWall=toString("%s[%i]",actor->data.p_actorLocationInitial,i);
								CLocation* addw=CLocationManager::getInstance().getLocationByNameSoft(sAdditionalWall);
								if(addw){
									dot=addw->Convert2rect();
									naviwalls.push_back(dot);
								}
							}
						}
						// Добавляем подводы
						if(actor->data.p_actorLocationEntrance.GetLength()>0){
							CLocation* addw=CLocationManager::getInstance().getLocationByNameSoft(actor->data.p_actorLocationEntrance);
							if(addw){
								rctFloor dotf(addw->Convert2rect());
								dotf.bFixedAgainstWall=bFixedAgainstWall;
								navifloor.push_back(dotf);
							}
						}
						#ifdef _DEBUG
						if(DDD){
							CSpriteNode* nd=VisualizeRCTF(dot);
							addText(getLevel()->data.actors[i]->data.p_sName,_v3_NULL,nd);
						}
						#endif
					}
				}
			}
		}
		{
			MEASURE_THIS2("RECALC_PREPINT");
			// Пересекаем с динамическими...
			for(UINT i=0;i<navifloor.size();i++)
			{
				rctFloor& floor=navifloor[i];
				if(floor.bFixedAgainstWall<0){
					// Эти не трогаем!
					continue;
				}
				if(floor.bFixedAgainstWall>0 && i==floor.bFixedAgainstWall){
					// Эти не трогаем!
					continue;
				}
				for(UINT j=0;j<naviwalls.size();j++)
				{
					_rctf& wall=naviwalls[j];
					if(		wall.isPointInside(_p2(floor.LowerRightCorner.X,floor.LowerRightCorner.Y))
						||	wall.isPointInside(_p2(floor.LowerRightCorner.X,floor.UpperLeftCorner.Y))
						||	wall.isPointInside(_p2(floor.UpperLeftCorner.X,floor.UpperLeftCorner.Y))
						||	wall.isPointInside(_p2(floor.UpperLeftCorner.X,floor.LowerRightCorner.Y))
						){
							//rectMinus(floor,wall,0.1f);
							floor.MoveOutOfMap();
					}
				}
			}
		}
	}
	{
		MEASURE_THIS2("RECALC_CONNECTS");
		naviconnections.reallocate(navifloor.size()*navifloor.size());// Квадрат, с запасом
		for(UINT i=0;i<navifloor.size()-1;i++)
		{
			for(UINT j=i+1;j<navifloor.size();j++)
			{
				if(IsDotsConnected(i,j)){
					_v2i way1(i,j);
					naviconnections.push_back(way1);
					//_v2i way2(j,i);naviconnections.push_back(way2);
#ifdef _DEBUG
					if(DDD){
						_p2 p1=navifloor[i].getCenter();
						_p2 p2=navifloor[j].getCenter();
						VisualizeLINE(_v2(p1.X,p1.Y),_v2(p2.X,p2.Y));
					}
#endif
				}
			}
		}
	}
}

void CLocationManagerNavigatorX::CopyNavs(_array_rctfloor& from, _array_rctfloor& to)
{
	to.clear();
	to.reallocate(from.size());
	for(u32 i=0;i<from.size();i++)
	{
		rctFloor plik(from[i]);
		to.push_back(plik);
	}
}

BOOL CLocationManagerNavigatorX::GetWalkablePath(_v2& startPosition, _v2& targetPosition, CWalkData& walkData, BOOL bJustTest)
{
	BOOL bDebugPath=0;
#ifdef _DEBUG
	if(getKbState(VK_DIVIDE)<0){
		bDebugPath=1;
	}
#endif
	if(walkData.bDebugPath){
		bDebugPath=1;
	}

	u32 i=0,j=0;
	_array_v2& navigationDots=walkData.activeNavigationMap;
	_array_rctf& navigationDotsZones=walkData.activeNavigationMapZones;
	navigationDots.clear();
	navigationDotsZones.clear();

	_array_rctfloor& allUsedDots=navifloor;
	_array_v2i& dot2dot_naviconnections=naviconnections;
	_array_i StepsHolder;
	int beginNodeId=-1;
	int endNodeId=-1;
	int maxStepsBegin=50;
	int maxSteps=maxStepsBegin;
	{
		MEASURE_THIS2("GETPATH_BPHASE");
		BOOL bTargetPointType=EnsureWalkablePos(targetPosition);
		if(bJustTest && bTargetPointType!=WALKSTATE_IN){
			return FALSE;
		}
		EnsureWalkablePos(startPosition);
		for(i=0;i<allUsedDots.size();i++)
		{
			StepsHolder.push_back(0);
			if(isPointInside(allUsedDots[i],startPosition)){
				beginNodeId=i;
			}
			if(isPointInside(allUsedDots[i],targetPosition)){
				endNodeId=i;
			}
		}
		if(beginNodeId==-1 || endNodeId==-1)
		{
			//GfxMessageBox("Одна из точек (начала или конца) не лежит ни в одном круге!!!");
			return FALSE;
		}
		StepsHolder[beginNodeId]=maxSteps;
		while(maxSteps>0)
		{
			BOOL bWaveFrontIncreased=0;
			for(i=0;i<dot2dot_naviconnections.size();i++)
			{
				_v2i& dotcon=dot2dot_naviconnections[i];
				// В одну
				BOOL bIncreaseHappened=0;
				if( StepsHolder[ dotcon.X ]>0 && StepsHolder[ dotcon.Y ]==0)
				{
					StepsHolder[ dotcon.Y ]=StepsHolder[ dotcon.X ]-1;
					bIncreaseHappened=1;
				}
				// В другую (COPYPASTE)
				if( StepsHolder[ dotcon.Y ]>0 && StepsHolder[ dotcon.X ]==0)
				{
					StepsHolder[ dotcon.X ]=StepsHolder[ dotcon.Y ]-1;
					bIncreaseHappened=2;
				}
				if(bIncreaseHappened)
				{
					bWaveFrontIncreased++;
					if(bDebugPath){
						VisualizeRCTF(allUsedDots[bIncreaseHappened==1?dotcon.Y:dotcon.X]);
					}
					if(walkData.bMarkVisitedFloors)
					{
						allUsedDots[dotcon.Y].bWalkable++;
						allUsedDots[dotcon.X].bWalkable++;
					}
				}
			}
			if(!bWaveFrontIncreased){
				maxSteps=-1;
				break;
			}
			int iOnEnd=StepsHolder[ endNodeId ];
			if(iOnEnd>0)
			{
				// нашли!!!
				break;
			}
			maxSteps--;
		}
	}
	if(maxSteps>0){
		if(!bJustTest){
			MEASURE_THIS2("GETPATH_PATHGET");
			// путь есть...
			// В конце...
			DEBUG_ASSERT(navigationDots.size()==0,"Wrong navidsize!");
			DEBUG_ASSERT(navigationDotsZones.size()==0,"Wrong navidzsize!");
			navigationDots.push_back(targetPosition);
			navigationDotsZones.push_back(allUsedDots[endNodeId]);

			int lPathFrom=endNodeId;
			int MaxVal=-1;
			int MaxValIndex=-1;
			_v2 centersOffset;
			while(lPathFrom != beginNodeId)
			{
				MaxVal=-1;
				MaxValIndex=-1;
				for(j=0;j<dot2dot_naviconnections.size();j++)
				{
					if(dot2dot_naviconnections[j].X==lPathFrom && StepsHolder[ dot2dot_naviconnections[j].Y ]>MaxVal)
					{
						MaxVal=StepsHolder[ dot2dot_naviconnections[j].Y ];
						MaxValIndex=dot2dot_naviconnections[j].Y;
					}
					if(dot2dot_naviconnections[j].Y==lPathFrom && StepsHolder[ dot2dot_naviconnections[j].X ]>MaxVal)
					{
						MaxVal=StepsHolder[ dot2dot_naviconnections[j].X ];
						MaxValIndex=dot2dot_naviconnections[j].X;
					}
				}
				if(MaxValIndex>=0)
				{
					if(MaxValIndex!=endNodeId && MaxValIndex!=beginNodeId)
					{// Добавляем (кроме первой и последней!)
						_rctf dot=allUsedDots[MaxValIndex];
						_p2 dotCenter=dot.getCenter();
						_d2 dotSize=dot.getSize();
						_v2 vNewPathDot(dotCenter.X,dotCenter.Y);
						_v2 autoShift=(startPosition+navigationDots[navigationDots.size()-1])/2.0f;
						// Случайное смещение чтобы пути были разные
						centersOffset=autoShift-vNewPathDot;
						f32 offsetLenRandom=rndf(0.1f,0.95f);
						centersOffset=centersOffset.normalize()*(min(dotSize.Width,dotSize.Height)*offsetLenRandom);
						_v2 vNewPathDotFinished=_v2(vNewPathDot.X,vNewPathDot.Y)+_v2(centersOffset.X,centersOffset.Y);
						navigationDots.push_back(vNewPathDotFinished);
						navigationDotsZones.push_back(dot);
					}
					lPathFrom=MaxValIndex;
				}else{
					break;
				}
			}
			navigationDots.push_back(startPosition);// Откуда начинаем
			navigationDotsZones.push_back(allUsedDots[beginNodeId]);
		}
		return TRUE;
	}
	return FALSE;
}

void CLocationManagerNavigatorX::VisualizeFloor(_v2& checkPosition)
{
	_array_rctfloor& locs=navifloor;
	for(u32 i=0;i<locs.size();i++)
	{
		if(!isPointInside(locs[i],checkPosition)){
			continue;
		}
		CSpriteNode* nd=VisualizeRCTF(locs[i]);
	}
}

void CLocationManagerNavigatorX::VisualizeWall(_v2& checkPosition)
{
	_array_rct& locs=naviwalls;
	for(u32 i=0;i<locs.size();i++)
	{
		if(!isPointInside(locs[i],checkPosition)){
			continue;
		}
		CSpriteNode* nd=VisualizeRCTF(locs[i],getDataPath("\\gui\\editor\\square.png"));
	}
}