function level_OpenMenu(param)
{
	if(floorAttachInterval != 0 || floorAutoDragInterval != 0
		|| humanDragInterval != 0){
		// У нас идет скрипт... нельзя
		return;
	}
	if(game_IsCutscene()){
		// катсцена
		return;
	}
	if(gui_IsFade()){
		// идет фейд
		return;
	}
	gui_ShowDialog(null,null);
	core_OpenDialog("level_menu_dialog");
}

function game_initMainMenuDialog()
{
	if(core_GetProfileOption("GameWasStarted")!=1)
	{
		core_DeleteSprite("StartOverButton");
	}
	if(core_GetIniParameter("EnableIngameEditor")!="0"){
		core_CreateSprite("gui\\editor\\main_menu_editor.spr","main_menu");
	}
	Profiles_setProfileGreeting();
}

function startGameButton(forceNewGame)
{
	if(forceNewGame)
	{
		core_SetProfileOption("TutorialDone",0);
	}
	core_SetProfileOption("GameWasStarted",1);
	if(core_GetProfileOption("TutorialDone")!=1)
	{
		core_SetProfileOption("TutorialDone",1);
		TransitionOpenDialog("comics_startup","fade");
	}else{
		// Запускаем игру, не устанавливая сейв файл. в этом случае будет взят последняя сохраненка
		TransitionOpenDialog("main_office_level","fade");
	}
}

//=========================+++++++++++==================================================
//========================= OPTIONS +=================================================
//=========================+++++++++++==================================================

function handleVolume_Effects(param)
{
	local clickPos=core_GetLastClickPosition(param,true);
	local clickNod=core_GetNode(param);
	local xOff=(clickPos._x+clickNod._w*0.5)/clickNod._w*1.1;
	core_SetSoundVolume(1,xOff<0.03?0:xOff);
	core_SetNodeAttribute(param,"sliderval",xOff<0.03?0.0001:xOff);
}

function handleVolume_Music(param)
{
	local clickPos=core_GetLastClickPosition(param,true);
	local clickNod=core_GetNode(param);
	local xOff=(clickPos._x+clickNod._w*0.5)/clickNod._w*1.1;
	core_SetSoundVolume(0,xOff<0.03?0:xOff);
	core_SetNodeAttribute(param,"sliderval",xOff<0.03?0.0001:xOff);
}
//=========================+++++++++++==================================================
//========================= MAGNAME =================================================
//=========================+++++++++++==================================================
g_newMagName <- "";
function game_AskMagazineName()
{
	core_OpenDialog("magazine_name");
	while(core_IsDialogOpened("magazine_name")){
		::suspend();
	}
	local sName=g_newMagName;//core_GetNodeAttribute("magedit_slot","magedit_text");
	if(sName && sName.len()>0){
		game_SetMagazinName(sName);
	}
	//core_Warning("New name: ["+sName+"]!");
}

function game_initMagNameDialog()
{
	g_newMagName = "";
	core_SetNodeAttribute("magedit_slot","magedit_text",game_GetMagazinName());
}

function game_CloseMagNameDialog()
{
	local sName=core_GetNodeAttribute("magedit_slot","magedit_text");
	if(sName && sName.len()>0){
		g_newMagName = sName;
		core_CloseDialog("magazine_name");
	}
}

function game_initPuzzleDialog()
{
	core_SetNodeText("puzzle_magname","<font-size:+2>\n"+game_GetMagazinName());
}

function game_closePuzzleDialog()
{
	core_CloseDialog("puzzle_main");
}

function game_StartPuzzle()
{
	core_OpenDialog("puzzle_main");
}

