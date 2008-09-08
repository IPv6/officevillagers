function dropOnTable(thisActor,tableActor)
{
	//core_Warning(format("Drop on table! %s on %s ",thisActor.Name,tableActor.Name));
	//скидывание на авторские столы для авторов и редакторов заставляет их писать статьи
	if(actor_GetActorProfession(thisActor)=="AUTHOR" || actor_GetActorProfession(thisActor)=="AUTHOR-EXPERT"){
		if(tableActor.Name.find("AUTHOR")!=null){
			actor_SitOnChair(thisActor,tableActor,true);
			actor_SwitchToAction(thisActor,"AUTHOR_Script");
			return true;
		}
	}
	
	//скидывание на дебажны стол
	if(tableActor.Name.find("TABLE.AUTHOR1_R")!=null){
		actor_SitOnChair(thisActor,tableActor,true);
		actor_SwitchToAction(thisActor,"Debug_table_Script");
		return true;
	}
	
	//кидаем редакторов на их столы
	if(actor_GetActorProfession(thisActor)=="AUTHOR-EXPERT"){
		if(tableActor.Name.find("EDITOR")!=null){
			actor_SitOnChair(thisActor,tableActor,true);
			actor_SwitchToAction(thisActor,"AUTHOR-EXPERT_Script");
			return true;
		}
	}	
	return false;
}

function dropOnPers(thisActor,persActor)
{
	
	//дебажная катсцена
	if (thisActor.Name=="ACTOR2"){
		actor_SwitchToAction("Office","DEBUGCutscene");
		return;
	}
	
	core_Warning(format("Drop on pers! %s on %s ",thisActor.Name,persActor.Name));
	actor_MakePocketEmpty(thisActor);
	actor_MakePocketEmpty(persActor);
	actor_StopWalking(thisActor,2);
	actor_StopWalking(persActor,2);
	actor_EnsureDistance(persActor,thisActor,2);
	actor_OrientBody(thisActor,persActor);
	actor_OrientBody(persActor,thisActor);
	quest_conditions({ _fromDrop=1, _actorFrom=thisActor, _actorTo=persActor });

	//если стажера кинуть на не стажера...стажер начнет учиться, предвариетльно забрав учителя себе в атрибуты...так же он отмечает, что учительу него есть
	if(actor_GetActorProfession(thisActor)=="STAGER" && actor_GetActorProfession(persActor)!="STAGER"){
		actor_SetAttribute(thisActor,"__ScheduledTeacher",persActor.Name);
		actor_SetAttribute(thisActor,"__MANUALSTUDY",1);
		actor_SwitchToAction(thisActor,"STAGER_Script");
		return true;
	}
	
	return false;
}

