function actor_Unsit_clear(thisActor)
{
	if(actor_Unsit(thisActor)){
		actor_SwitchToAction(thisActor,"");
	}
}

function actor_SendToLocation(thisActor,locname)
{
	thisActor = actor_GetActor(thisActor);
	// Проверяем, если перс сидит - его надо поднять
	actor_Unsit(thisActor);
	// посылаем перса по адресу...
	return actor_SendToLocation_i(thisActor,locname);
}

function actor_SetActorPos(thisActor,locname)
{
	thisActor = actor_GetActor(thisActor);
	// Проверяем, если перс сидит - его надо поднять
	actor_Unsit(thisActor);
	// посылаем перса по адресу...
	return actor_SetActorPos_i(thisActor,locname);
}

function getPersSitSprite(thisActor,forTable)
{
	local res="";
	thisActor=actor_GetActor(thisActor);
	local spriteData=actor_GetSpriteData(thisActor);
	local sex="s";
	local body_x=0;
	if(spriteData.Sprite.find("=m")>0){
		sex="m";
		body_x=1;
	}
	local isW=spriteData.Sprite.find("=w");
	if(isW>0){
		sex="w";
		body_x=2;
	}
	local body_y=spriteData.Body;
	local head_x=3;
	local head_y=spriteData.Head;
	local sign="";
	if(forTable.Name.find("_R")>0)
	{
		sign="-";
		head_x=0;
	}
	if(forTable.Name.find("TABLE.AGATAMAIN")!=null)
	{// Noie aaaou
		head_x=3;
		if(forTable.Name.find("_2")!=null || forTable.Name.find("_4")!=null || forTable.Name.find("_6")!=null)
		{
			body_x = body_x+6;
		}else{
			body_x = body_x+3;
		}
		sign="-";
	}
	res = format("\\actors\\persons\\human_sit.spr?hsex=%s&body_x=%i&body_y=%i&head_x=%i&head_y=%i&xhead=%s&sign=%s",sex,body_x,body_y,head_x,head_y,"0.0",sign);
	//core_Alert(res);
	return res;
}
//====================================================================================================================================================
// Метод посадки/использования объектов.
//====================================================================================================================================================
function actor_SitOnChairX(thisActor, tableActor, position, sitOrNot)
{
	if(position>1){
		tableActor=actor_GetActor(tableActor);
		tableActor=format("%s_%i",tableActor.Name,position);
	}
	actor_SitOnChair(thisActor, tableActor, sitOrNot);
}

function actor_SitOnChair(thisActor, tableActor, sitOrNot)
{
	// Так как некоторые вещи объяснять бесполезно...
	thisActor = actor_GetActor(thisActor);
	tableActor = actor_GetActor(tableActor);
	// Меняем текстурки
	if(sitOrNot){
		{// Проверяем не сидит ли кто там уже
			// Актер
			if(actor_IsSitting(thisActor)){
				actor_Unsit_clear(thisActor);
			}
			// Сиденье
			local isSitting2=actor_GetItem(tableActor,"sitting");
			if(isSitting2){
				// Если за столом ктото есть - сбрасываем ивент садящемуся
				actor_SwitchToAction(thisActor,"");
				return;
			}
		}
		actor_AddAttribute(thisActor,"SITTING",tableActor.Name);
		actor_AddAttribute(tableActor,"SITTING",thisActor.Name);
		local persSitSprite=getPersSitSprite(thisActor,tableActor);
		actor_AddItem(tableActor,"sitting",{_spr=persSitSprite, _param=thisActor.Name});
		//actor_PutIntoPocket(tableActor,thisActor,"sitting");
	}else{
		actor_AddAttribute(thisActor,"SITTING","");
		actor_AddAttribute(tableActor,"SITTING","");
		actor_DelItem(tableActor,"sitting");
	}
}

function actor_IsSitting(thisActor)
{
	local thisActorObj=actor_GetActor(thisActor);
	// Проверяем, если перс сидит - его надо поднять
	local isSitting=actor_GetAttributeS(thisActorObj,"SITTING");
	if(isSitting && isSitting.len()>0){
		return true;
	}
	return false;
}

function actor_IsSittingGetTable(thisActor)
{
	thisActor = actor_GetActor(thisActor);
	// Проверяем, если перс сидит - его надо поднять
	local isSitting=actor_GetAttributeS(thisActor,"SITTING");
	if(isSitting && isSitting.len()>0){
		return isSitting;
	}
	return "";
}

function actor_Unsit(thisActor)
{
	local isSitting=actor_GetAttributeS(thisActor,"SITTING");
	if(isSitting && isSitting.len()>0){
		// Надо поднять
		actor_SitOnChair(thisActor, isSitting, false);
		return true;
	}
	return false;
}

// Ивенты посадки
function event_OnSit(itemNode)
{
	local tableNode=actor_GetActor(itemNode._actorName);
	actor_AddAttribute(tableNode,"__BUSY",1);
	actor_PutIntoPocket(itemNode._actorName,itemNode._param,"sitting");
}

function event_OnSitSpawn(itemNode)
{
	local tableNode=actor_GetActor(itemNode._actorName);
	local sActorSittierName=actor_GetAttributeS(tableNode,"SITTING");
	core_SetNodeAttribute(itemNode,"ACTORID",actor_GetActor(sActorSittierName).ActorID);
	// Получаем подспрайт - свободное сиденье
	local tableChairNodeFree=core_GetNodeSubnode(tableNode.ActorID,${SUBID_SIT});
	if(tableChairNodeFree){
		core_EnableNode(tableChairNodeFree,false);
	}
	local tableChairNodeSit=core_GetNodeSubnode(tableNode.ActorID,${SUBID_SITX});
	if(tableChairNodeSit){
		core_EnableNode(tableChairNodeSit,true);
		// Двигаем в заданную точку
		local sitPers = core_GetNode(itemNode);
		sitPers._x=tableChairNodeSit._x;
		sitPers._y=tableChairNodeSit._y;
		// Кастомные оффсеты
		local lOnSeatY = core_GetNodeAttribute(tableChairNodeSit,"OnSeatY");
		if(lOnSeatY){
			sitPers._y+=lOnSeatY;
		}
		local lOnSeatX = core_GetNodeAttribute(tableChairNodeSit,"OnSeatX");
		if(lOnSeatX){
			sitPers._x+=lOnSeatX;
		}
		core_SetNode(itemNode,sitPers);
	}
}

function event_OnUnSit(itemNode)
{
	if(itemNode){
		local tableNode=actor_GetActor(itemNode._actorName);
		actor_AddAttribute(tableNode,"__BUSY",0);
		// Получаем подспрайт - свободное сиденье
		local tableChairNodeFree=core_GetNodeSubnode(tableNode.ActorID,${SUBID_SIT});
		if(tableChairNodeFree){
			core_EnableNode(tableChairNodeFree,true);
		}
		local tableChairNodeSit=core_GetNodeSubnode(tableNode.ActorID,${SUBID_SITX});
		if(tableChairNodeSit){
			core_EnableNode(tableChairNodeSit,false);
		}
	}
}
