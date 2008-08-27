function game_CutsceneBegin(skipParams)
{
	if(game_IsCutscene()){
		$ifdebug core_Alert("game_CutsceneBegin error: cutscene already present!");
		return;
	}
	if(!("lastActiveCutSceneNumber" in game_GetSafe())){
		gui_isCutSceneWasSkipped();// Создаем параметры необходимые
	}
	if(!("_skipSave" in skipParams) || (skipParams._skipSave==false)){
		game_SaveGame();
	}
	if(("_forcePersStop" in skipParams) && (skipParams._forcePersStop==true)){
		// Находим все актеров людей и включаем им катсцене-драйвер
		local i=0;
		local objectsArray=actor_GetActors({_byType=1});
		for(i=0;i<objectsArray.len();i++){
			actor_SwitchToAction(objectsArray[i],"Cutscene.driver",true);
		}
	}
	local cutSceneType=1;
	if(("_tutorial" in skipParams) && (skipParams._tutorial==true)){
		cutSceneType=2;
	}
	game_CutsceneBegin_i(cutSceneType);
	if("_who" in skipParams){
		gui_EnableSkipCurrentCutsceneButton(skipParams);
	}else{
		gui_DisableSkipCurrentCutsceneButton();
	}
	game_MarkSafe().lastActiveCutSceneNumber++;
	gui_EnableInterface(false);
	return;
}

function game_CutsceneEnd(params)
{
	if(!("_changeUI" in params) || params._changeUI==true){
		// Менять интерфейс надо даже если катсцены УЖЕ НЕТ
		// Для перехода между катсценами и с пропусков катсцен
		gui_EnableInterface(true);
	}
	gui_DisableSkipCurrentCutsceneButton();
	if(!game_IsCutscene()){
		return;
	}
	game_CutsceneEnd_i();
	// Сбрасываем всем персам ивенты
	local i=0;
	local objectsArray=actor_GetActors({_byType=1});
	for(i=0;i<objectsArray.len();i++){
	    
		if(objectsArray[i].Action=="Cutscene.driver"){
			actor_SwitchToAction(objectsArray[i],"",true);
		}
	}
	return;
}

function gui_StartCutscene(cutSceneEvent)
{
	gui_StopSoftActorWatch();
	local office = actor_GetActor();
	actor_SwitchToAction(office,cutSceneEvent);
}

// Вызывется из кнопки
SkipCurrentCutsceneParams <- {_who="Office", _action="", _altSwitch=false, _changeUI=true};
function gui_SkipCurrentCutscene(params)
{
     if(gui_IsFade()){
          return;
     }
     if(game_IsCutscene()){
          game_CutsceneEnd(SkipCurrentCutsceneParams);
     }
     gui_DisableSkipCurrentCutsceneButton();
     game_MarkSafe().lastSkippedCutScene <- game_GetSafe().lastActiveCutSceneNumber;
     if("_altSwitch" in SkipCurrentCutsceneParams){
          actor_SwitchActionAlt(SkipCurrentCutsceneParams._who);
     }else{
          actor_SwitchToAction(SkipCurrentCutsceneParams._who,SkipCurrentCutsceneParams._action);
     }
}


function gui_isCutSceneWasSkipped()
{
	if(!("lastActiveCutSceneNumber" in game_GetSafe())){
		game_GetSafe().lastActiveCutSceneNumber <- 0;
		game_MarkSafe().lastSkippedCutScene <- (-1);
	}
	if(game_GetSafe().lastSkippedCutScene == game_GetSafe().lastActiveCutSceneNumber){
		return true;
	}
	return false;
}

function gui_EnableSkipCurrentCutsceneButton(skipParams)
{
	SkipCurrentCutsceneParams=skipParams;
	core_EnableNode("GuiSkipCutscene",true);
}

function gui_DisableSkipCurrentCutsceneButton()
{
	core_EnableNode("GuiSkipCutscene",false);
}

function gui_EnableInterface(isEnable)
{
	if(isEnable){
		core_EnableNode("main_level_interface",true);
		core_EnableNode("cutscene_interface_static",false);
		setMainFloorClamp(true);
	}else{
		core_EnableNode("main_level_interface",false);
		core_EnableNode("cutscene_interface_static",true);
		setMainFloorClamp(false);
	}
}