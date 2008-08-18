#include "StdAfx.h"
#include "level.h"
#include "actor.h"
#include "actorItem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

BOOL CActor::AddAttribute(const char* szName, const char* value)
{
	data.p_Attributes.setString(szName,value);
	return TRUE;
};

BOOL CActor::AddAttribute(const char* szName, long value)
{
	data.p_Attributes.setLong(szName,value);
	return TRUE;
};

BOOL CActor::AddAttribute(const char* szName, f32 value)
{
	data.p_Attributes.setDouble(szName,value);
	return TRUE;
};

CString CActor::GetAttributeStr(const char* szName)
{
	return data.p_Attributes.getString(szName);
};

long CActor::GetAttributeLong(const char* szName)
{
	return data.p_Attributes.getLong(szName);
};

f32 CActor::GetAttributeF32(const char* szName)
{
	return (f32)data.p_Attributes.getDouble(szName);
};

BOOL CActor::DelAttribute(const char* szName)
{
	return data.p_Attributes.delKey(szName);
};

BOOL CActor::AddItem(const char* szNameRaw,const char* szParamsRaw,const char* szCustomParam)
{
	CString szName=szNameRaw;
	CString sParams=szParamsRaw;
	CActorItem* newOne=new CActorItem();
	newOne->itemDsc=getLevel()->getItemDscByName(szNameRaw);
	BOOL bFake=FALSE;
	if(newOne->itemDsc && newOne->itemDsc->bFake){
		bFake=newOne->itemDsc->bFake;
	}
	newOne->sName=szNameRaw;
	if(!bFake)
	{// Кладем до вызова аттача... так как аттач по имени уже может искать данный итем
		data.p_items.push_back(newOne);
	}
	// Вычисляем имя спрайта...
	CString sSprFile=newOne->sName;
	if(newOne->itemDsc && newOne->itemDsc->sDefaultSpr.GetLength()){
		sSprFile=newOne->itemDsc->sDefaultSpr;
	}else{
		sSprFile=toString("%s.spr",sSprFile);
	}
	if(szParamsRaw && szParamsRaw[0]){
		if(szParamsRaw[0]=='\\'){
			// Уже готовый спрайт - в параметре
			sSprFile=szParamsRaw;
		}else{
			sSprFile+="?";
			sSprFile+=szParamsRaw;
		}
	}
	
	newOne->sCorrespondingSprFile=sSprFile;
	newOne->sCustomParam=szCustomParam;
	newOne->OnItemAttach(this);
	if(bFake){
		newOne->itemNode=0;// Само удалится
		delete newOne;
	}
	for(u32 i=0;i<data.p_items.size();i++){
		data.p_items[i]->bCheckHideIf=TRUE;// Пересчитываем видимости у ВСЕХ итемов актера!
	}
	return 1;
}

BOOL CActor::DelItem(CActorItem* p)
{
	BOOL bRes=FALSE;
	u32 i;
	for(i=0;i<data.p_items.size();i++){
		CActorItem* item=data.p_items[i];
		if(item==p){
			item->OnDetach(this);
			data.p_items.erase(i);
			bRes=TRUE;
			break;
		}
	}
	if(bRes){
		// Обновляем данные отдельно, иначе часть может не обновится
		for(i=0;i<data.p_items.size();i++){
			CActorItem* item=data.p_items[i];
			item->bCheckHideIf=TRUE;// Пересчитываем видимости
		}
		ThinkItems();// Чтобы связанные скрития отработали в этомже кадре
	}
	return bRes;
}

// Важен именно порядоковость в массиве
BOOL CActor::DelItem(const char* szName)
{
	for(u32 i=0;i<data.p_items.size();i++){
		CActorItem* item=data.p_items[i];
		if(item->sName==szName){
			return DelItem(item);
		}
	}
	return FALSE;
}

// Важен именно порядоковость в массиве
CActorItem* CActor::GetItemByName(const char* szName)
{
	for(u32 i=0;i<data.p_items.size();i++){
		if(data.p_items[i]->sName==szName){
			return data.p_items[i];
		}
	}
	return NULL;
}

CActorItem* CActor::GetItemByIndex(int iIndex)
{
	if(iIndex>=0 && iIndex<int(data.p_items.size())){
		return data.p_items[iIndex];
	}
	return NULL;
}

CActorItem* CActor::GetItemByMask(const char* szName)
{
	for(u32 i=0;i<data.p_items.size();i++){
		if(patternMatch(data.p_items[i]->sName,szName)){
			return data.p_items[i];
		}
	}	
	return NULL;
}