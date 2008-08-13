function event_OnStartGame(param)
{
	core_SetKeyHandlerMode(1);
}

_saveCameraOverwrite <- {overwrite=false, cameraX=0.0, cameraY=0.0};
function event_OnSaveGame(gameSafe)
{
	local safe=game_GetSafe();
	local officePos = core_GetNode("officeFloor");
	safe.cameraX <- officePos._x;
	safe.cameraY <- officePos._y;
}

function event_OnLoadGame(gameSafe)
{
	actor_DetachCamera();
	if(_saveCameraOverwrite.overwrite)
	{
		_saveCameraOverwrite.overwrite = false;
		setFloorPosition({_x=_saveCameraOverwrite.cameraX, _y=_saveCameraOverwrite.cameraY});
	}else{
		local safe=game_GetSafe();
		if("cameraX" in safe && "cameraY" in safe){
			setFloorPosition({_x=safe.cameraX, _y=safe.cameraY});
		}else{
			// Начальное положение "Камеры"
			local officeActor=actor_GetActorPos(null);
			setFloorPosition({_x=-officeActor._x, _y=-officeActor._y});
		}
	}
	setMainFloorClamp(true);// Баррикада и т.п.
}

function setGlobal(name,value)
{
	actor_SetAttribute(actor_GetActor(),"GLOBAL::"+name,value);
}

function getGlobal(name)
{
	return actor_GetAttributeN(actor_GetActor(),"GLOBAL::"+name);
}

function setLevelGlobal(name,value)
{
	actor_SetAttribute(actor_GetActor(),"GLOBAL::LEVEL::"+name,value);
}

function getLevelGlobal(name)
{
	return actor_GetAttributeN(actor_GetActor(),"GLOBAL::LEVEL::"+name);
}

g_lastActor <- {Name="????Any"};
function gui_JumpToNextActor()
{
	// Ближайший актер
	local curCurPos = core_GetCurrentCursorPosition("officeFloor",true);
	local actors=actor_GetActors({_byType=1, _byPosition=curCurPos, _byPositionLimits={_l=100,_r=100,_u=100,_d=100}});//, _closest=true
	if(actors.len()==0){
		return;
	}
	// Берем следующего за последним
	local i=0, finalI=100000;
	for(i=0;i<actors.len();i++){
		//core_Warning(actors[i].Name);
		if(actors[i].Name==g_lastActor){
			finalI=i+1;
			break;
		}
	}
	if(finalI>=actors.len()){
		finalI=0;
	}
	g_lastActor=actors[finalI].Name;
	gui_CenterCameraOnPers(g_lastActor);
}

function level_WaitForLoad()
{
	while(game_GetState()<3){
		wait(1);
	}
	//core_Alert("load done");
}

function event_BarricadeOff(param)
{
	setMainFloorClamp(true);
}

function __DebugActor(thisActor,debugLine)
{
	actor_SetAttribute(thisActor,"__DEBUG",debugLine);
	//thisActor._StepInThread.__DEBUG <- debugLine;
	core_SetScriptStepDebugLine(thisActor.Name+" said: "+debugLine);
}