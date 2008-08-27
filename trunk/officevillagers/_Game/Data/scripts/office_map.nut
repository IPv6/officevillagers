// Глобальные переменные
floorDragInterval <- 0;
floorDragLastPos <- {_x = 0, _y = 0};

floorAttachInterval <- 0;
floorAttachTarget <- "";
floorAttachTargetType <- 0;
floorAttachTargetTime <- 0;
floorAttachTargetTimeFrom <- 0;
floorAttachTargetPosFrom <- {_x = 0, _y = 0};

floorSoftAttachInterval <- 0;

floorClamps <- [{_l=11,_r=-0.6,_u=15.1,_d=16.4},{_l=11,_r=-0.6,_u=15.1,_d=-5.7},{_l=0.6,_r=-0.6,_u=12.4,_d=-5.7}];
floorClampsNum <- 0;

function setMainFloorClamp(bUseMain)
{
	if(bUseMain){
		if(actor_IsActorExist("Heaps.Barricada")){
			floorClampsNum = 0;
		}else{
			floorClampsNum = 1;
		}
	}else{
		floorClampsNum = 2;//Cutscene clamp
	}
	setFloorPosition(core_GetNode("officeFloor"));
}

// Методы
g_DebugCmap <- false;
function clampPosition(object)
{
	local res=false;
	// Левый край
	$ifdebug if(g_DebugCmap) core_Warning(format("wx=%.02f wy=%.02f",object._x,object._y));
	if(object._x > object._w/4+floorClamps[floorClampsNum]._l )
	{
		object._x=object._w/4+floorClamps[floorClampsNum]._l;
		$ifdebug if(g_DebugCmap) core_Warning(format("l=%.02f",floorClamps[floorClampsNum]._l));
		res=true;
	}
	
	if(object._x < -object._w/4+floorClamps[floorClampsNum]._r )
	{
		object._x = -object._w/4+floorClamps[floorClampsNum]._r;
		$ifdebug if(g_DebugCmap) core_Warning(format("r=%.02f",floorClamps[floorClampsNum]._r));
		res=true;
	}

	// Нижний край
	if(object._y > object._h/4+floorClamps[floorClampsNum]._u )
	{
		object._y=object._h/4+floorClamps[floorClampsNum]._u;
		$ifdebug if(g_DebugCmap) core_Warning(format("u=%.02f",floorClamps[floorClampsNum]._u));
		res=true;
	}
	
	if(object._y < -object._h/4+floorClamps[floorClampsNum]._d )
	{
		object._y = -object._h/4+floorClamps[floorClampsNum]._d;
		$ifdebug if(g_DebugCmap) core_Warning(format("d=%.02f",floorClamps[floorClampsNum]._d));
		res=true;
	}
	//if(res){core_Warning(format("clampx=%f clampy=%f",object._x,object._y));}
	$ifdebug if(g_DebugCmap) core_Warning(format("wxO=%.02f wyO=%.02f",object._x,object._y));
	return res;
}

function setFloorPosition(floorPos)
{
	local spritePos = core_GetNode("officeFloor");
	spritePos._x = floorPos._x;
	spritePos._y = floorPos._y;
	local res=clampPosition(spritePos);
	core_SetNode("officeFloor",spritePos);
	return res;
}

function floorDragLoop(param)
{
	local curCurPos = core_GetCurrentCursorPosition("officeFloor");
	if(curCurPos._x != floorDragLastPos._x || curCurPos._y != floorDragLastPos._y)
	{
	
		local spritePos = core_GetNode("officeFloor");
		spritePos._x += (curCurPos._x - floorDragLastPos._x);
		spritePos._y += (curCurPos._y - floorDragLastPos._y);
		clampPosition(spritePos);
		core_SetNode("officeFloor",spritePos);
	}
	floorDragLastPos = curCurPos;
	if(!core_GetIsLeftButtonDown() || floorAttachInterval != 0)
	{
		core_CancelInterval(floorDragInterval);
		floorDragInterval = 0;
	}
}

