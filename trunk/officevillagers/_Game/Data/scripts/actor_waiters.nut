function actorWait_GoTo(thisActor,locationName)
{
	if(actor_SendToLocation(thisActor,locationName)!=true){
		$ifdebug {core_Alert(format("Goto failed!!! %s to %s",actor_GetActor(thisActor).Name,locationName.tostring()));game_SetGameSpeed(0);b=b123/b;};
		actor_SwitchToAction(thisActor,"",true);// На прескрипты
		::suspend();
		return false;
	}
	local lStartTime=getTime();
	// Если актер больше не идет в локацию - тоже стоппимся! Это актуально для разговоров - партнер может принудимтельно остановить движение перса
	while(actor_IsMovingToLocation(thisActor,locationName)==true && actor_IsInLocation(thisActor,locationName)==false){
		::suspend();
		if(getTime()-lStartTime>500){
			// Для защиты от застреваний если перс не может в течении двух минут попасть к цели - возвращаем что все, ахтунг! не может
			$ifdebug {core_Alert(format("Goto failed (too long)!!! %s to %s",actor_GetActor(thisActor).Name,locationName.tostring()));game_SetGameSpeed(0);};
			actor_SwitchToAction(thisActor,"",true);// На прескрипты
			::suspend();
			return false;
		}
	}
	return true;
}

// Ждем пока профессия не...
function actorWait_WhileProfNotIn(thisActor,profName)
{
	while(actor_GetActor(thisActor).Profession!=profName){
		::suspend();
	}
}

function actorWait_WaitToActionStart(thisActor,actionName)
{
	local ActorName=thisActor.Name;
	while(actor_GetActor(ActorName).Action != actionName){
		::suspend();
	}
}

function actorWait_SwitchToAction(thisActor,actionName)
{
	actor_SwitchToAction(thisActor,actionName);
	actorWait_WaitToActionStart(thisActor,actionName);
}

// Ждет определенное время игровых секунд (на двукратной скорости - в два раза меньше)
function actorWait_Timeout(timeout)
{
	local curTime=game_GetTickCount();
	while(curTime+timeout*1000 > game_GetTickCount()){
		::suspend();
	}
}