g_puzzlebg_num <- 0;
function game_PuzzleDialogNextBG()
{
	local bgSprite=core_GetNode("puzzle_bgroot");
	core_DeleteSprite("puzzle_bg_slide");
	g_puzzlebg_num= g_puzzlebg_num+1;
	if(g_puzzlebg_num>=3)
	{
		g_puzzlebg_num = 0;
	}
	core_CreateSprite(format("gui\\puzzle\\puzzle_bg%i.jpg",g_puzzlebg_num),"puzzle_bgroot",{z=-0.0001, name="puzzle_bg_slide", w=bgSprite._w, h=bgSprite._h});
}

g_puzzlelogo_num <- 0;
function game_PuzzleDialogNextLogo()
{
	local bgSprite=core_GetNode("puzzle_logoroot");
	core_DeleteSprite("puzzle_logo_slide");
	g_puzzlelogo_num= g_puzzlelogo_num+1;
	if(g_puzzlelogo_num>=3)
	{
		g_puzzlelogo_num = 0;
	}
	core_CreateSprite(format("gui\\puzzle\\puzzle_logo%i.jpg",g_puzzlelogo_num),"puzzle_logoroot",{z=-0.00011, name="puzzle_logo_slide", w=bgSprite._w, h=bgSprite._h});
}

//=========================+++++++++++==================================================
//========================= PROFILES ==================================================
//=========================+++++++++++==================================================
g_LastProfileName <- "";
g_ForCurrentProfile <- true;
function Profiles_askUserName(forCProf)
{
	g_ForCurrentProfile = forCProf;
	g_LastProfileName = core_GetActiveProfileName();
	core_OpenDialog("profile_name");
}

function Profiles_askNameInit()
{
	core_SetNodeAttribute("magedit_slot","magedit_text",g_LastProfileName);
}

function Profiles_askNameDeinit()
{
	core_SetProfileOption("UserNameAsked",1);
	local sName=core_GetNodeAttribute("magedit_slot","magedit_text");
	if(sName && sName.len()>0){
		g_LastProfileName = sName;
		core_CloseDialog("profile_name");
	}
	core_SwitchProfile(g_LastProfileName);
	core_SetProfileOption("UserNameAsked",1);// Уже...
	Profiles_setProfileGreeting();// Спросит имя если надо
	Profiles_initDialog();//Обновляем слоты
}

function Profiles_setProfileGreeting()
{
	if(core_GetProfileOption("UserNameAsked")!=1){
		Profiles_askUserName(true);
	}
	core_SetNodeText("profile_greeting",format("%s, %s!",core_Translate("GUI.USERPROFILE.GREETING"),core_GetActiveProfileName()));
}

function Profiles_setProfileSlot(slotName,slotValue)
{
	core_SetNodeText(slotName,slotValue);
	if(slotValue == core_GetActiveProfileName())
	{
		local marker=core_GetNode("profile_marker");
		local slot=core_GetNode(slotName);
		marker._x=slot._x;
		marker._y=slot._y;
		core_SetNode("profile_marker",marker);
		core_EnableNode("profile_marker",true);
	}
	if(slotValue.len()==0){
		core_EnableNode(slotName,false);
	}else{
		core_EnableNode(slotName,true);
	}
}

function Profiles_ActivateSlot(slotNum)
{
	local profname=core_GetPastProfileName(slotNum);
	core_SwitchProfile(profname);
	Profiles_setProfileGreeting();// Спросит имя если надо
	Profiles_initDialog();
}

function Profiles_AddProfile()
{
	Profiles_askUserName(false);
}

function Profiles_DelProfile()
{
	core_DeleteProfile(core_GetActiveProfileName());
	Profiles_initDialog();
	Profiles_setProfileGreeting();
}

function Profiles_initDialog()
{
	if(!core_IsDialogOpened("profiles_dialog")){
		return;
	}
	core_EnableNode("profile_marker",false);
	Profiles_setProfileSlot("profile_slot0",core_GetPastProfileName(0));
	Profiles_setProfileSlot("profile_slot1",core_GetPastProfileName(1));
	Profiles_setProfileSlot("profile_slot2",core_GetPastProfileName(2));
}

function Profiles_deinitDialog()
{
	core_SaveProfile();
}