function dropOnTrash(thisActor,trashActor)
{
	core_Warning(format("Drop on trash %s!",trashActor.Name));
	quest_conditions({_fromDropTrash=1, _actorFrom=thisActor, _locName=trashActor});
	//кидаем уборщиков в мусор
	if(actor_GetAttributeN(trashActor,"__BUSY")==0
		&& (actor_GetActorProfession(thisActor)=="JANITOR" || actor_GetActorProfession(thisActor)=="JANITOR-EXPERT"))
	{
		// Проверяем что до этого мусора есть путь от середины корридора	
		if(nloc_CheckWalkablePath("FurniDrops::BARR_FROM",trashActor)==false){
			$ifdebug core_Warning("Drop on trash skipped: no path to "+trashActor.Name);
			// Если пути нет - это еще не конец. Отдельные мусора имеют альтернативные подходы для дропа, проверяем может путь есть к ним?
			if(trashActor.Name.find(".allowDown")!=null){
				local lAltLocation = trashActor.Location+"_Down";
				if(nloc_CheckWalkablePath("FurniDrops::BARR_FROM",lAltLocation)!=false){
					// Есть альтернативный подход!
					actor_SetActorPos(thisActor,lAltLocation);
					// Переводим на уборку
					actor_SetAttribute(thisActor,"CurrentTrashName",trashActor.Name);
					actor_SwitchToAction(thisActor,"JANITOR_script");
					return 1;
				}else{
					$ifdebug core_Warning("Drop on trash skipped: no path to alt "+lAltLocation);
				}
			}
			return 0;
		}

		
		// Переводим на уборку
		actor_SetAttribute(thisActor,"CurrentTrashName",trashActor.Name);
		actor_SwitchToAction(thisActor,"JANITOR_script");
		return 1;
	}

	if(actor_GetAttributeN(trashActor,"__BUSY")==0
		&& (actor_GetActorProfession(thisActor)=="CREATOR" || actor_GetActorProfession(thisActor)=="CREATOR-EXPERT"))
	{
		if(trashActor.Name=="Heaps.Barricada"){
		//Кто-то сильно постарался, заваливая проход частями статуи
			actor_SetActionLabel(thisActor,"CREATOR3");
			actor_SwitchToAction(thisActor,"event_15sec_stop");
			return 1;
		}
		if(trashActor.Name=="Heaps.Musor52"){
		//Починить, и можно будет играть
			actor_SetActionLabel(thisActor,"CREATOR4");
			actor_SwitchToAction(thisActor,"event_15sec_stop");
			return 1;
		}
		if(trashActor.Name=="Heaps.Musor57"){
		//Неплохо бы её сдуть и кинуть фонтан, после его починки
			actor_SetActionLabel(thisActor,"CREATOR5");
			actor_SwitchToAction(thisActor,"event_15sec_stop");
			return 1;
		}
		if(trashActor.Name=="Heaps.Musor54"){
		//Низковата. Жаль
			actor_SetActionLabel(thisActor,"CREATOR6");
			actor_SwitchToAction(thisActor,"event_15sec_stop");
			return 1;
		}
		if(trashActor.Name=="Heaps.Musor16"){
		//Жалко, у нас коммунизм
			actor_SetActionLabel(thisActor,"CREATOR7");
			actor_SwitchToAction(thisActor,"event_15sec_stop");
			return 1;
		}
		if(trashActor.Name=="Heaps.Musor23"){
		//Халявная газировка всем желающим
			actor_SetActionLabel(thisActor,"CREATOR8");
			actor_SwitchToAction(thisActor,"event_15sec_stop");
			return 1;
		}
		if(trashActor.Name=="Heaps.Musor27"){
		//Под этой горой может быть что-то полезное
			actor_SetActionLabel(thisActor,"CREATOR9");
			actor_SwitchToAction(thisActor,"event_15sec_stop");
			return 1;
		}
		if(trashActor.Name=="Heaps.Musor59"){
		//Если его починить, у нас тут всегда будет приятный шум воды
			actor_SetActionLabel(thisActor,"CREATOR10");
			actor_SwitchToAction(thisActor,"event_15sec_stop");
			return 1;
		}
		
	}
	
		//если это баррикада, проверяем открыта ли технология
	if(trashActor.Name=="Heaps.Barricada" && actor_GetAttributeN(trashActor,"__BUSY")==1 
	&& (actor_GetActorProfession(thisActor)=="JANITOR" || actor_GetActorProfession(thisActor)=="JANITOR-EXPERT")){
		//если не открыта, то выводим лейбел и не позволяем её убирать
		actor_SetActionLabel(thisActor,"NONE_TECH");
		actor_SwitchToAction(thisActor,"event_15sec_stop");
		return 1;
	}
	
	
	//кидаем персов в коллектиблы
	if(actor_GetAttributeN(trashActor,"__BUSY")==0 
		&& actor_GetActorProfession(thisActor)!="JANITOR" && actor_GetActorProfession(thisActor)!="JANITOR-EXPERT" 
		&& actor_GetAttributeN(trashActor,"IsCollectible")==1)
	{
		// Добавить перетекание в итем
		actor_SetAttribute(thisActor,"CurrentCollectible",trashActor.Name);
		actor_AddItem(thisActor,"collectible_item","collectible_number=0");
		actor_PutIntoPocket(thisActor,trashActor,"collectible_item");
		actor_SwitchToAction(thisActor,"Collectibles_event");
		return 1;
	}
	
	//actor_SwitchToAction(thisActor,"NONE_Script");//Это на уровень выше тепер
	return -1;
}

function dropOnLocation(thisActor,locName)
{	
	//core_Warning(format("Drop on location %s!",locName));
	quest_conditions({ _fromDropLocation=1, _actorFrom=thisActor, _locName=locName});
    return false;
}

