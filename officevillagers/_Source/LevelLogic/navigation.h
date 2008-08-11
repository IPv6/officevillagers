#ifndef _H_PROTECTOR_NAVIGATION
#define _H_PROTECTOR_NAVIGATION
#include "../engineBindings.h"
#define UNDEFINED_2DPOS		_v2(-1000.1f,-1000.2f)
#define DYNLOCATOR_PREFIX		"DYNAMIC"
#define FLOORWALL_PREFIX		"WALL"

#define WALKSTATE_NONE	0
#define WALKSTATE_JUMP	0
#define WALKSTATE_IN	1
#define WALKSTATE_PROXI	2

class CActor;
struct CWalkData
{
	CWalkData();
	long		lWalkCount;
	_array_v2	activeNavigationMap;
	_array_rctf	activeNavigationMapZones;
	int			activeNavigationMapStepNum;
	u32			activeNavigationDotTimeStart;
	u32			activeNavigationDotTime;
	//Маркирование - используется только в CLocationManagerNavigatorX
	//CLocationManagerNavigatorY - макс глубина просмотра
	u32			bMarkVisitedFloors;
	BOOL		bDebugPath;
};

class CLocation: public StorageNamed::CXmlStruct
{
public:
	CLocation()
	{
		x=y=w=h=0.0f;
		szName="";
		bNotSavable=FALSE;
		bScaled=0;
	}
	~CLocation()
	{
		szName="";
	}
	CLocation& operator =(const CLocation& other)
	{
		x=other.x;
		y=other.y;
		w=other.w;
		h=other.h;
		szName=other.szName;
		bScaled=other.bScaled;
		bNotSavable=other.bNotSavable;
		return *this;
	}
	f32 x;
	f32 y;
	f32 w;
	f32 h;
	CString szName;

	MAP2XML_BEGIN("Location")
		MAP2XML (&szName, "Name")
		MAP2XML (&x, "x")
		MAP2XML (&y, "y")
		MAP2XML (&w, "w")
		MAP2XML (&h, "h")
	MAP2XML_END()
	BOOL IsSerializableAsArrayItem(StorageNamed::CIrrXmlStorage* storage)
	{
		if(bNotSavable){
			return FALSE;
		}
		if(szName.Find("::")==-1){// Только перфиксированные
			return FALSE;
		}
		return TRUE;
	}
	void ApplySerialization()
	{
		x=clamp(x,-1.01f,1.01f);
		y=clamp(y,-1.01f,1.01f);
	}

	BOOL bNotSavable;
	BOOL bScaled;
	BOOL IsInLocation(CActor* who);
	BOOL IsInLocation(const _v2& dot, _rctf* zazors=NULL);
	_v2 getPosition()// По аналогии с актером
	{
		return _v2(x,y);
	}
	_rctf Convert2rect()
	{
		_rctf dot;
		dot.DOWN_LEFT.X=x-w*0.5f;
		dot.DOWN_LEFT.Y=y-h*0.5f;
		dot.UP_RIGHT.X=x+w*0.5f;
		dot.UP_RIGHT.Y=y+h*0.5f;
		return dot;
	}
};

class CLocationConnector
{
public:
	char szNameFrom[128];
	char szNameTo[128];
};

typedef core::array<CLocation*> _array_CLocationsP;
typedef core::array<CLocationConnector> _array_CLocationConnector;
class CLocationManager: public StorageNamed::CXmlStruct
{
public:
	BOOL bChanged;
	_d2 lastUsedScale;
	_array_CLocationsP locations;
	_array_CLocationConnector autoMoverRules;
	MAP2XML_BEGIN("LocationManager")
		MAP2XML (&locations, "Locations")
	MAP2XML_END()
	void PrepareSerialization()
	{
	};
	static CLocationManager& getInstance(BOOL bWhat=0)
	{
		static CLocationManager lm;
		static CLocationManager navmap;
		if(bWhat){
			return navmap;
		}
		return lm;
	}
	CLocationManager()
	{
		bChanged=0;
	}
	~CLocationManager()
	{
		ClearAll();
	}
	void ClearAll()
	{
		if(locations.size()){
			CLocation* loc=0;
			for(u32 i=0;i<locations.size();i++)
			{
				loc=locations[i];
#ifdef _DEBUG
				loc->x=-999.9f;
				loc->y=-999.9f;
				loc->h=-999.9f;
				loc->w=-999.9f;
#endif
				delete loc;
			}
			locations.clear();
		}
	}
	CLocation* getLocationByName(const char* szName);
	CLocation* getLocationByNameSoft(const char* szName);
	BOOL getLocationsFromPoint(const _v2& dot, _array_CLocationsP& outSet);
	BOOL Remove(const char* szNme);
	CLocation* AddLocator(CLocation* newOne);
	BOOL ScaleWith(_d2 scale);
	BOOL CheckOffscreen();
	CLocation* CreateDynLocator(const char* szFullName, CLocation* copyFrom);
	BOOL EnsureWalkablePos(_v2& posInQuestion, BOOL bCheck=FALSE);
	CString sOriginalFile;
	BOOL Load(CString sFrom);
	BOOL Save();

};

