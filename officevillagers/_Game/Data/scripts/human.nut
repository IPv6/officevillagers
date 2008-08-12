// Глобальные переменные
humanDragInterval <- 0;
humanDragLastId <- 0;
humanDragLastRealId <- 0;
humanDragLastPos <- {_x = 0, _y = 0};
humanDragFirstPos <- {_x = 0, _y = 0};
humanDragActorFirstPos <- {_x = 0, _y = 0};
humanDragRealDrag <- 2;
humanInterfaceFocusActor <- false;
gFlag_hDragAllowCameraSlide <- true;
tryUnsitFirst <- false;
humanDragClipArea <- {_left=0.26,_top=0.05,_right=0.97,_bottom=0.66, _zazro=0.06}
function humanDragLoop(param)
{
	local curCurPos = core_GetCurrentCursorPosition("officeFloor",true);
	{
		local xoff=abs(humanDragFirstPos._x-curCurPos._x);
		local yoff=abs(humanDragFirstPos._y-curCurPos._y);
		if(xoff<0.1 && yoff<0.1){
			if(humanDragRealDrag > 1){
				humanDragRealDrag = 0;
			}
		}else{
			humanDragRealDrag = 1;
			if(tryUnsitFirst){
				//core_Warning("try unsit!");
				tryUnsitFirst=false;
				local actor=actor_GetActor(humanDragLastId);
				actor_Unsit_clear(actor);
				actor_MakePocketEmpty(actor);
			}
		}
	}		
	if(curCurPos._x != humanDragLastPos._x || curCurPos._y != humanDragLastPos._y)
	{
		local spritePos = core_GetNode(humanDragLastId);
		if(spritePos && humanDragRealDrag>0){
			if(gFlag_hDragAllowCameraSlide){// И двигаем фон к персу... типа центрируем
				local officeNode=core_GetNode("officeFloor");
				local centroidOffset={_x=0,_y=0};
				local curCurRelPos = core_GetCurrentCursorPosition();//0:0-1:1
				//core_Warning(format("%.02f:%.02f l=%.02f r=%.02f",curCurRelPos._x, curCurRelPos._y,humanDragClipArea._left,humanDragClipArea._right));
				if(fabs(curCurRelPos._x-humanDragClipArea._left)<humanDragClipArea._zazro){
					centroidOffset._x=0.5;
				}
				if(fabs(curCurRelPos._x-humanDragClipArea._right)<humanDragClipArea._zazro){
					centroidOffset._x=-0.5;
				}
				if(fabs(curCurRelPos._y-humanDragClipArea._top)<humanDragClipArea._zazro){
					centroidOffset._y=-0.5;
				}
				if(fabs(curCurRelPos._y-humanDragClipArea._bottom)<humanDragClipArea._zazro){
					centroidOffset._y=0.5;
				}
				/*local spritePosOnScreen=core_GetNodeOnScreenPosition(humanDragLastId);
				
				// Делаем чтобы камера смещалась так чтобы перс оказался в центре экрана
				centroidOffset._x-=(spritePosOnScreen._x-0.5)*2;//-((humanDragActorFirstPos._xOnScreen)*2);
				humanDragActorFirstPos._xOnScreen-=humanDragActorFirstPos._xOnScreen*0.01;
				
				centroidOffset._y+=(spritePosOnScreen._y-0.5)*2;//-((humanDragActorFirstPos._yOnScreen)*2);
				humanDragActorFirstPos._yOnScreen-=humanDragActorFirstPos._yOnScreen*0.01;*/
				
				if(fabs(centroidOffset._x)>0.38 || fabs(centroidOffset._y)>0.38*(4/3))
				{
					//core_Warning(abs(centroidOffset._x).tostring());
					setFloorPosition({_x=officeNode._x+centroidOffset._x*2.0,_y=officeNode._y+centroidOffset._y*2.0});
					//spritePos._x-=centroidOffset._x;
					//spritePos._y-=centroidOffset._y;
				}
			}
			// Снеппим к руке
			spritePos._x = curCurPos._x;
			if(core_GetCursor()!=0){
				spritePos._x += 0.5;
			}
			spritePos._y = curCurPos._y-2;
			spritePos._z = -1.01;
			core_SetNode(humanDragLastId,spritePos);
			actor_UpdatePosition(humanDragLastId);
		}
	}
	humanDragLastPos = curCurPos;
	if(!core_GetIsLeftButtonDown())
	{
		core_SetCurrentCursorClipArea();// Сбрасываем ограничение на курсор
		{//  Возвращаем в свой z
			local spritePos = core_GetNode(humanDragLastId);
			if(spritePos){
				spritePos._z = -0.001;
				core_SetNode(humanDragLastId,spritePos);
			}
		}
		local spritePos = core_GetNode(humanDragLastId);
		if(humanDragRealDrag==0){
			// Это клик!!!
			if(spritePos){
				local actor=actor_GetActor(humanDragLastId);
				if(actor_IsSitting(actor)){
					openInterfaceOnPerson(actor,humanDragLastRealId);
				}else{
					openInterfaceOnPerson(actor,null);
				}
			}
		}else{
			// Это драг-н-дроп!!!
			local actor=actor_GetActor(humanDragLastId);
			actor_OnDropActor(actor,spritePos);
			//core_Alert(format("drop end %f:%f",spritePos._x,spritePos._y));
		}
		local humanShadow=core_GetNodeSubnode(humanDragLastId,${SUBID_SHADOW});
		humanShadow._alpha=1;
		core_SetNode(humanShadow,humanShadow);
		actor_SetMovementsPause(humanDragLastId,false,"humanDrag");
		core_CancelInterval(humanDragInterval);
		humanDragInterval = 0;
		if(spritePos){
			// Если был резкий "бросок" при дропе - центрируем камеру на персе
			local spritePos2 = core_GetNode(humanDragLastId);
			if(fabs(spritePos2._x-spritePos._x)+fabs(spritePos2._y-spritePos._y) > 3.0){
				gui_CenterCameraOnPers(actor_GetActor(humanDragLastId));
			}
		}
	}
}

