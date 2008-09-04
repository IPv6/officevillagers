#include "StdAfx.h"
#include "level.h"
#include "actor.h"
#include "actorItem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif
BOOL& isLevelLoading();

CActorItemDesc::CActorItemDesc()
{
	bFake=0;
	bAsPocket=0;
	bAutoArrange=0;
	iInSprParent=-1;
	bSaveItemInSaves=1;
	lSkipInFF=0;
};

CActorItem::CActorItem()
{
	itemNode=0;
	lCreationTime=CLevelThinker::getThinker()->getGameTimer();
	itemDsc=0;
	bCheckHideIf=0;
};

CActorItem::~CActorItem()
{
	OnDetach(0);
}

BOOL CActorItem::OnItemAttach(CActor* who)
{
	if(!itemDsc){
		itemDsc=getLevel()->getItemDscByName(sName);
	}
	sqoItem=SquirrelVM::CreateTable();
	sqoItem.SetValue("_spr",sCorrespondingSprFile.GetBuffer(0));
	sqoItem.SetValue("_param",sCustomParam.GetBuffer(0));
	sqoItem.SetValue("_name",sName.GetBuffer(0));
	sqoItem.SetValue("_actorName",who->data.p_sName.GetBuffer(0));
	sqoItem.SetValue("_spawned",long(0));
	if(itemDsc){
		if(sHideWhileItem.GetLength()==0){
			if(itemDsc->sHideWhileItem.GetLength()!=0){
				sHideWhileItem=itemDsc->sHideWhileItem;
			}
		}
		if(itemDsc->sScriptOnAttach.GetLength())
		{
			getGame().scripter.CallPrecompiledMethod(itemDsc->sScriptOnAttach,sqoItem);
		}
	}
	bCheckHideIf=TRUE;
	CActorItem::SpawnItemUI(this,who);
	return TRUE;
}

DWORD WINAPI OnItemDel(LPVOID p1, LPVOID p2, CNodeBasement* pDeleteNode)
{
	if(isLevelLoading()){
		return 0;
	}
	CLevel* lvl=getLevel();
	if(getGame().isPaused()>=PAUSELEVEL_EXIT || !lvl || lvl->dwLevelState<=LEVELSTATE_BEFORGM){
		// Игра выключется... или перезагрузка уровня
		return 0;
	}
	CActorItem* item=(CActorItem*)p1;
	if(item){
		item->itemNode=0;
	}
	return 0;
}

class CActorItemWatcher:
	public irr::IReferenceCounted
{
public:
	CString who;
	CString item;
	CActorItemWatcher(const CString& _who, const CString& _item)
	{
		who=_who;
		item=_item;
	}
	~CActorItemWatcher()
	{
		CLevel* lvl=getLevel();
		if(getGame().isPaused()>=PAUSELEVEL_EXIT || !lvl || lvl->dwLevelState<=LEVELSTATE_BEFORGM){
			// Игра выключется... или перезагрузка уровня
			return;
		}
		CActor* actor=getLevel()->getActorByName(who);
		if(actor){
			for(u32 i=0;i<actor->data.p_items.size();i++)
			{
				if(actor->data.p_items[i] && actor->data.p_items[i]->sName==item){
					OnItemDel((LPVOID)actor->data.p_items[i],0,0);
				}
			}
		}
	}
};