function officeMapLClick(param)
{
	gui_StopSoftActorWatch();
	if(core_IsDebug() && core_KeyPressed(0x11))//VK_CONTROL
	{
		local spritePos = core_GetNode("officeFloor");
		floorDragLastPos = core_GetLastClickPosition("officeFloor");
		actor_SendToLocation_i( humanDragLastId, floorDragLastPos._x-spritePos._x,floorDragLastPos._y-spritePos._y);
		return;
	}
	if(core_IsDebug() && core_KeyPressed(0x12))//Alt
	{
		local spritePos = core_GetNode("officeFloor");
		floorDragLastPos = core_GetLastClickPosition("officeFloor");
		local actorPos=actor_GetActorPos(humanDragLastId);
		actor_OrientBody( humanDragLastId, (floorDragLastPos._x-spritePos._x)-actorPos._x,(floorDragLastPos._y-spritePos._y)-actorPos._y);
		return;
	}
	if(floorDragInterval==0 && humanDragInterval==0 && floorAttachInterval==0)
	{
		floorDragLastPos = core_GetLastClickPosition("officeFloor");
		floorDragInterval = core_SetIntervalLocal("main_office_level",20,"floorDragLoop",floorDragLastPos);
	}
	return;
}

floorAutoDragInterval <- 0;
floorAutoDragStopAt <- 0;
scheduledFloorOffset <- {_x =0, _y =0};
function floorOffsetLoop(param)
{
	local spritePos = core_GetNode("officeFloor");
	spritePos._x += scheduledFloorOffset._x;
	spritePos._y += scheduledFloorOffset._y;
	if(clampPosition(spritePos))
	{// Сбрасываем...
		floorAutoDragStopAt = 0;
	}
	core_SetNode("officeFloor",spritePos);
	if(humanDragInterval != 0)
	{// Хак чтобы зажатый человечек не дрожал
		humanDragLoop(param);
	}
	if(core_GetTick()>floorAutoDragStopAt)
	{
		scheduledFloorOffset = {_x = 0, _y = 0};
		core_CancelInterval(floorAutoDragInterval);
		floorAutoDragInterval = 0;
	}
}

function officeMapScrollBy(offset)
{
	if(floorDragInterval != 0 ||  floorAttachInterval != 0)
	{
		return;
	}
	floorAutoDragStopAt = core_GetTick()+150;
	scheduledFloorOffset._x = offset._x;
	scheduledFloorOffset._y = offset._y;
	if(floorAutoDragInterval!=0)
	{
		return;
	}
	gui_StopSoftActorWatch();
	floorAutoDragInterval = core_SetIntervalLocal("main_office_level",20,"floorOffsetLoop",offset);
}

function floorAttachLoop(param)
{
	if(floorAttachTarget == ""){
		core_CancelInterval(floorAttachInterval);
		floorAttachInterval = 0;
		return;
	}
	if(floorAttachTargetTimeFrom == 0)
	{
		floorAttachTargetTimeFrom = core_GetTick();
		floorAttachTargetPosFrom = core_GetNode("officeFloor");
	}
	local ok = false;
	local offset = {_x=0, _y=0};
	if(floorAttachTargetType==0)
	{// Привязка к актеру
		local actor=actor_GetActor(floorAttachTarget);
		if(actor != false){
			local actorNode=core_GetNode(actor.ActorID);
			if(actorNode != false){
				ok = true;
				offset._x = actorNode._x;
				offset._y = actorNode._y;
			}
		}
	}
	if(floorAttachTargetType==1)
	{// Привязка к локации
		local loca=nloc_GetLocation(floorAttachTarget);
		if(loca != false){
			ok = true;
			offset._x = -loca._x;//Для (floorAttachTargetTime == 0) это неверно
			offset._y = -loca._y;
		}	
	}
	if(ok){
		local floor=core_GetNode("officeFloor");
		if(floorAttachTargetTime == 0){
			floor._xNew <- -offset._x;
			floor._yNew <- -offset._y;

			floor._x+=0.2*(floor._xNew-floor._x);
			floor._y+=0.2*(floor._yNew-floor._y);
		}else{
			// lerpим
			local timeNow = core_GetTick();
			if(timeNow<floorAttachTargetTimeFrom+floorAttachTargetTime){
				local needle = (timeNow - floorAttachTargetTimeFrom).tofloat()/floorAttachTargetTime.tofloat();
				floor._x = (floorAttachTargetPosFrom._x*(1-needle)+offset._x*(needle));
				floor._y = (floorAttachTargetPosFrom._y*(1-needle)+offset._y*(needle));
				//core_Warning(format("needl=%f %i ti=%i",needle,timeNow-floorAttachTargetTimeFrom,floorAttachTargetTime));
			}else{
				// Застреваем?
				//floorAttachTarget = "";
			}
		}
		clampPosition(floor);
		core_SetNode("officeFloor",floor);
	}else{
		floorAttachTarget = "";
	}
}

