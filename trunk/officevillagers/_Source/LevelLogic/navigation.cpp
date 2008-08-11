#include "StdAfx.h"
#include "navigation.h"
#include "actor.h"
#include "level.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

int& CurrentLocManager()
{
	static int iLastUsedNloc=0;
	return iLastUsedNloc;
}

CSpriteNode* VisualizeRCTF(_rctf dot,const char* szSpr, BOOL bAutoDestroy)
{
	_d2 s=dot.getSize();
	_p2 cen=dot.getCenter();
	CString spr=szSpr;
	if(spr.GetLength()==0){
		spr="gui\\editor\\editor_navimap_loc.spr";
	}
	CSpriteNode* bd=addBillboard(spr,s,_v3(cen.X,cen.Y,-0.01f),getNode("officeFloor"));
	if(bAutoDestroy){
		attachAnimator(bd,new CSelfdestruction(2000, 500));
	}
	return bd;
}

void VisualizeLINE(_v2 v1, _v2 v2)
{
	_v2 mid=v1.getInterpolated(v2,0.5f);
	f32 len=(mid-v1).getLength();
	//if(len<10)
	{
		_d2 s(mid.X-v1.X,mid.Y-v1.Y);
		if(fabs(s.Height)<0.3f){
			s.Height=0.3f;
		}
		if(fabs(s.Width)<0.3f){
			s.Width=0.3f;
		}
		CSpriteNode* nd=addBillboard("gui\\editor\\editor_navimap_con.spr",s,_v3(mid.X,mid.Y,-0.01f),getNode("officeFloor"));
	}
	/*
	_v2 dir=(v1-v2);
	f32 len=dir.getLength();
	//_d2 s(len,len);
	_d2 s(1.0f,1.0f);
	CSpriteNode* nd=addBillboard("gui\\editor\\editor_navimap_con.spr",s,_v3(v1.X,v1.Y,-0.01f),getNode("officeFloor"));
	nd->rotateToTarget(_v3(v1.X,v1.Y,-0.01f),_v3(v2.X,v2.Y,-0.01f));*/
}

BOOL CLocation::IsInLocation(const _v2& dot, _rctf* zazors)
{
	if(bScaled){
		_rctf szs(x-w*0.5f,y+h*0.5f,x+w*0.5f,y-h*0.5f);
		if(zazors){
			szs.LowerRightCorner.X+=zazors->LowerRightCorner.X;
			szs.LowerRightCorner.Y+=zazors->LowerRightCorner.Y;
			szs.UpperLeftCorner.X+=zazors->UpperLeftCorner.X;
			szs.UpperLeftCorner.Y+=zazors->UpperLeftCorner.Y;
		}
		if(dot.X>szs.LowerRightCorner.X || dot.X<szs.UpperLeftCorner.X){
			return FALSE;
		}
		if(dot.Y>szs.UpperLeftCorner.Y || dot.Y<szs.LowerRightCorner.Y){
			return FALSE;
		}
	}else{
		return FALSE;
	}
	return TRUE;
}

BOOL CLocation::IsInLocation(CActor* who)
{
	if(!who){
		return FALSE;
	}
	BOOL bRes=IsInLocation(who->data.p_pos);
	/*if(!bRes){
		// Если локация не на walkable-территории - проверяем окрестности!
		int isWalkable=EnsureWalkablePos
	}*/
	return bRes;
}

CLocation* CLocationManager::getLocationByNameSoft(const char* szName)
{
	for(u32 i=0;i<locations.size();i++)
	{
		if(locations[i]->szName==szName)
		{
			return locations[i];
		}
	}
	return NULL;
}

CLocation* CLocationManager::getLocationByName(const char* szName)
{
	CLocation* res=getLocationByNameSoft(szName);
	if(!res){
		ThrowCriticalError(toString("Undeclared location '%s' used!",szName));
	}
	return res;
}

BOOL CLocationManager::getLocationsFromPoint(const _v2& dot, _array_CLocationsP& outSet)
{
	outSet.clear();
	for(u32 i=0;i<locations.size();i++)
	{
		if(locations[i]->IsInLocation(dot))
		{
			outSet.push_back(locations[i]);
		}
	}
	return outSet.size()?TRUE:FALSE;
}