function actor_OnDropActor(thisActor, targetPosition)
{
	if("_onDropCustomMethod" in thisActor){
		local customMethod=thisActor._onDropCustomMethod;
		if(customMethod != null){
			customMethod(thisActor, targetPosition);
		}
	}
	//чтобы из туториала не совались
	if(game_IsCutscene()){
		return;
	}	
	local bSwitchToNoneAction = false;
	thisActor=actor_GetActor(thisActor);
	gui_SoftActorWatch(thisActor);
	// вьешательство игрока сбрасывает персу ивент!!! Всегда!
	// важно для сброса разговоров, итемов и т.п.
	// если надо другой ивент то он установится в проверках ниже
	actor_SwitchToAction(thisActor,"");
	// Список объектов по позиции
	local i, objectsArray, locArray, allFurnsArray;
	// Проверяем скидывания в запретные зоны
	locArray=nloc_GetLocations( {_byPosition=targetPosition});
	/*for(i=0;i<locArray.len();i++){
		local locName=locArray[i];
		if(locName.find("FurniDrops::BARR_")!=null && actor_GetActor("Heaps.Barricada")!=false){
			if(!isBarricadaReachable()){
				// Кидаем в точку перед мусорным завалом
				actor_SetActorPos(thisActor,"FurniDrops::BARR_FROM");
				return;
			}
		}
	}*/
	
	allFurnsArray=actor_GetActors( {_byPosition=targetPosition, _byType=0});
	if(allFurnsArray.len()>0){
		// Но сначала двигаем актера в точку дефолтного дропа, если есть${SUBID_DEFDROP}
		local furnActor=allFurnsArray[0];
		local targetActorNode=core_GetNode(furnActor.ActorID);
		local defDrop=core_GetNodeSubnode(targetActorNode,${SUBID_DEFDROP});
		if(defDrop){
			actor_SetActorPos(thisActor,{_raw=true, _x=targetActorNode._x+defDrop._x, _y=targetActorNode._y+defDrop._y});
			actor_OrientBody(thisActor,furnActor);
			//core_Warning("Safe drop found");
		}
	}
	
	// Проверяем столы
	objectsArray=actor_GetActors( {_byPosition=targetPosition, _byName="*TABLE*"});
	for(i=0;i<objectsArray.len();i++){
		local actorName=objectsArray[i];
		if(dropOnTable(thisActor,actor_GetActor(actorName))){
			return;
		}
	}
	// Проверяем других персонажей
	objectsArray=actor_GetActors( {_byPosition=targetPosition, _byPositionLimits={_l=2,_r=2,_u=3,_d=1}, _byType=1});
	for(i=0;i<objectsArray.len();i++){
		local actorName=objectsArray[i];
		//core_Warning(format("checking drop on %s",actorName.Name));
		if(actorName.Name == thisActor.Name){
			// Сами на себя... не бросаем :)
			continue;
		}
		//core_Warning(format("drop %s on %s",actorName.Name,thisActor.Name));
		if(dropOnPers(thisActor,actorName)){
			return;
		}
	}
	// Проверяем мусор
	objectsArray=actor_GetActors( {_byPosition=targetPosition, _byProfession="COLLECTIBLE*"});
	for(i=0;i<objectsArray.len();i++){
		local actorName=objectsArray[i];
		local trashResult=dropOnTrash(thisActor,actor_GetActor(actorName));
		if(trashResult>0){
			return;
		}
		if(trashResult<0){
			bSwitchToNoneAction = true;
		}
	}
	// Checking locations
	for(i=0;i<locArray.len();i++){
		local locName=locArray[i];
		if(dropOnLocation(thisActor,locName)){
			return;
		}
	}
	if(bSwitchToNoneAction){
		actor_SwitchToAction(thisActor,"NONE_Script");
	}
}

function tutorial_fastSintyDrop(thisActor, targetPosition)
{
	local objectsArray=actor_GetActors( {_byPosition=targetPosition, _byName="*TABLE*"});
	if(objectsArray.len()>0){
		local table=actor_GetActor(objectsArray[0]);
		if(table.Name=="TABLE.AUTHOR1_L"){
			actor_SitOnChair("ACTOR.SINTI","TABLE.AUTHOR1_L",true);
		}
	}
}