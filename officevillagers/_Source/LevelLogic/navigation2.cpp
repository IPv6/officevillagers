#include "StdAfx.h"
#include "navigation.h"
#include "actor.h"
#include "level.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

_p2i _unpackCoord(void* p)
{
	DWORD d=(DWORD)p;
	_p2i res;
	res.X=HIWORD(d);
	res.Y=LOWORD(d);
	return res;
}

void* _packCoord(int X, int Y)
{
	DWORD d=MAKELONG(Y,X);
	return (LPVOID)d;
}

CLocationManagerNavigatorY::CLocationManagerNavigatorY()
{
	Clear();
	iBasicWallsSize=0;
	pather=new micropather::MicroPather(this);
}

CLocationManagerNavigatorY::~CLocationManagerNavigatorY()
{
	delete pather;
}

void CLocationManagerNavigatorY::Clear()
{
	//memset(map,0,sizeof(map[0][0]*MAP_SIZEX*MAP_SIZEY));
	for(int i=0;i<MAP_SIZEX;i++){
		for(int j=0;j<MAP_SIZEY;j++){
			map[i][j]=0;
		}
	}
}

void CLocationManagerNavigatorY::AddWall(const _rctf& wall,int iValue)
{
#define INCPOS(XXX,YYY)	{_p2i point=V2Map(_v2(XXX,YYY));if(point.X>=0 && point.Y>=0 && point.X<MAP_SIZEX && point.Y<MAP_SIZEY){map[point.X][point.Y]=max(map[point.X][point.Y]+iValue,0);};};
	f32 stepX=mapCellRealSize.Width*0.41f;//max(wall.getSize().Width/10.0f,mapRealSize.Width/f32(MAP_SIZEX))*0.5f;
	f32 stepY=mapCellRealSize.Height*0.41f;//max(wall.getSize().Height/10.0f,lastUsedScale.Height/f32(MAP_SIZEY))*0.5f;
	for(f32 xl=wall.UpperLeftCorner.X+stepX;xl<wall.LowerRightCorner.X;xl+=stepX){
		for(f32 yl=wall.UpperLeftCorner.Y+stepY;yl<wall.LowerRightCorner.Y;yl+=stepY){
			INCPOS(xl,yl);
		}
	}
	INCPOS(wall.UpperLeftCorner.X,wall.UpperLeftCorner.Y);
	INCPOS(wall.UpperLeftCorner.X,wall.LowerRightCorner.Y);
	INCPOS(wall.LowerRightCorner.X,wall.UpperLeftCorner.Y);
	INCPOS(wall.LowerRightCorner.X,wall.LowerRightCorner.Y);
}

void CLocationManagerNavigatorY::RecalcConnections()
{
	MEASURE_THIS;
	Clear();
	int i=0;
	// Сбрасываем все что сверх...
	naviwalls.set_used(iBasicWallsSize);
	_array_rct navifloors;
	// Добавляем динамические
	for(i=0;i<getLevel()->data.actors.GetSize();i++){
		CActor* actor=getLevel()->data.actors[i];
		if(actor->data.p_Enabled && actor->data.p_sInPocket_Actor.GetLength()==0){
			if(actor->data.p_iActorType==0){
				CLocation* loc=actor->actorLocationLocator;
				if(actor->p_Profession && actor->p_Profession->bChangeNavigation && loc){
					_rctf dot=loc->Convert2rect();
					naviwalls.push_back(dot);
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
					if(actor->data.p_actorLocationEntrance.GetLength()>0 && actor->data.p_actorLocationEntrance.Find(HARDENTR_PREFIX)!=-1){
						CLocation* addw=CLocationManager::getInstance().getLocationByNameSoft(actor->data.p_actorLocationEntrance);
						if(addw){
							navifloors.push_back(addw->Convert2rect());
						}
					}
				}
			}
		}
	}
	for(i=0;i<int(naviwalls.size());i++){ 
		_rctf& wall=naviwalls[i];
		AddWall(wall);
	}
	for(i=0;i<int(navifloors.size());i++){ 
		_rctf& floor=navifloors[i];
		AddWall(floor,-1000);
	}
	pather->Reset();
}

