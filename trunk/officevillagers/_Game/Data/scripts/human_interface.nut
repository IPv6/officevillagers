gFlag_hIntCloseOnProfchange <- true;
gFlag_hIntCloseOnMoveout <- true;
gFlag_hIntRawSetProfession <- false;
gFlag_hIntNoPauseOnOpen <- false;
gFlag_hIntStagerFocusActor <- false;
gFlag_hIntCenterCameraOnPers <- false;
function actorWait_InterfaceOpened(thisActor, whatWait)
{
	if(whatWait){
		//core_Warning("waiting for interface");
		gFlag_hIntNoPauseOnOpen = true;// Игра не должна быть на паузе - иначе это эейтствие не будет думаться...
		while(!humanInterfaceFocusActor || humanInterfaceFocusActor.Name != thisActor.Name)
		{
			::suspend();
		}
		//core_Warning("waiting for interface stop");
		gFlag_hIntNoPauseOnOpen=false;// Возвращаем значение
	}
}

function actor_IsProfOk(thisActor,profName)
{
	if(strstr(thisActor.ValidProfessions,profName)){
		return true;
	}
	if(isProfExpaOnProfOk(thisActor,profName)){
		return true;
	}
	return false;
}

function openInterfaceOnPerson(thisActor, parentNode)
{
	humanInterfaceFocusActor = actor_GetActor(thisActor);
	if("_ProfChangeAccess" in humanInterfaceFocusActor && humanInterfaceFocusActor._ProfChangeAccess==false){
		// Нельзя
		return
	}
	if(actor_IsProfOk(humanInterfaceFocusActor,"AUTHOR")==false 
		&& actor_IsProfOk(humanInterfaceFocusActor,"FINANCIER")==false
		&& actor_IsProfOk(humanInterfaceFocusActor,"CREATOR")==false
		&& actor_IsProfOk(humanInterfaceFocusActor,"JANITOR")==false){
		// Nothing to change!
		return;
	}
    if(!parentNode){
        parentNode = humanInterfaceFocusActor.ActorID;
    }
	// Выравниваем камеру на персонажа если можно
	if(gFlag_hIntCenterCameraOnPers){
		local persPos = actor_GetActorPos(humanInterfaceFocusActor);
		setFloorPosition({_x=-persPos._x, _y=-persPos._y});
		core_SetTimeout(1,"core_SetCurrentCursorPosition({_x=0.0, _y=0.0},false,core_GetNode(humanInterfaceFocusActor.ActorID));");
	}
	core_OpenDialog("person_interface");
	core_AttachToNode("person_interface", parentNode);
	if(gFlag_hIntNoPauseOnOpen){
		game_Pause(false);
	}
}

function closePersonInterfaceDialog(bOnMoveout, bOnProffesion)
{
	if(bOnMoveout && !gFlag_hIntCloseOnMoveout){
		return;
	}
	if(bOnProffesion && !gFlag_hIntCloseOnProfchange){
		return;
	}
	core_CloseDialog("person_interface");
}

function initPersonInterfaceDialog()
{
	//core_Warning("person interface opened");
	if(humanInterfaceFocusActor == false){
		core_Alert("Focus actor not found!");
		return;
	}
	if(actor_SetMovementsPause(humanInterfaceFocusActor,true,"humanInterface")){
		actor_PauseInteractivity(humanInterfaceFocusActor,true);
	}
	local actorTitle=humanInterfaceFocusActor.NameLocalized;
	core_SetNodeText("person_interface_name",actorTitle);
	
	local bt;
	bt = core_GetNode("_piAUTHORbt");
	bt._alpha=1.0;
	if(actor_IsProfOk(humanInterfaceFocusActor,"AUTHOR")==false){
		bt._alpha=0.2;
	}
	core_SetNode("_piAUTHORbt",bt);
	
	bt = core_GetNode("_piFINANCIERbt");
	bt._alpha=1.0;
	if(actor_IsProfOk(humanInterfaceFocusActor,"FINANCIER")==false){
		bt._alpha=0.2;
	}
	core_SetNode("_piFINANCIERbt",bt);
	
	bt = core_GetNode("_piCREATORbt");
	bt._alpha=1.0;
	if(actor_IsProfOk(humanInterfaceFocusActor,"CREATOR")==false){
		bt._alpha=0.2;
	}
	core_SetNode("_piCREATORbt",bt);
	
	bt = core_GetNode("_piJANITORbt");
	bt._alpha=1.0;
	if(actor_IsProfOk(humanInterfaceFocusActor,"JANITOR")==false){
		bt._alpha=0.2;
	}
	core_SetNode("_piJANITORbt",bt);
}

