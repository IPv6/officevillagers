#include "StdAfx.h"
#include "animators.h"
#include "../LevelLogic/Actor.h"
#include "../LevelLogic/Level.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

CSpriteConnector::CSpriteConnector(s32 s1,s32 s2)
{
	fr=s1;
	to=s2;
}

void CSpriteConnector::externInit(CNodeBasement* node)
{
}

void CSpriteConnector::stepAnimation(CNodeBasement* node, u32 timeMs)
{
	CSpriteNode* nd=(CSpriteNode*)node;
	CSpriteNode* nfr=(CSpriteNode*)getNode(fr);
	CSpriteNode* nto=(CSpriteNode*)getNode(to);
	if(!nfr || !nto)
	{
		removeNode(node);
		removeThisAnimator();
		return;
	}
	if(!nfr->isVisible() || !nto->isVisible())
	{
		nd->setInvisible(1);
		return;
	}
	nd->setInvisible(0);
	_v3 nor=nto->getPosition()-nfr->getPosition();
	nd->setXYRotation(getXYRotationFromDirection(nor,-45));
	_d2 thisaSz;
	thisaSz.Width=nor.getLength()*0.5f;
	thisaSz.Height=thisaSz.Width;
	nd->setSize(thisaSz);
	nd->setPosition(nfr->getPosition()+nor/2.0f);//+_v3(thisaSz.Width*0.5f,thisaSz.Height*0.5f,0.0f)
}

CSpriteBodySync::CSpriteBodySync(_array_i& _pather)
{
	pather=_pather;
	parentActor=0;
	actorBody=0;
}

void CSpriteBodySync::externInit(CNodeBasement* node)
{
	int iActorID=node->getAttribL("ACTORID");
	parentActor=getLevel()->getActorByID(iActorID);
	if(parentActor && parentActor->bNodeSpawned){
		originalOffset=node->getPosition();
		actorBody=(CSpriteNode*)getNode(SUBID_BODY,parentActor->node);
		DEBUG_ASSERT(pather.size()==(int)actorBody->atlas_xtotal*2,"body sync pather incomplete");
	}
}

void CSpriteBodySync::stepAnimation(CNodeBasement* node, u32 timeMs)
{
	if(!actorBody){
		externInit(node);// Еще раз...
		if(!actorBody){
			return;
		}
	}
	CSpriteNode* nods=(CSpriteNode*)node;
	_d2 bd_sze=actorBody->getSize();
	_d2 this_sze=nods->getSize();
	if(fsgn(this_sze.Width)!=fsgn(bd_sze.Width))
	{
		this_sze.Width=-this_sze.Width;
		nods->setSize(this_sze);
	}
	int iOffsetNum=(int)actorBody->atlas_x;
	iOffsetNum=iOffsetNum%(pather.size()/2);

	f32 xDir=fsgn(this_sze.Width);
	_v3 resOffset=originalOffset+_v3(pather[iOffsetNum*2]*0.02f,pather[iOffsetNum*2+1]*0.02f,0.0f);
	resOffset.X=fabs(resOffset.X)*xDir;
	node->setPosition(resOffset);
}


CSpriteHeadSync::CSpriteHeadSync()
{
	parentActor=0;
	actorHead=0;
	fPrevAtlasX=0.0f;
}

void CSpriteHeadSync::externInit(CNodeBasement* node)
{
	int iActorID=((CNodeBasement*)node->getParent())->getAttribL("ACTORID");
	parentActor=getLevel()->getActorByID(iActorID);
	if(parentActor && parentActor->bNodeSpawned){
		actorHead=(CSpriteNode*)getNode(SUBID_HEAD,parentActor->node);
		fPrevAtlasX=actorHead->atlas_x;
	}
}

void CSpriteHeadSync::stepAnimation(CNodeBasement* node, u32 timeMs)
{
	if(!actorHead){
		externInit(node);// Еще раз...
		if(!actorHead){
			return;
		}
	}
	if(fPrevAtlasX!=actorHead->atlas_x){// Меняем только с того момента как основной перс покрутил головой!
		CSpriteNode* nods=(CSpriteNode*)node;
		nods->SetAtlasSprite(actorHead->atlas_x,nods->atlas_y,nods->atlas_xtotal,nods->atlas_ytotal);
	}
}


CFontEffectTyper::CFontEffectTyper()
{
	isAllOver=0;
	iCurPos=0;
	iTime=getTick();
	iMaxChar=0;
}

#define CHARS_TRAIL	10
BOOL CFontEffectTyper::DoCharPerturbation()
{
	if(bPrepareLine || isAllOver){
		if(isAllOver && sAutoMarker.GetLength()>0){
			CNodeBasement* ndMarker=getNode(sAutoMarker);
			ndMarker->setAlpha(1.0f);
		}
		if(isAllOver){
			// Сбрасываем резерв инпута
			//CSimpleAnykeyManager::Instance().ReserveInputTarget(1,0);
			((CSpriteNode*)callerNode->getParent())->setAttrib("stopEffect",(long)1);
		}
		return 1;// Строки не трогаем...
	}
	// Когда обновляться?
	if(getTick()-iTime>40){
		iTime=getTick();
		iCurPos++;
	}
	int showLen=charNumberGlobal;//+lineNumber*20;
	if(showLen>iCurPos){
		pColor->setAlpha(0);
		if(iMaxChar<iCurPos){
			iMaxChar=iCurPos;
		}
		return 1;
	}
	if(showLen<iCurPos-CHARS_TRAIL){
		if(showLen>=iMaxChar-1){
			// Показали последнюю строчку!
			isAllOver=1;
			DoCharPerturbation();// Обновим
		}
		return 1;
	}
	if(iCurPos>charsLen-CHARS_TRAIL*0.7f){
		isAllOver=1;
		DoCharPerturbation();// Обновим
		return 1;
	}
	/*
	CSimpleAnykeyManager::Instance().ReserveInputTarget(1,getTick()+1);
	if(CSimpleAnykeyManager::Instance().IsInputOccured(0,1)){
		isAllOver=1;
		DoCharPerturbation();// Обновим
		return 1;
	}
	*/
	if(((CSpriteNode*)callerNode->getParent())->getAttribL("stopEffect")>0){
		isAllOver=1;
		DoCharPerturbation();// Обновим
		return 1;
	}
	// Вычисляем альфу
	f32 fAlpha=1.0f;
	fAlpha=1.0f-f32(showLen-iCurPos)/f32(CHARS_TRAIL);
	pColor->setAlpha(intAlpha(fAlpha));
	//(*pScale)=2.0f-fAlpha;
	return 1;
}