void CLocationManagerNavigatorY::InitFrom(CLocationManager* mn)
{
	MEASURE_THIS;
	Clear();
	mapRealSize=mn->lastUsedScale;
	mapCellRealSize.Width=mapRealSize.Width/f32(MAP_SIZEX)*2.0f;
	mapCellRealSize.Height=mapRealSize.Height/f32(MAP_SIZEY)*2.0f;
	UINT i=0;
	_rctf dot;
	for(i=0;i<mn->locations.size();i++)
	{
		CLocation* loc=mn->locations[i];
		if(loc->szName.Find(FLOORWALL_PREFIX)!=-1){
			dot=loc->Convert2rect();
			naviwalls.push_back(dot);
		}
	}
	iBasicWallsSize=naviwalls.size();
}

BOOL CLocationManagerNavigatorY::EnsureWalkablePos(_v2& posInQuestion, BOOL bNoSnapToClosest, BOOL bDoReperPointValidation)
{
	MEASURE_THIS;
	if(CLocationManager::getInstance().EnsureWalkablePos(posInQuestion,bNoSnapToClosest)){
		// Автоскок
		return WALKSTATE_JUMP;
	}
	_p2i mpos=V2Map(posInQuestion);
	BOOL bUsestateVec=FALSE;
	irr::core::array<void*> stateVec;
	if(bDoReperPointValidation)
	{
		static CString sReperPoint=getManifest().Settings.getString("NavigationReperPoint");
		CLocation* addw=CLocationManager::getInstance().getLocationByNameSoft(sReperPoint);
		_v2 startPosition=addw->getPosition();
		_v2 targetPosition=posInQuestion;
		EnsureWalkablePos(targetPosition,FALSE,FALSE);// Без учета реперной точки. GetWalkablePath плюс для тестового режима обязан иметь валидный конец
		CWalkData walkData;
		walkData.bMarkVisitedFloors=10000;
		GetWalkablePath(startPosition,targetPosition, walkData, TRUE);
		pather->StatesInPool( &stateVec );
		bUsestateVec=stateVec.size()?TRUE:FALSE;
#ifdef _DEBUG
		if(getKbState(VK_DIVIDE)<0){
			for(s32 i=0;i<int(stateVec.size());i++){
				_p2i dotmap=_unpackCoord(stateVec[i]);
				_rctf area=Map2Rect(dotmap);
				VisualizeRCTF(area);
			}
		}
#endif
	}
	_p2i closest(-1,-1);
	{// ближайший
		f32 distance=FLT_MAX;
		if(bUsestateVec){
			for(int x=0;x<int(stateVec.size());x++){
				f32 est=LeastCostEstimate(stateVec[x],_packCoord(mpos.X,mpos.Y));
				if(est<distance){
					distance=est;
					_p2i mappos=_unpackCoord(stateVec[x]);
					closest.X=mappos.X;
					closest.Y=mappos.Y;
				}
			}
		}else if(!bNoSnapToClosest){
			for(int x=0;x<MAP_SIZEX;x++){
				for(int y=0;y<MAP_SIZEY;y++){
					if(map[x][y]==0){
						f32 est=LeastCostEstimate(_packCoord(x,y),_packCoord(mpos.X,mpos.Y));
						if(est<distance){
							distance=est;
							closest.X=x;
							closest.Y=y;
						}
					}
				}
			}
		}
	}
	if(closest.X==mpos.X && closest.Y==mpos.Y){
		return WALKSTATE_IN;
	}
	if(!bNoSnapToClosest){
		if(closest.X>=0 && closest.Y>=0){
			_rctf dot=Map2Rect(closest);
			//_v2 closesDot=Map2V();dot.DOWN_LEFT.X=closesDot.X;dot.DOWN_LEFT.Y=closesDot.Y;dot.UP_RIGHT=dot.DOWN_LEFT+_p2(mapCellRealSize.Width,mapCellRealSize.Height);
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
			return WALKSTATE_PROXI;
		}
	}
	if(bNoSnapToClosest==999){
		map[mpos.X][mpos.Y]=0;
		pather->Reset();
	}
	if(map[mpos.X][mpos.Y]==0){
		return WALKSTATE_IN;
	}
	return WALKSTATE_NONE;
}

