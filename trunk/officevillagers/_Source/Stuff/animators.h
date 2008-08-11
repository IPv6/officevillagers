#ifndef _H_PROTECTOR_ANIM1
#define _H_PROTECTOR_ANIM1
#include "../engineBindings.h"

class CSpriteConnector: public INodeAnimator
{
public:
	s32 fr;
	s32 to;
	CSpriteConnector(s32 s1,s32 ds2iAlpha);
	void externInit(CNodeBasement* node);
	virtual void stepAnimation(CNodeBasement* node, u32 timeMs);
};

class CFlyAnimatorX: public CFlyAnimator
{
public:
	int iRemoveReason;
	int iDedicatedWalkNumber;
	CFlyAnimatorX(const _v3& startPoint, const _v3& endPoint, u32 timeForWay)
		:CFlyAnimator(startPoint, endPoint, timeForWay)
	{
		iRemoveReason=0;
		iDedicatedWalkNumber=-1;
	}
};

class CActor;
class CSpriteBodySync: public INodeAnimator
{
public:
	CSpriteNode* actorBody;
	_v3 originalOffset;
	CActor* parentActor;
	_array_i pather;
	CSpriteBodySync(_array_i& _pather);
	void externInit(CNodeBasement* node);
	virtual void stepAnimation(CNodeBasement* node, u32 timeMs);
};

class CSpriteHeadSync: public INodeAnimator
{
public:
	float fPrevAtlasX;
	CSpriteNode* actorHead;
	CActor* parentActor;
	CSpriteHeadSync();
	void externInit(CNodeBasement* node);
	virtual void stepAnimation(CNodeBasement* node, u32 timeMs);
};


class CFontEffectTyper: public CCharPerturbator
{
public:
	CString sAutoMarker;
	int iTime;
	int iCurPos;
	int isAllOver;
	int iMaxChar;
	CFontEffectTyper();
	BOOL DoCharPerturbation();
};
#endif