function human2actor(nodeID)
{
	local actor=actor_GetActor(nodeID);
	if(!actor){
		// Возможно сидит
		local node=core_GetNode(nodeID);
		if(node){
			local sitCode=core_GetNodeAttribute(node,"ACTORID");
			if(sitCode){
				actor=actor_GetActor(sitCode);
				//core_Warning(format("sit pers found! %i",sitCode));
			}
		}
	}
	if(!actor){
		return false;
	}
	return actor;
}

function onDragActor(node)
{
	local actor=human2actor(node._parentid);
	if(!actor){
		return;
	}
	if(!actor_IsInteractive(actor)){
		return false;
	}
	if(floorDragInterval==0 && humanDragInterval==0)
	{
		humanDragLastPos = core_GetLastClickPosition("officeFloor");
		humanDragFirstPos = core_GetCurrentCursorPosition("officeFloor",true);
		humanDragLastId = actor.ActorID;
		humanDragActorFirstPos = core_GetNode(humanDragLastId);
		local actorOnScreen = core_GetNodeOnScreenPosition(humanDragActorFirstPos);
		humanDragActorFirstPos._xOnScreen <- actorOnScreen._x-0.5;//-0.5:0.5
		humanDragActorFirstPos._yOnScreen <- actorOnScreen._y-0.5;//-0.5:0.5
		//core_Alert(format("%.02f-%.02f",humanDragActorFirstPos._xOnScreen,humanDragActorFirstPos._yOnScreen));
		humanDragLastRealId = node._parentid;
		humanDragRealDrag = 2;
		tryUnsitFirst = true;
		actor_SetMovementsPause(humanDragLastId,true,"humanDrag");
		if(gFlag_hDragAllowCameraSlide){
			// Ограничение на движение - только если разрешен слайд. Для туториала он запрещен и  клиареа там не имеет смысла!
			core_SetCurrentCursorClipArea(humanDragClipArea);
		}
		local humanShadow=core_GetNodeSubnode(humanDragLastId,${SUBID_SHADOW});
		humanShadow._alpha=0;
		core_SetNode(humanShadow,humanShadow);
		// Запускаем драг
		humanDragInterval = core_SetIntervalLocal("main_office_level",20,"humanDragLoop",humanDragLastPos);
	}
	return;
}

function onActorMouseOver(node)
{
	local actor=human2actor(node._parentid);
	if(!actor){
		return;
	}
	if(floorDragInterval==0 && humanDragInterval==0)
	{
		//actor_SetMovementsPause(actor,true,"humanMOver");
		actor_ShowLabels(actor,true,node._parentid);//node._parentid может указывать на сидячего перса!
	}
}

function onActorMouseOut(node)
{
	local actor=human2actor(node._parentid);
	if(!actor){
		return;
	}
	// Анпаузим даже если перс в драге и т.п. -там свои паузы ставятся!
	//actor_SetMovementsPause(actor,false,"humanMOver");
	// ============
	if(floorDragInterval==0 && humanDragInterval==0)
	{
		actor_ShowLabels(actor,false,node._parentid);
	}
}
