g_AutoSaveTime <- 0;

function actorGetValidMethod(thisActor)
{
	if(thisActor._StepInThreadType>0){
		if(thisActor._StepInThreadMethodAlt.len()>0){
			return thisActor._StepInThreadMethodAlt;
		}
		thisActor._StepInThreadType=0;
	}
	return thisActor._StepInThreadMethod;
}

function runActorStepInThread(thisActor)
{
	if(thisActor._StepInThread==0){
		//core_Alert(StepIn[thisActor._StepInThreadMethod]);
		$ifdebug __DebugActor(thisActor,thisActor.Name+": calling "+actorGetValidMethod(thisActor));
		thisActor._StepInThread=::newthread(StepIn[actorGetValidMethod(thisActor)]);
		thisActor._StepInThread.call(thisActor);
		return;
	}
	if(thisActor._StepInThread.getstatus()=="suspended"){
		//core_Warning(thisActor._StepInThread);
		$ifdebug __DebugActor(thisActor,thisActor.Name+": waking up "+actorGetValidMethod(thisActor));
		thisActor._StepInThread.wakeup();
		return;
	}
	// Если экшн сделает ретюрн, то у потока будет статус idle
	// getstatus -> ("idle","running","suspended")
	// соответсвенно в этих статусах ничего не делаем, просто возвращаемся обратно
	return;
}

function actor_RestartAction(thisActor)
{// Перезапускает текущий ивент с начала. Script.On/off НЕ вызываются
	thisActor._StepInThread=0;
}

function actor_SwitchActionAlt(thisActor)
{// запускает текущий ивент с Script.In.Alt
	thisActor=actor_GetActor(thisActor);
	if(thisActor._StepInThreadType==0){
		if(thisActor._StepInThreadMethodAlt.len()>0){
			thisActor._StepInThreadType=1;
			thisActor._StepInThread=0;
			::suspend();
			return true;
		}
		$ifdebug core_Alert("Error! '"+thisActor.Name+"'/"+thisActor.Action+" does not have Script.In.Alt");
	}
	return false;
}

function event_OnStartGame(param)
{
	core_SetKeyHandlerMode(1);
	core_SetIntervalLocal("main_level_interface",1000,"gui_UpdateHud();");
	RecalcOfficeOnLevelChange();
	if(gui_QuestCheck(8)<2){
		core_EnableNode("techs_button",false);
	}
	if(actor_IsActorExist("Heaps.Barricada")!=false && actor_GetAttributeN("Office","TECH_MUSCLES")==0 ){
		actor_SetAttribute("Heaps.Barricada","__BUSY",1);
	}
}

_saveCameraOverwrite <- {overwrite=false, cameraX=0.0, cameraY=0.0};
function event_OnSaveGame(gameSafe)
{
	local safe=game_MarkSafe();
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
			local officeActor=actor_GetActorPos("Office");
			setFloorPosition({_x=-officeActor._x, _y=-officeActor._y});
		}
	}
	// Грузим дерево технологий
	local lTechs=core_ReadTextFile(core_GetDataPath("\\actors\\quests\\techtree.txt"));
	g_techTree <- core_DeserializeObj(lTechs,true);
	//core_Alert(dump(g_techTree));
}

function setGlobal(name,value)
{
	actor_SetAttribute(actor_GetActor("Office"),"GLOBAL::"+name,value);
}

function getGlobal(name)
{
	return actor_GetAttributeN(actor_GetActor("Office"),"GLOBAL::"+name);
}

function setLevelGlobal(name,value)
{
	actor_SetAttribute(actor_GetActor("Office"),"GLOBAL::LEVEL::"+name,value);
}

function getLevelGlobal(name)
{
	return actor_GetAttributeN(actor_GetActor("Office"),"GLOBAL::LEVEL::"+name);
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
	gui_SoftActorWatch(g_lastActor);
}

function level_StartForLoad()
{
	local lHoursToSkip=office_CalcSkipTime();
	// Вычисляем сколько часов надо пропустить
	if(lHoursToSkip>0.0){
		//game_SetFastForward(lHoursToSkip);
		core_CreateNode("\\gui\\preloader\\clocks.spr","pole");
		core_SetProfileOption("LEVEL_HoursToSkip",lHoursToSkip);//LEVEL_HoursToSkip - особый элемент профиля
	}
}

function level_WaitForLoad()
{
	local hrArrow=core_GetNode("_clock_hour_arrow");
	local mnArrow=core_GetNode("_clock_min_arrow");
	while(game_GetState()<3 || game_IsFastForward()){
		if(hrArrow){
			// Ставим время!
			local timer=game_GetIngameRealTime();
			mnArrow._rotation=-(timer._min)*3.1415*2.0/60;
			hrArrow._rotation=-((timer._hour%12).tofloat()+timer._min.tofloat()/60.0)*3.1415*2.0/12;
			core_SetNode(mnArrow,mnArrow);
			core_SetNode(hrArrow,hrArrow);
		}
		wait(100);
	}
	wait(100);
	//core_Alert("load done");
	core_DeleteNode("_ff_clocks");
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

function gui_UpdateHud()
{
	local persons=actor_GetActors({_byType=1});
	core_SetNodeText("hudPeoplesCount",format("%i",persons.len()));
	local idaes=format("%i",actor_GetAttributeN("Office","ideas"));
	core_SetNodeText("hudIdeasCount",idaes);
	if(g_TechInterfaceOpened){
		core_SetNodeText("hudIdeasCount2",idaes);
	}
}

function game_SetGameSpeed(speed)
{
	game_SetGameSpeed_i(speed);
}

function event_OnTimeOut(safe)
{
	core_Warning("Time is out!");
	level_ResetTimeToNewIssue();
}