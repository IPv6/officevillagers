g_techTree <- {};
g_TechsOnPage <- 4;
g_TechInterfaceOpened <- false;

function level_OpenTechs(param)
{
	if(floorAttachInterval != 0 || floorAutoDragInterval != 0
		|| humanDragInterval != 0){
		// У нас идет скрипт... нельзя
		return;
	}
	gui_ShowDialog(null,null);
	core_OpenDialog("level_techs");
}

function level_CloseTechs()
{
	if(core_IsDialogOpened("level_techs")){
		core_CloseDialog("level_techs");
	}
}

function deinitTechsDialog()
{
	g_TechInterfaceOpened = false;
}

function initTechsDialog()
{
	local i=0;
	for(i=0;i<g_TechsOnPage;i++){
		local slotNodeName=format("tech_slot%i",i);
		core_DeleteNode(slotNodeName);
		core_CreateSprite("\\gui\\level_quest_slot.spr","level_techs",{x=-4.5, y=8.6-5.2*i, z=-0.0002, w=0.97, h=1.5, name=slotNodeName });
		core_SetNodeText(core_GetNode(slotNodeName),"");
	}
	local iSlotNum=0;
	// Пробегаемся по технологиям и добавляем те что доступны к покупке
	local points=actor_GetAttributeN("Office","ideas");
	foreach(singletech in g_techTree.techs.tech)
	{
		if(iSlotNum>=g_TechsOnPage){
			break;
		}
		//core_Alert(dumpVariable(singletech));
		// Проверяем доcтупна ли технология
		if(singletech.Points2View>points){
			continue;
		}
		local bPrereqFailed=false;
		foreach(prereq in singletech.Prerequisite){
			core_Warning(format("prereq %s==%i",prereq.OfficeAttribute,actor_GetAttributeN("Office",prereq.OfficeAttribute)));
			if(actor_GetAttributeN("Office",prereq.OfficeAttribute)!=prereq.ValueN){
				bPrereqFailed=true;
			}
		}
		if(bPrereqFailed){
			continue;
		}
		// Добавляем
		local slotNodeName=format("tech_slot%i",iSlotNum);
		core_CreateSprite(singletech.Sprite,slotNodeName,{x=0, y=0, z=-0.0002, w=12, h=4});
		core_SetNodeAction(core_GetNode(slotNodeName),format("level_BuyTechs(\"%s\");",singletech.Name));
		iSlotNum++;
	}

	// Число идей
	g_TechInterfaceOpened = true;
	gui_UpdateHud();
	// Окно помощи при первом заходе
	if(core_GetProfileOption("TutorialTechsHelp")!=1){
		core_SetProfileOption("TutorialTechsHelp",1);
		helpDialog("level_techs_help");
	}
}

function level_BuyTechs(techNumber)
{
	local points=actor_GetAttributeN("Office","ideas");
	$ifdebug core_Warning(techNumber+" test:");
	foreach(singletech in g_techTree.techs.tech)
	{
		if(singletech.Name!=techNumber){
			continue;
		}
		if(singletech.Points2Buy>points){
			core_CreateSprite("gui\\buttons\\tech_points_hightlight.spr","hudIdeasCount2",{y=0.3, w=8.5, h=1.8});
			$ifdebug core_Warning("not enoughp points for "+techNumber);
			return;
		}
		// Покупаем!!!
		quest_conditions({_fromTech=true, newTech=techNumber});
		$ifdebug core_Warning(techNumber+" куплен!");
		actor_SetAttribute("Office","ideas",points-singletech.Points2Buy);
		foreach(result in singletech.Result){
			actor_SetAttribute("Office",result.OfficeAttribute,result.ValueN);
		}
		// Все заново!!!
		initTechsDialog();
		return;
	}
}
