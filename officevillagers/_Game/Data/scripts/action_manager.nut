function eventManager(thisActor,eventsMask)
{
	// Менеджер берет все ивенты подходящие по маске и смотрит те из них что
	// Были неактивны не меньше минуты
	// Имеют меньше максимального числа персов на нем
	// перс пересажывается на него
	local actions=event_GetEvents({ _byName=eventsMask, _randomize=true });
	local i;
	for(i=0;i<actions.len();i++)
	{
		if(game_GetTickCount()-actions[i].LastActivationTime<10*1000){
			// Кто-то недавно пользовался ивентом...
			continue;
		}
		if(actions[i].UniqueUsage>0 && actions[i].ActionUsageCount>=actions[i].UniqueUsage){
			// И так толпа ходит...
			continue;
		}
		// Ивент нам подходит!
		actor_SwitchToAction(thisActor,actions[i].Name);
		return true;
	}
	return false;
}

function oldestPers(a,b)
{
	if(a.CreationTime<b.CreationTime){//Если время создания меньше - перс СТАРШЕ
		return 1;
	}
	return -1;
}

function getOldestPersons()
{
	local i=0;
	local objectsArray=actor_GetActors( {_byType=1, _randomize=true });
	local res = [];
	for(i=0;i<objectsArray.len();i++){
		if(objectsArray[i].Profession != "AUTHOR" && objectsArray[i].Profession != "AUTHOR-EXPERT"
		&& objectsArray[i].Profession != "CREATOR" && objectsArray[i].Profession != "CREATOR-EXPERT"
		&& objectsArray[i].Profession != "FINANCIER" && objectsArray[i].Profession != "FINANCIER-EXPERT"){
			continue;
		}
		if(objectsArray[i].BodyLock>0){// сюжетные персы не нужны
			continue;
		}
		res.append(objectsArray[i]);
	}
	res.sort(oldestPers);
	return res;
}