function deinitPersonInterfaceDialog()
{
	//core_Warning("person interface closed");
	if(humanInterfaceFocusActor == false){
		core_Alert("Focus actor not found!");
		return;
	}
	if(actor_SetMovementsPause(humanInterfaceFocusActor,false,"humanInterface")){
		actor_PauseInteractivity(humanInterfaceFocusActor,false);
	}
	humanInterfaceFocusActor = false;
}

function onClickOutPInterface(param)
{
	closePersonInterfaceDialog(true,false);
	return false;// Чтобы это не считалось за клик в общемто!
}

function onDragOutPInterface(param)
{
	// Проверяем где курсор
	local cursorPos=core_GetCurrentCursorPosition("person_interface",true); 
	local PINode=core_GetNode(param._id);
	//core_ConvertXYCoords(PINode,PINode,"person_interface");
	//core_Warning(format("%i:%i vs %i:%i",cursorPos._x,cursorPos._y,PINode._x,PINode._y));
	if(cursorPos._x>=PINode._x-PINode._w/2
		&& cursorPos._x<=PINode._x+PINode._w/2
		&& cursorPos._y>=PINode._y-PINode._h/2
		&& cursorPos._y<=PINode._y+PINode._h/2){
		//core_Warning("cursor out skipped!");
		return;
	}
	closePersonInterfaceDialog(true,false);
}

function personChooseProfession(profName)
{
	core_SetTimeoutLocal("main_office_level",100,"core_SetAnyInputOcured();");// Для туториала и так просто
	if(actor_IsProfOk(humanInterfaceFocusActor,profName)==false){
		return;
	}
	$ifdebug core_Warning(format("changin '%s' to %s",humanInterfaceFocusActor.Name,profName));
	actor_Unsit(humanInterfaceFocusActor);// На случай если он сидел
	actor_UnsetTeacher(humanInterfaceFocusActor);// На случай если он бегал за кемлибо
	if(gFlag_hIntRawSetProfession){
		actor_SetProfessionRaw(humanInterfaceFocusActor,profName);
	}else{
		actor_SetProfession(humanInterfaceFocusActor,profName);
	}
	quest_conditions({_fromSetProf=1, who=humanInterfaceFocusActor, prof=profName});
	closePersonInterfaceDialog(false,true);
}

mouseOverCountCurrent <- "";
function onPUIMouseOut(param)
{
	if(mouseOverCountCurrent==param){
		core_SetNodeText("_pistatus","");
	}
}

function onPUIMouseOver(param)
{
	mouseOverCountCurrent = param;
	core_SetNodeText("_pistatus",core_GetTranslation(param));
}

function openStagerInterfaceOnPerson(thisActor)
{
	gFlag_hIntStagerFocusActor = thisActor;
	core_OpenDialog("person_interface_stager");
}

function closePersonStagerInterfaceDialog()
{
	core_CloseDialog("person_interface_stager");
}

function initPersonStagerInterfaceDialog()
{
	if(gFlag_hIntStagerFocusActor == false){
		core_Alert("StagerFocus actor not found!");
		return;
	}
	local actorTitle=gFlag_hIntStagerFocusActor.NameLocalized;
	core_SetNodeText("person_interface_name",actorTitle);
}

function deinitPersonStagerInterfaceDialog()
{
	gFlag_hIntStagerFocusActor = false;
}

function personChooseProfessionStager(profName)
{
	actor_SetProfessionStager(gFlag_hIntStagerFocusActor,profName);
	actor_AddAttribute(actor_GetActor(gFlag_hIntStagerFocusActor),"STAGER_PROF",profName);
	closePersonStagerInterfaceDialog();
}