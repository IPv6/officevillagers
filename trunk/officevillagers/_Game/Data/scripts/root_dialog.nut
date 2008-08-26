g_KeyHandlerStates <- {};
function levelKeyHandler(key)
{
	if(core_IsDebug() && key._pressed)
	{
		//core_Warning(format("Keypress: %i",key._key));
		if(key._control)
		{
			//17-CONTROL 16-SHIFT 18-ALT
			//if(key._key==77)// 'M'
			//if(key._key==78)//  'N'
			//if(key._key==79)//  'O'
			//105)//  Num9
			if(key._key==145)// Control-Shift-ScrollLock
			{
				game_SetGameSpeed(20.0);
				core_Warning(format("Current game speed: %f",game_GetGameSpeed()));
			}
			if(key._key==33)// PageUp
			{
				game_SetGameSpeed(game_GetGameSpeed()+0.5);
				//core_Warning(format("Current game speed: %f",game_GetGameSpeed()));
			}
			if(key._key==34)// PageDn
			{
				game_SetGameSpeed(game_GetGameSpeed()-0.5);
				//core_Warning(format("Current game speed: %f",game_GetGameSpeed()));
			}
			if(key._key==76)// 'L'
			{
				if(core_IsDialogOpened("main_office_level")){
					// Оверрайтим положение камеры
					local officePos = core_GetNode("officeFloor");
					_saveCameraOverwrite.overwrite <- true;
					_saveCameraOverwrite.cameraX <- officePos._x;
					_saveCameraOverwrite.cameraY <- officePos._y;
					core_SetTimeout(1000,"core_OpenDialog(\"main_office_level\")");
					//core_Warning("Restarting level!");
				}//else...
				return;//!!!!
			}
			if(key._key==75)// 'K'
			{
				level_CheckLocks();
			}
			if(key._key==69)//  'E'
			{
				local curCurPos = core_GetCurrentCursorPosition("officeFloor",true);
				local objectsArray=actor_GetActors( {_byPosition=curCurPos, _byPositionLimits={_l=2,_r=2,_u=3,_d=1}, _closest=true});
				g_LastEGA_Actor = null;
				if(objectsArray.len()==1){
					g_LastEGA_Actor = objectsArray[0];
				}else{// Выбор!
					local fnames=[],i=0;
					for(i=0;i<objectsArray.len();i++){
						fnames.append(objectsArray[i].Name);
					}
					local choice=core_AskChoice(fnames,"Выберите группу");
					if(choice!=-1)
					{
						g_LastEGA_Actor=fnames[choice];
					}
				}
				core_OpenDialog("editor_global_attr");
			}
			if(key._key==189)//  '-'
			{
				SaveGameAttrEditDialog();
			}
			if(key._key==187)//  '='
			{
				LoadGameAttrEditDialog();
			}
			//=============================================
			if(key._key==49)//  '1'
			{
				quest_open(3);
			}
			if(key._key==50)//  '2'
			{
				quest_close(3);
			}
		}
		//if(key._rmb)
		{
			//=============================================
			if(key._key==19)// Pause
			{
				if(game_GetGameSpeed()>0){
					game_SetGameSpeed(0);
				}else{
					game_SetGameSpeed(1.0);
				}
				//core_Warning(format("Current game speed: %f",game_GetGameSpeed()));
			}
			if(key._key==103)//  Num7
			{
				actor_InitTalking(actor_GetActor("ACTOR.SHEKEL"),actor_GetActor("ACTOR.AGATA"),"OfficeTalks::DOT10");
			}
			if(key._key==104)//  Num8
			{
				gui_EnableInterface(false);
			}
			if(key._key==105)//  Num9
			{
				gui_EnableInterface(true);
			}
			if(key._key==97)//  Num1
			{
				local curCurPos = core_GetCurrentCursorPosition("officeFloor",true);
				game_VisualizeLocations(curCurPos._x,curCurPos._y);
			}
			if(key._key==110)//  Num.
			{
				local curCurPos = core_GetCurrentCursorPosition("officeFloor",true);
				core_Warning(format("Navimap at %.02f:%.02f",curCurPos._x,curCurPos._y));
				game_VisualizeNavimap(curCurPos._x,curCurPos._y,1);
			}
			if(key._key==98)//  Num2
			{
				local curCurPos = core_GetCurrentCursorPosition("officeFloor",true);
				game_VisualizeNavimap(curCurPos._x,curCurPos._y,2);
			}
			if(key._key==99)//  Num3
			{
				local curCurPos = core_GetCurrentCursorPosition("officeFloor",true);
				game_VisualizeNavimap(curCurPos._x,curCurPos._y,0);
			}
			if(key._key==96)//  Num0
			{
				if("dbgOver" in g_KeyHandlerStates && g_KeyHandlerStates.dbgOver==true){
					g_KeyHandlerStates.dbgOver=false;
					actor_ShowLabels(g_KeyHandlerStates.dbgOverName,0);
				}else{
					local curCurPos = core_GetCurrentCursorPosition("officeFloor",true);
					local objectsArray=actor_GetActors( {_byPosition=curCurPos, _byPositionLimits={_l=2,_r=2,_u=3,_d=1}, _closest=true});
					if(objectsArray && objectsArray.len()>0){
						//core_Warning(format("actor found %s",objectsArray[0].Name));
						actor_ShowLabels(objectsArray[0],200);
						g_KeyHandlerStates.dbgOver<-true;
						g_KeyHandlerStates.dbgOverName<-objectsArray[0].Name;
					}else{
						core_Warning(format("No actor found at %.02f:%.02f",curCurPos._x,curCurPos._y));
					}
				}
			}
		}
	}
	if(key._pressed){
		if(core_IsDebug()){
			if(key._key==49)//1
			{
				game_SetGameSpeed(1.0);
				core_Warning(format("Current game speed: %f",game_GetGameSpeed()));
			}
			if(key._key==50)//2
			{
				game_SetGameSpeed(3.0);
				core_Warning(format("Current game speed: %f",game_GetGameSpeed()));
			}
			if(key._key==51)//3
			{
				game_SetGameSpeed(5.0);
				core_Warning(format("Current game speed: %f",game_GetGameSpeed()));
			}
			if(key._key==52)//4
			{
				game_SetGameSpeed(7.0);
				core_Warning(format("Current game speed: %f",game_GetGameSpeed()));
			}
			if(key._key==53)//5
			{
				game_SetGameSpeed(9.0);
				core_Warning(format("Current game speed: %f",game_GetGameSpeed()));
			}
		}
		// Основное
		if(key._key==2)//Правая кнопка мыши
		{
			if(!game_IsCutscene() && !humanInterfaceFocusActor){
				gui_JumpToNextActor();
			}
		}
		if(key._key==39)//Right,D
		{
			officeMapScrollBy({_x=-0.3,_y=0});
		}
		if(key._key==68)//Right,D
		{
			officeMapScrollBy({_x=-0.9,_y=0});
		}
		if(key._key==37)//Left
		{
			officeMapScrollBy({_x=0.3,_y=0});
		}
		if(key._key==65)//Left
		{
			officeMapScrollBy({_x=0.9,_y=0});
		}
		if(key._key==40)//Down
		{
			officeMapScrollBy({_x=0,_y=0.3});
		}
		if(key._key==83)//Down
		{
			officeMapScrollBy({_x=0,_y=0.9});
		}
		if(key._key==38)//Up
		{
			officeMapScrollBy({_x=0,_y=-0.3});
		}
		if(key._key==87)//Up
		{
			officeMapScrollBy({_x=0,_y=-0.9});
		}
		if(key._key==32)// Space
		{
			if(game_GetGameSpeed()>0){
				game_SetGameSpeed(0);
			}else{
				game_SetGameSpeed(1.0);
			}
			//core_Warning(format("Current game speed: %f",game_GetGameSpeed()));
		}
	}
} 

function helpDialog(helpDialogName)
{
	core_OpenDialog(helpDialogName);
}