CLocation* getLocation(StackHandler& sa, int iParaNum);
CLocation* getLocationPrefixed(CString sByName);
CString getCurrentLocationPrefix();
int& CurrentLocManager();
void VisualizeLINE(_v2 v1, _v2 v2);
CSpriteNode* VisualizeRCTF(_rctf dot,const char* szSpr=0, BOOL bAutoDestroy=TRUE);
void* _packCoord(int X, int Y);
_p2i _unpackCoord(void* p);

class rctFloor: public _rctf
{
public:
	BOOL bWalkable;
	void Init()
	{
		bFixedAgainstWall=0;
		bWalkable=1;// По умолчанию - обязательно!!!
		bStrictCheck=0;
	}
	rctFloor() : _rctf()
	{
		Init();
	}
	rctFloor(f32 x, f32 y, f32 x2, f32 y2): _rctf(x, y, x2, y2)
	{
		Init();
	}
	rctFloor(const _rctf& rtf): _rctf(rtf)
	{
		Init();
	}
	rctFloor(const position2d<f32>& upperLeft, const position2d<f32>& lowerRight): _rctf(upperLeft, lowerRight)
	{
		Init();
	}
	BOOL bFixedAgainstWall;
	BOOL bStrictCheck;
	void MoveOutOfMap()
	{
		UpperLeftCorner.X+=UNDEFINED_2DPOS.X;
		UpperLeftCorner.Y+=UNDEFINED_2DPOS.Y;
		LowerRightCorner.X+=UNDEFINED_2DPOS.X;
		LowerRightCorner.Y+=UNDEFINED_2DPOS.Y;
	}
};
typedef irr::core::array<rctFloor> _array_rctfloor;

class CLocationManagerNavigator
{
public:
	virtual void RecalcConnections()=0;
	virtual void InitFrom(CLocationManager* mn)=0;
	virtual BOOL EnsureWalkablePos(_v2& posInQuestion, BOOL bCheck=FALSE, BOOL bDoReperPointValidation=FALSE)=0;
	virtual BOOL GetWalkablePath(_v2& startPosition, _v2& targetPosition, CWalkData& walkData, BOOL bJustTest=FALSE)=0;
	virtual void VisualizeWall(_v2& checkPosition)=0;
	virtual void VisualizeFloor(_v2& checkPosition)=0;
	virtual void VisualizePath(_v2& startPosition, _v2& stopPosition)=0;
};

class CLocationManagerNavigatorX:public CLocationManagerNavigator
{
public:
	CLocationManagerNavigatorX()
	{
		iBasicWallsSize=0;
	}
	int iBasicWallsSize;
	//_rctf: приямоугольних где с точки зрения игровых координат перепутаны upper и lower...
	_array_v2i		naviconnections;
	_array_rctfloor navifloor;
	_array_rctfloor naviStrickZonesfloor;
	_array_rctfloor navifloorOrigin;
	_array_rct naviwalls;
	void CopyNavs(_array_rctfloor& from, _array_rctfloor& to);
	void InitFrom(CLocationManager* mn);
	void RecalcConnections();
	BOOL EnsureWalkablePos(_v2& posInQuestion, BOOL bCheck=FALSE, BOOL bDoReperPointValidation=FALSE);
	BOOL GetClosestFloorDot(_v2 pos, int* iClosestFloorDot=0, BOOL bDoReperPointValidation=FALSE);
	BOOL IsDotsConnected(int iDot1, int iDot2);
	BOOL GetWalkablePath(_v2& startPosition, _v2& targetPosition, CWalkData& walkData, BOOL bJustTest=FALSE);
	int WallIntersect(const f32& x1, const f32& y1, const f32& x2, const f32& y2);
	void VisualizeWall(_v2& checkPosition);
	void VisualizeFloor(_v2& checkPosition);
};

#define MAP_SIZEX	100
#define MAP_SIZEY	100
#define WALL_COST	1000.0f
#include "mpather/micropather.h"
class CLocationManagerNavigatorY: 
	public CLocationManagerNavigator
	,public micropather::Graph
{
public:
	_d2 mapRealSize;
	_d2 mapCellRealSize;
	_array_rct naviwalls;
	int iBasicWallsSize;
	DWORD map[MAP_SIZEX][MAP_SIZEY];

	CLocationManagerNavigatorY();
	~CLocationManagerNavigatorY();
	void Clear();
	void AddWall(const _rctf& wall);
	void RecalcConnections();
	void InitFrom(CLocationManager* mn);
	BOOL EnsureWalkablePos(_v2& posInQuestion, BOOL bNoSnapToClosest=FALSE, BOOL bDoReperPointValidation=FALSE);
	BOOL GetWalkablePath(_v2& startPosition, _v2& targetPosition, CWalkData& walkData, BOOL bJustTest=FALSE);
	void VisualizeWall(_v2& checkPosition);
	void VisualizeFloor(_v2& checkPosition);
	void VisualizePath(_v2& startPosition, _v2& stopPosition);

	_v2 Map2V(const _p2i& posmap);
	_p2i V2Map(const _v2& pos);
	_rctf Map2Rect(const _p2i& posmap);

	//Micropather
	micropather::MicroPather* pather;
	float 	LeastCostEstimate (void *stateStart, void *stateEnd);
	void 	AdjacentCost (void *state, irr::core::array< micropather::StateCost > *adjacent);
	void 	PrintStateInfo (void *state){};
};

#endif