BOOL CLocationManager::Remove(const char* sName)
{
	for(u32 i=0;i<locations.size();i++)
	{
		if(locations[i]->szName==sName){
			if(locations[i]->IsSerializableAsArrayItem(0)){
				bChanged++;
			}
			delete locations[i];
			locations.erase(i);
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CLocationManager::EnsureWalkablePos(_v2& posInQuestion, BOOL bCheck)
{
	MEASURE_THIS;
	if(!autoMoverRules.size()){
		return WALKSTATE_NONE;
	}
	_array_CLocationsP outSet;
	if(getLocationsFromPoint(posInQuestion, outSet)){
		for(UINT i=0;i<autoMoverRules.size();i++){
			for(UINT j=0;j<outSet.size();j++){
				if(patternMatch(outSet[j]->szName,autoMoverRules[i].szNameFrom)){
#ifdef _DEBUG
					CString s1=outSet[j]->szName;
					CString s2=autoMoverRules[i].szNameFrom;
#endif
					if(!bCheck){
						// Нашли куда переслать!
						CLocation* locTo=getLocationByName(autoMoverRules[i].szNameTo);
						posInQuestion.X=locTo->x;
						posInQuestion.Y=locTo->y;
#ifdef _DEBUG
						if(Debug_ShowInfo() && getKbState(VK_SHIFT)<0){
							_rctf rcc;
							CLocation* locFrom=getLocationByName(outSet[j]->szName);
							rcc.UP_RIGHT.X=locFrom->x+locFrom->w*0.5f;
							rcc.UP_RIGHT.Y=locFrom->y+locFrom->h*0.5f;
							rcc.DOWN_LEFT.X=locFrom->x-locFrom->w*0.5f;
							rcc.DOWN_LEFT.Y=locFrom->y-locFrom->h*0.5f;
							VisualizeRCTF(rcc);
							AddDebugScreenLine(toString("Warning: matching autodrop loc %s!",outSet[j]->szName),5000);
						}
#endif
					}
					return WALKSTATE_JUMP;
				}
			}
		}
	}
	return WALKSTATE_NONE;
}

CLocation* CLocationManager::CreateDynLocator(const char* szName, CLocation* copyFrom)
{
	CString sLocName=toString(DYNLOCATOR_PREFIX"::%s",szName);
	CLocation* newOne=getLocationByNameSoft(sLocName);
	if(newOne){
		// Уже есть. такое возможно при подкрузки рекурсивно описаний актеров в нескольких файлах
		return newOne;
	}
	newOne=new CLocation();
	if(copyFrom){
		(*newOne)=(*copyFrom);
	}
	newOne->szName=sLocName;
	newOne->bNotSavable=TRUE;
	newOne->bScaled=1;
	AddLocator(newOne);
	return newOne;
}

// Добавляет УЖЕ ВНОВЬ СОЗДАННЫЙ локатор, не копию!
CLocation* CLocationManager::AddLocator(CLocation* newOne)
{
	while(1){
		if(!Remove(newOne->szName)){
			break;
		}
	}
	locations.push_back(newOne);
	if(newOne->IsSerializableAsArrayItem(0)){
		bChanged++;
	}
	return newOne;
}

BOOL CLocationManager::CheckOffscreen()
{
	for(u32 i=0;i<locations.size();i++)
	{
		CLocation* rr=locations[i];
		if(rr->bScaled)
		{
			continue;
		}
		DEBUG_ASSERT((rr->x)<1.0f,"Offscreenloc! "+rr->szName);
		DEBUG_ASSERT((rr->x)>-1.0f,"Offscreenloc! "+rr->szName);
		DEBUG_ASSERT((rr->y)<1.0f,"Offscreenloc! "+rr->szName);
		DEBUG_ASSERT((rr->y)>-1.0f,"Offscreenloc! "+rr->szName);
		//DEBUG_ASSERT((rr->x+rr->w*0.5f)<1.0f,"Offscreenloc! "+rr->szName);
		//DEBUG_ASSERT((rr->x-rr->w*0.5f)>-1.0f,"Offscreenloc! "+rr->szName);
		//DEBUG_ASSERT((rr->y+rr->h*0.5f)<1.0f,"Offscreenloc! "+rr->szName);
		//DEBUG_ASSERT((rr->y-rr->h*0.5f)>-1.0f,"Offscreenloc! "+rr->szName);
	}
	return 0;
}

BOOL CLocationManager::ScaleWith(_d2 navidotsScale)
{
	navidotsScale.Width*=0.5f;
	navidotsScale.Height*=0.5f;
	lastUsedScale=navidotsScale;
	for(u32 i=0;i<locations.size();i++)
	{
		CLocation* rr=locations[i];
		if(rr->bScaled)
		{
			continue;
		}
		rr->bScaled=1;
		rr->x*=navidotsScale.Width;
		rr->y*=navidotsScale.Height;
		rr->w*=navidotsScale.Width;
		rr->h*=navidotsScale.Height;
	}
	return TRUE;
}

BOOL CLocationManager::Load(CString sFrom)
{
#ifdef _DEBUG
	_flushall();
#endif
	ClearAll();
	sOriginalFile=getDataPath(sFrom);
	StorageNamed::CIrrXmlStorage storage(sOriginalFile,true);
	DoSerialization(&storage);
	bChanged=0;
	return 1;
}

BOOL CLocationManager::Save()
{
	if(!bChanged){
		return 0;
	}
	_d2 navidotsScale=lastUsedScale;
	for(u32 i=0;i<locations.size();i++)
	{
		CLocation* rr=locations[i];
		if(!rr->bScaled)
		{
			continue;
		}
		rr->bScaled=0;
		rr->x/=navidotsScale.Width;
		rr->y/=navidotsScale.Width;
		rr->w/=navidotsScale.Height;
		rr->h/=navidotsScale.Height;
	}
	StorageNamed::CIrrXmlStorage storage(sOriginalFile,false);
	DoSerialization(&storage);
	bChanged=0;
	return 1;
}

CString getCurrentLocationPrefix()
{
	CAction* curAction=getLevel()->getCurrentAction();
	CString sPrefix;
	if(curAction){
		if(curAction->m_parentAction){
			curAction=curAction->m_parentAction;
		}
		sPrefix=curAction->p_sActionName;
	}
	return sPrefix;
}

CLocation* getLocationPrefixed(CString sByName)
{
	CLocation* loc=0;
	if(sByName.Find("::")==-1){
		CString sPrefix=getCurrentLocationPrefix();
		if(sPrefix.GetLength()){
			loc=CLocationManager::getInstance().getLocationByNameSoft(sPrefix+"::"+sByName);
		}
	}
	if(!loc)
	{
		loc=CLocationManager::getInstance().getLocationByName(sByName);
	}
	return loc;
}