extern int pathFindingsPerFrm;
extern int g_maxpathFindingsPerFrm;
BOOL CLocationManagerNavigatorY::GetWalkablePath(_v2& startPosition, _v2& targetPosition, CWalkData& walkData, BOOL bJustTest)
{
	MEASURE_THIS;
	BOOL bTargetPointType=EnsureWalkablePos(targetPosition);
	if(bJustTest && bTargetPointType!=WALKSTATE_IN){
		return FALSE;
	}
	EnsureWalkablePos(startPosition);
	_p2i mapStart=V2Map(startPosition);
	_p2i mapStop=V2Map(targetPosition);
	float totalCost=0.0f;
	irr::core::array<void*> outPath;
	pathFindingsPerFrm++;
	int iRes=pather->Solve(_packCoord(mapStart.X,mapStart.Y),_packCoord(mapStop.X,mapStop.Y),&outPath,&totalCost,walkData.bMarkVisitedFloors);
	if(iRes==micropather::MicroPather::NO_SOLUTION){
		return FALSE;
	}
	BOOL bDebugPath=0;
#ifdef _DEBUG
	if(getKbState(VK_DIVIDE)<0){
		bDebugPath=1;
	}
#endif
	if(walkData.bDebugPath){
		bDebugPath=1;
	}
	_array_v2& navigationDots=walkData.activeNavigationMap;
	_array_rctf& navigationDotsZones=walkData.activeNavigationMapZones;

	navigationDots.clear();
	navigationDotsZones.clear();
	navigationDots.push_back(targetPosition);
	navigationDotsZones.push_back(Map2Rect(V2Map(targetPosition)));
	//
	const int trimTails=1;
	for(s32 i=int(outPath.size()-1)-trimTails;i>=0+trimTails;i--){
		_p2i dotmap=_unpackCoord(outPath[i]);
		_rctf area=Map2Rect(dotmap);
		_p2 cnt=area.getCenter();
		navigationDots.push_back(_v2(cnt.X,cnt.Y));
		navigationDotsZones.push_back(area);
		if(bDebugPath){
			VisualizeRCTF(area);
		}
	}
	//
	navigationDots.push_back(startPosition);// Откуда начинаем
	navigationDotsZones.push_back(Map2Rect(V2Map(startPosition)));
	return TRUE;
}

void CLocationManagerNavigatorY::VisualizeWall(_v2& checkPosition)
{
	_array_rct& locs=naviwalls;
	for(u32 i=0;i<locs.size();i++)
	{
		_rctf& rc=locs[i];
		if(!isPointInside(rc,checkPosition)){
			continue;
		}
		CSpriteNode* nd=VisualizeRCTF(locs[i],getDataPath("\\gui\\editor\\square.png"));
		// Ищем локейшн совпадающий с этим
		for(UINT i=0;i<CLocationManager::getInstance().locations.size();i++){
			_rctf rc2=CLocationManager::getInstance().locations[i]->Convert2rect();
			if(rc.LowerRightCorner.X==rc2.LowerRightCorner.X && rc.LowerRightCorner.Y==rc2.LowerRightCorner.Y){
				if(rc.UpperLeftCorner.X==rc2.UpperLeftCorner.X && rc.UpperLeftCorner.Y==rc2.UpperLeftCorner.Y){
					AddDebugScreenLine(CLocationManager::getInstance().locations[i]->szName,5000);
				}
			}
		}
	}
	_p2i pos=V2Map(checkPosition);
	if(map[pos.X][pos.Y]>0){
		CSpriteNode* nd=VisualizeRCTF(Map2Rect(pos),getDataPath("\\gui\\editor\\panels.png"));
	}
}

void CLocationManagerNavigatorY::VisualizeFloor(_v2& checkPosition)
{
	_p2i pos=V2Map(checkPosition);
	if(map[pos.X][pos.Y]==0){
		CSpriteNode* nd=VisualizeRCTF(Map2Rect(pos),getDataPath("\\gui\\editor\\panels.png"));
	}
}


void CLocationManagerNavigatorY::VisualizePath(_v2& startPosition, _v2& targetPosition)
{
	CWalkData walkData;
	walkData.bMarkVisitedFloors=10000;
	irr::core::array<void*> stateVec;
	GetWalkablePath(startPosition,targetPosition, walkData, TRUE);
	pather->StatesInPool( &stateVec );
	for(s32 i=0;i<int(stateVec.size());i++){
		_p2i dotmap=_unpackCoord(stateVec[i]);
		_rctf area=Map2Rect(dotmap);
		VisualizeRCTF(area,0,50000);
	}
}

