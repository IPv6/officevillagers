/*
1)настрйоки
а)сменить время дедлайна
б)сменить усталость при которой работник уходит на перекур
в)сменить время кранча

2)новые комнаты
а)комната отдыха
б)кухня
в)игровые автоматы

3)уборщики
а)уборщкии разгребают завалы
б)ставят новую технику и муболь в комнаты
в)открывают главнюу дверь

4)авторы
а)появление аутсорсеров
б)возможность пистаь в соавторстве
в) ...тут ещё думаю
*/
function level_BuyTechs(techNumber)
{
}

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
	core_CloseDialog("level_techs");
}

function initTechsDialog()
{
/*
	local egaSlot0=core_GetNode("quest_slot0");
	local i=0;
	if(!egaSlot0){
		g_Quest_Slots = 0;
		for(i=0;i<g_QuestsOnPage;i++){
			core_CreateSprite("gui\\level_quest_slot.spr","level_quests",{x=0, y=7-2.5*i, z=-0.0002, w=1, h=1, name=format("quest_slot%i",g_Quest_Slots++) });
		}
	}
	local attrId=0;
	local attrByNum="";
	g_SkipQuest_Slots = skipItems;
	local quests = getOpenQuests();
	if(g_SkipQuest_Slots>quests.len())
	{
		g_SkipQuest_Slots = quests.len()-g_QuestsOnPage;
	}
	if(g_SkipQuest_Slots<0)
	{
		g_SkipQuest_Slots = 0;
	}
	for(i=0; i<g_Quest_Slots; i++)
	{
		local slotNode=core_GetNode(format("quest_slot%i",i));
		slotNode._alpha=1.0;
		local thisQuestNum = g_SkipQuest_Slots+i;
		if(thisQuestNum >= quests.len()){
			core_EnableNode(slotNode,false);
		}else{
			core_EnableNode(slotNode,true);
			core_SetNodeText(slotNode,format("%s",quests[thisQuestNum].locName));
			if(quests[thisQuestNum].state==1)
			{
				slotNode._alpha=0.5;
				//core_Alert("addfade!");
			}
			//core_SetNodeAction(slotNode,format("editAttrEditDialog(\"%s\",\"%s\");",thisActor.Name,attrByNum.name));
		}
		core_SetNode(format("quest_slot%i",i),slotNode);
	}
*/
	// Окно помощи при первом заходе
	local safe=game_GetSafe();
	if(!("techs_help_ok" in safe) || !safe.techs_help_ok){
		safe.techs_help_ok <- true;
		helpDialog("level_techs_help");
	}
}
