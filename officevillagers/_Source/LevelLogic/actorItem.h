#ifndef _H_PROTECTOR_LEVELACTORITEM
#define _H_PROTECTOR_LEVELACTORITEM
#include "../engineBindings.h"
#include "navigation.h"

class CActor;

class CActorItemDesc: public StorageNamed::CXmlStruct
{
public:
	CActorItemDesc();
	CString sName;
	CString sDefaultSpr;
	long	bFake;
	long	bAutoArrange;
	long	iInSprParent;
	long	bAsPocket;
	long bSaveItemInSaves;
	CString sHideWhileItem;
	CString sScriptOnAttach;
	CString sScriptOnSpawn;
	CString sScriptOnDetach;
	CString sAutoClearTypes;
	long lSkipInFF;
	MAP2XML_BEGIN("ItemDescription")
		MAP2XML (&sName, "Name")
		
		MAP2XML_DEF (&sDefaultSpr, "DefaultSpr", "")
		MAP2XML_DEF (&bFake, "Fake",0)
		MAP2XML_DEF (&bAutoArrange, "AutoArrange",0)
		MAP2XML_DEF (&iInSprParent, "InSprParent",-1)
		MAP2XML_DEF (&bAsPocket, "AsPocket",0)
		MAP2XML_DEF (&sHideWhileItem, "HideIf", "")
		MAP2XML_DEF (&bSaveItemInSaves, "SaveInSaves", 1)
		MAP2XML_DEF (&sAutoClearTypes, "DeleteOnEvent", "")
		MAP2XML_DEF (&sScriptOnAttach, "ScriptOnAttach", "")
		MAP2XML_DEF (&sScriptOnSpawn, "ScriptOnSpawn", "")
		MAP2XML_DEF (&sScriptOnDetach, "ScriptOnDetach", "")
		MAP2XML_DEF (&lSkipInFF, "SkipInFF", 0)
	MAP2XML_END()
};
typedef core::array<CActorItemDesc*> _array_CItemDscP;

class CActorItem: public StorageNamed::CXmlStruct
{
public:
	BOOL bCheckHideIf;
	CActorItem();
	~CActorItem();
	CString sName;
	CString sCorrespondingSprFile;
	CString sCustomParam;
	CString sHideWhileItem;
	u32	lCreationTime;
	SquirrelObject sqoItem;
	CUniOptionsHolder attribs;
	CString sAttribsAsStr;
	MAP2XML_BEGIN("Item")
		MAP2XML (&sName, "Name")
		MAP2XML (&sCorrespondingSprFile, "ItemSpr")
		MAP2XML_DEF (&sCustomParam, "ItemCustomParam", "")
		MAP2XML_DEF (&sAttribsAsStr, "Params", "")
		MAP2XML_DEF (&sHideWhileItem, "HideIf", "")
	MAP2XML_END()

	CSpriteNode* itemNode;
	CActorItemDesc* itemDsc;
	void ApplySerialization();
	void PrepareSerialization();
	BOOL OnItemAttach(CActor* who);
	BOOL OnDetach(CActor* who);
	BOOL Think(CActor* who);
	BOOL ThinkHideIf(CActor* who);

	static BOOL SpawnItemUI(CActorItem* item, CActor* who);
	BOOL IsSerializableAsArrayItem(StorageNamed::CIrrXmlStorage* storage)
	{
		if(itemDsc && !itemDsc->bSaveItemInSaves){
			return FALSE;
		}
		return TRUE;
	}
};
typedef core::array<CActorItem*> _array_CItemP;


#endif