function actor_DetachCamera()
{
	floorAttachTarget="";
}

function location_DetachCamera()
{
	actor_DetachCamera();
}

function actor_AttachCamera(Actor)
{
	gui_StopSoftActorWatch();
	floorAttachTargetType=0;
	floorAttachTarget=Actor.Name;
	if(floorAttachInterval != 0)
	{
		core_CancelInterval(floorAttachInterval);
	}
	floorAttachTargetTime = 0;
	if("_time" in Actor){
		floorAttachTargetTime = Actor._time*1000.0;
	}
	floorAttachTargetTimeFrom = 0;
	floorAttachInterval = core_SetIntervalLocal("main_office_level",10,"floorAttachLoop",floorAttachTarget);
}

function location_AttachCamera(Location)
{
	//g_DebugCmap = true;
	gui_StopSoftActorWatch();
	floorAttachTargetType=1;
	floorAttachTarget=Location;
	if(floorAttachInterval != 0)
	{
		core_CancelInterval(floorAttachInterval);
	}
	local locFixed=nloc_GetLocation(floorAttachTarget);
	floorAttachTarget=locFixed._name;
	floorAttachTargetTime = 100;
	if("_time" in Location){
		floorAttachTargetTime = Location._time*1000.0;
	}
	floorAttachTargetTimeFrom = 0;
	floorAttachInterval = core_SetIntervalLocal("main_office_level",20,"floorAttachLoop",floorAttachTarget);
}

function gui_StopSoftActorWatch()
{
	if(floorSoftAttachInterval!=0){
		core_CancelInterval(floorSoftAttachInterval);
		floorSoftAttachInterval=0;
	}
}

function gui_SoftActorWatch(thisActor)
{
	gui_StopSoftActorWatch();
	if(game_IsCutscene()){
		// Тут не работает
		return;
	}
	local actor=actor_IsActorExist(thisActor);
	if(actor==false){
		return;
	}
	floorAttachTarget=actor.Name;
	floorAttachTargetType=0;
	floorSoftAttachInterval = core_SetIntervalLocal("main_office_level",20,"floorAttachLoop",floorAttachTarget);
}

function location_JumpCamera(Location)
{
	gui_StopSoftActorWatch();
	local locFixed=nloc_GetLocation(Location);
	setFloorPosition( { _x=-locFixed._x , _y=-locFixed._y } );
}

function location_JumpActor(actor)
{
	gui_StopSoftActorWatch();
	local locFixed=actor_GetActorPos(actor);
	setFloorPosition( { _x=-locFixed._x , _y=-locFixed._y } );
}

function gui_CenterCameraOnPers(thisActor)
{
	gui_StopSoftActorWatch();
	local persPos = actor_GetActorPos(thisActor);
	setFloorPosition({_x=-persPos._x, _y=-persPos._y});
}