_v2 CLocationManagerNavigatorY::Map2V(const _p2i& posmap)
{
	_v2 out;
	out.X=f32(posmap.X-MAP_SIZEX/2)*mapCellRealSize.Width;
	out.Y=f32(posmap.Y-MAP_SIZEY/2)*mapCellRealSize.Height;
	return out;
}

_rctf CLocationManagerNavigatorY::Map2Rect(const _p2i& posmap)
{
	_rctf dot;
	_v2 p1=Map2V(posmap);
	dot.UpperLeftCorner.X=p1.X;
	dot.UpperLeftCorner.Y=p1.Y;
	dot.LowerRightCorner.X=p1.X+mapCellRealSize.Width;
	dot.LowerRightCorner.Y=p1.Y+mapCellRealSize.Height;
	return dot;
}

_p2i CLocationManagerNavigatorY::V2Map(const _v2& pos)
{
	_p2i posmap;
	//posmap.X=int(ceil(((pos.X/mapRealSize.Width)/2.0f+0.5f)*f32(MAP_SIZEX)));
	//posmap.Y=int(ceil(((pos.Y/mapRealSize.Height)/2.0f+0.5f)*f32(MAP_SIZEY)));
	posmap.X=int(ceil(pos.X/mapCellRealSize.Width))+MAP_SIZEX/2-1;
	posmap.Y=int(ceil(pos.Y/mapCellRealSize.Height))+MAP_SIZEY/2-1;
	if(posmap.X<0){
		posmap.X=0;
	}
	if(posmap.Y<0){
		posmap.Y=0;
	}
	if(posmap.X>=MAP_SIZEX){
		posmap.X=MAP_SIZEX-1;
	}
	if(posmap.Y>=MAP_SIZEY){
		posmap.Y=MAP_SIZEY-1;
	}
	return posmap;
}

float CLocationManagerNavigatorY::LeastCostEstimate (void *stateStart, void *stateEnd)
{
	_p2i start=_unpackCoord(stateStart);
	_p2i stop=_unpackCoord(stateEnd);
	return f32((start.X-stop.X)*(start.X-stop.X)+(start.Y-stop.Y)*(start.Y-stop.Y));
}

#define COST_CALC(XXX,YYY,WWW) micropather::StateCost st;st.state=_packCoord(dop.X+(XXX),dop.Y+(YYY));if(map[dop.X+(XXX)][dop.Y+(YYY)]==0){st.cost=WWW;adjacent->push_back(st);};
void 	CLocationManagerNavigatorY::AdjacentCost (void *state, irr::core::array< micropather::StateCost > *adjacent)
{
	_p2i dop=_unpackCoord(state);
	adjacent->clear();
	if(map[dop.X][dop.Y]){
		// Ни с чем не связана
		return;
	}
	if(dop.X<MAP_SIZEX-1){
		COST_CALC(1,0,1);
		if(dop.X<MAP_SIZEX-2){
			COST_CALC(2,0,1.3f);
		}
	}
	if(dop.Y<MAP_SIZEY-1){
		COST_CALC(0,1,1);
		if(dop.Y<MAP_SIZEY-2){
			COST_CALC(0,2,1.3f);
		}
	}
	if(dop.X>1){
		COST_CALC(-1,0,1);
		if(dop.X>2){
			COST_CALC(-2,0,1.3f);
		}
	}
	if(dop.Y>1){
		COST_CALC(0,-1,1);
		if(dop.Y>2){
			COST_CALC(0,-2,1.3f);
		}
	}
	//Диагонали
	if(dop.X<MAP_SIZEX-1 && dop.Y<MAP_SIZEY-1){
		COST_CALC(1,1,1);
	}
	if(dop.X>1 && dop.Y<MAP_SIZEY-1){
		COST_CALC(-1,1,1);
	}
	if(dop.X>1 && dop.Y>1){
		COST_CALC(-1,-1,1);
	}
	if(dop.X<MAP_SIZEX-1 && dop.Y>1){
		COST_CALC(1,-1,1);
	}
}