BOOL CActorItem::SpawnItemUI(CActorItem* item, CActor* who)
{
	if(!who->bNodeSpawned || getLevel()->dwFastForwardLeft){
		return FALSE;
	}
	if(item && item->itemNode){// уже
		return TRUE;
	}
	//sCorrespondingSprFile не вносим в getDataPath, так как все приводится в нижний регистр там!
	CString sSprReal=item->sCorrespondingSprFile;
	if(item->sCorrespondingSprFile[0]!='\\'){
		sSprReal=getDataPath("\\actors\\items\\")+sSprReal;
	}else{
		sSprReal=getDataPath(sSprReal);
	}
	CSpriteNode* itemParent=NULL;
	long iInSprParent=item->itemDsc?item->itemDsc->iInSprParent:-1;
	_v3 itemPos;
	if(iInSprParent==0){
		itemParent=(CSpriteNode *)who->node->getParent();
		itemPos=who->node->getPosition();
	}else
	{
		if(iInSprParent<0){
			iInSprParent=SUBID_ITEMATTACH;
		}
		if(iInSprParent){
			itemParent=(CSpriteNode*)getNode(iInSprParent,who->node);
		}
		if(!itemParent){
			itemParent=who->node;
		}
	}
	CSpriteNode* res=addBillboard3(sSprReal,itemPos,itemParent);
	res->setAttribL("ACTORID",who->iActorID);
	BOOL bAutoArrange=item->itemDsc?item->itemDsc->bAutoArrange:0;
	if(res->getAttribL("Item_AutoArrange")!=0){
		bAutoArrange=1;
	}
	res->AttachToTimer(1);
	if(item){
		item->itemNode=res;
	}
	{// Связь ноды и итема
		res->setOwner(new CActorItemWatcher(who->data.p_sName,item->sName));
		/*
		// Если на рутовом висит самоудаление... запоминаем
		CSelfdestruction* sfa=(CSelfdestruction*)res->getAnimatorByName(ANIMNAME_CSelfdestruction,FALSE);
		if(sfa){
		sfa->setOnDestroyCallback(OnItemDel,item,0,FALSE);
		}
		*/
	}
	// Автоарранжим другие если надо
	if(bAutoArrange){
		// Сдвигаем такие же с темже именем вверх
		for(u32 i=0;i<who->data.p_items.size();i++){
			CActorItem* ca=who->data.p_items[i];
			if(ca->sName==item->sName && ca->itemNode){
				_v3 pos=ca->itemNode->getPosition();
				pos.Y+=1.0f;
				ca->itemNode->setPosition(pos);
			}
		}
	}
	if(item->itemDsc && item->itemDsc->sScriptOnSpawn.GetLength())
	{
		CNode2Script ns;
		ns.bCanIdName=1;
		ns.DumpNodeTo(res,item->sqoItem);
		item->sqoItem.SetValue("_spawned",long(1));
		getGame().scripter.CallPrecompiledMethod(item->itemDsc->sScriptOnSpawn,item->sqoItem);
	}
	item->bCheckHideIf=TRUE;// Однократно? для установки видимостей
	item->ThinkHideIf(who);
	return TRUE;
}

BOOL CActorItem::OnDetach(CActor* whoOrNull)
{
	if(itemNode)
	{
		if(itemDsc){
			if(whoOrNull && itemDsc->sScriptOnDetach.GetLength())// При выключении не вызываем
			{
				getGame().scripter.CallPrecompiledMethod(itemDsc->sScriptOnDetach,sqoItem);
			}
		}
		if(getGame().isPaused()<PAUSELEVEL_EXIT){
			itemNode->setVisible(false);
			removeNode(itemNode);
		}
		itemNode=0;
	}
	return TRUE;
}

BOOL CActorItem::ThinkHideIf(CActor* who)
{
	if(!bCheckHideIf){
		return FALSE;
	}
	bCheckHideIf=FALSE;
	if(!sHideWhileItem.GetLength()){
		return FALSE;
	}
	if(itemNode){
		CActorItem* otherItem=who->GetItemByName(sHideWhileItem);
		if(otherItem){
			itemNode->setVisible(false);
		}else{
			sHideWhileItem="";
			itemNode->setVisible(true);
		}
	}
	return TRUE;
}

BOOL CActorItem::Think(CActor* who)
{
	if(who->bNodeSpawned){
		if(!itemNode){// Итем убился!!!
			who->DelItem(this);
		}
		ThinkHideIf(who);
	}
	return TRUE;
}

void CActorItem::ApplySerialization()
{
	// Восстанавливаем анимацию!!!
	///TODO OnSpawnUI();/Attach
	if(sAttribsAsStr.GetLength()){
		attribs.UnpackStringShort(sAttribsAsStr);
	}
}

void CActorItem::PrepareSerialization()
{
	sAttribsAsStr=attribs.PackStringShort();
}