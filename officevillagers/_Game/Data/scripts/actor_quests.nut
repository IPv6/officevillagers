//0 (такого квеста нет)
//1 (еще не открыт)
//2 (открыт)
//3 (закрыт)
function gui_QuestCheck(questNum)
{
	local safe=game_GetSafe();
	if(!("quests" in safe)){
		safe.quests <- {};
	}
	local quest=quest_GetQuest(questNum);
	if(!(quest.ID in safe.quests)){
		// Квест еще не был открыт
		safe.quests[quest.ID] <- {};
		safe.quests[quest.ID].ID <- quest.ID;
		safe.quests[quest.ID].State <- 1;
		safe.quests[quest.ID].OpenTime <- 0;
		safe.quests[quest.ID].CloseTime <- 0;
		safe.quests[quest.ID].highlighted <- false;
		game_MarkSafe();
	}
	return safe.quests[quest.ID].State;
}

function quest_set(questNum,state)
{
	gui_QuestCheck(questNum);// Обновляем все данные
	local quest=quest_GetQuest(questNum);
	local safe=game_MarkSafe();
	safe.quests[quest.ID].State <- state;
}

function gui_QuestPreOpen(questNum)
{
	local questState=gui_QuestCheck(questNum);
	local quest=quest_GetQuest(questNum);
	if(questState != 1){
		return false;
	}
	quest_set(questNum,1);
	return true;
}

function gui_QuestOpen(questNum)
{
	local questState=gui_QuestCheck(questNum);
	local quest=quest_GetQuest(questNum);
	if(questState != 1 && quest.AllowReopen==0){
		core_Alert(format("Error trying to open quest %i (state = %i)",questNum,questState));
		return false;
	}
	//core_Alert(format("quest %i state %i",questNum,questState));
	quest_set(questNum,2);
	local safe=game_GetSafe();
	safe.quests[quest.ID].OpenTime <- level_TimeAbs();
	addQuestsHighlight();
	gui_QuestHighlight(questNum);
	return true;
}

function gui_QuestClose(questNum)
{
	local office=actor_GetActor("Office");
	local questState=gui_QuestCheck(questNum);
	local quest=quest_GetQuest(questNum);
    if(questState != 2){
        //core_Alert(format("Error trying to open quest %i (state = %i)",questNum,questState));
        return false;
    }
	addQuestsHighlight();
	quest_set(questNum,3);
	local safe=game_GetSafe();
	safe.quests[quest.ID].CloseTime <- level_TimeAbs();
	return true;
}

function gui_QuestHighlight(questNum)
{
	addQuestsHighlight();
	gui_QuestCheck(questNum);// Обновляем все данные
	local safe=game_GetSafe();
	local quest=quest_GetQuest(questNum);
	safe.quests[quest.ID].highlighted = true;
}

g_QuestCache <- {};
g_questNames <- "";
function quest_GetQuest(questNum)
{
	local questName;
	if(questNum.tostring().find("quest_") != null){
		questName=questNum;
	}else{
		questName = "quest_"+questNum.tostring();
	}
	if( questName in g_QuestCache){
		return g_QuestCache[questName];
	}
	if(g_questNames == ""){
		g_questNames=core_GetDataPath("\\text\\quest_names.lng");
	}
	// Читаем из файла
	local lQuestFile=core_GetDataPath("\\actors\\quests\\"+questName+".txt");
	local lQuestContent=core_ReadTextFile(lQuestFile);
	g_QuestCache[questName] <- {};
	local Name = core_GetSprIniParameter("Name",lQuestContent);
	if(Name.len()==0)
	{
		core_Alert(format("Undeclared quest used!!! %s",questName));
		return null;
	}
	g_QuestCache[questName].ID <- questName;
	g_QuestCache[questName].Name <- core_Translate(Name,g_questNames);
	//core_Alert(Name+";l "+g_questNames+"\n"+g_QuestCache[questName].Name);
	g_QuestCache[questName].AllowReopen <- core_GetSprIniParameter("Reopen",lQuestContent).tointeger();
	g_QuestCache[questName].SortId <- core_GetSprIniParameter("SortId",lQuestContent).tointeger();
	g_QuestCache[questName].UsedAttributes <- ::split(core_GetSprIniParameter("UsedAttributes",lQuestContent),",; ");
	return g_QuestCache[questName];
}

function addQuestsHighlight()
{
	clearQuestsHighlight(true);
	core_CreateSprite("gui\\buttons\\quest_button_hightlight.spr","quests_button",{z=-0.0001, name="quests_highlight"});
}

function clearQuestsHighlight(bFast)
{
	if(bFast){
		core_DeleteSprite("quests_highlight");
	}else{
		core_FadeSprite("quests_highlight",1.0,0.0,400,1);
	}
}

function level_OpenQuests(param)
{
	if(floorAttachInterval != 0 || floorAutoDragInterval != 0
		|| humanDragInterval != 0){
		// У нас идет скрипт... нельзя
		return;
	}
	clearQuestsHighlight(false);
	gui_ShowDialog(null,null);
	core_OpenDialog("level_quests");
}

function level_CloseQuests()
{
	if(core_IsDialogOpened("level_quests")){
		core_CloseDialog("level_quests");
	}
}

function quest_timecompare(a,b)
{
     if(a.sortId>b.sortId){
          return 1;
     }else if(a.sortId<b.sortId){
          return -1;
     }
     return 0;
}

function getOpenQuests()
{
	local i=0;
	local quests=[];
	local safe=game_GetSafe();
	if(!("quests" in safe)){
		return quests;
	}
	//local str=core_SerializeObj(safe.quests);core_Alert(str);
	foreach(questSt in safe.quests){
		if("State" in questSt && (questSt.State==2))//Берем только открытые, без || questSt.State==1
		{
			local questID=questSt.ID;
			//core_Alert(questID+" "+questSt.State.tostring());
			local quest=quest_GetQuest(questID);
			local questItem = {};
			questItem.questNum <- questID;
			questItem.locName <- quest.Name;
			questItem.locName = ReplaceOfficeAttrs(questItem.locName,quest.UsedAttributes);
			questItem.state <- safe.quests[questID].State;
			questItem.sortId <- /*safe.quests[questID].OpenTime*100+*/quest.SortId;
			//questItem.locName+=" "+questItem.openTime.tostring();
			questItem.highlighted <- safe.quests[questID].highlighted;
			quests.append(questItem);
		}
	}
	quests.sort(quest_timecompare);
	return quests;
}

g_Quest_Slots <- 0;
g_SkipQuest_Slots <- 0;
g_SkipQuest_Slots_This <- 0;
g_SkipQuest_Slots_Prev <- [0];
g_QuestsOnPage <- 5;
function initQuestDialog(skipItems)
{
	local safe=game_GetSafe();
	local i=0;
	g_Quest_Slots = 0;
	for(i=0;i<g_QuestsOnPage;i++){
		local slotNodeName=format("quest_slot%i",g_Quest_Slots);
		core_DeleteNode(slotNodeName);
		core_CreateSprite("gui\\level_quest_slot.spr","level_quests",{x=-0.5, y=7-2.5*i, z=-0.0002, w=1, h=1, name=slotNodeName });
		g_Quest_Slots++;
	}
	local attrId=0;
	local attrByNum="";
	if(skipItems!=g_SkipQuest_Slots_Prev[g_SkipQuest_Slots_Prev.len()-1]){
		g_SkipQuest_Slots_Prev.append(skipItems);
	}
	g_SkipQuest_Slots = skipItems;
	g_SkipQuest_Slots_This = skipItems;
	local quests = getOpenQuests();
	if(g_SkipQuest_Slots<0 || g_SkipQuest_Slots>quests.len())
	{
		g_SkipQuest_Slots = 0;
	}
	local multiLineOffset=0;
	local lLineHH=1.15;
	local slotId=0;
	local bt_next=false;
	local bt_prev=skipItems>0?true:false;
	for(slotId=0; slotId<g_Quest_Slots; slotId++)
	{
		local slotName=format("quest_slot%i",slotId);
		local slotNode=core_GetNode(slotName);
		slotNode._alpha=1.0;
		local thisQuestNum = g_SkipQuest_Slots+slotId;
		local nodeText="";
		if(thisQuestNum >= quests.len()){
			core_EnableNode(slotNode,false);
		}else{
			core_EnableNode(slotNode,true);
			local questtitle=quests[thisQuestNum].locName;
			questtitle=core_JustifyText(questtitle,4.8);
			slotNode._y=9-multiLineOffset;//ДО вычисления числа строк данного квеста!!!
			// Определяем число строк
			local lStrokNum=charsInText(questtitle,"\n");
			slotNode._h=lLineHH*(lStrokNum+1);
			multiLineOffset+=lLineHH*(lStrokNum+1);
			multiLineOffset+=0.3;
			if(multiLineOffset>19.0){
				// Кончилось
				bt_next = true;
				core_EnableNode(slotNode,false);
				g_SkipQuest_Slots = thisQuestNum;
				break;
			}
			slotNode._y-=slotNode._h*0.5;
			local dy=0.5*lStrokNum;
			nodeText=format("<font-size:-1><dy=%.02f><dx=-12.5><left>\n%s",dy.tofloat(),questtitle);
			core_SetNodeText(slotNode,nodeText);
			if(quests[thisQuestNum].highlighted)
			{
				safe.quests[quests[thisQuestNum].questNum].highlighted = false;
				core_CreateSprite("gui\\level_quest_slot_hl.spr",slotName,{x=0, y=0, z=-0.1, w=1, h=1});
			}
			//core_SetNodeAction(slotNode,format("editAttrEditDialog(\"%s\",\"%s\");",thisActor.Name,attrByNum.name));
		}
		core_SetNode(slotName,slotNode);
	}
	core_EnableNode("quest_button_left",bt_prev);
	core_EnableNode("quest_button_right",bt_next);
	
	// Окно помощи при первом заходе
	if(core_GetProfileOption("TutorialQuestHelp")!=1){
		core_SetProfileOption("TutorialQuestHelp",1);
		helpDialog("level_quests_help");
	}
}

function openQuestsUI()
{
	g_SkipQuest_Slots <- 0;
	g_SkipQuest_Slots_This <- 0;
	g_SkipQuest_Slots_Prev <- [0];
	initQuestDialog(0);
}

function quest_left()
{
	local prev = 0,i=0;
	for(i=0;i<g_SkipQuest_Slots_Prev.len();i++)
	{
		if(g_SkipQuest_Slots_Prev[i]<g_SkipQuest_Slots_This){
			prev=g_SkipQuest_Slots_Prev[i];
		}
	}
	//core_Alert(g_SkipQuest_Slots.tostring());//dump(g_SkipQuest_Slots_Prev));
	initQuestDialog(prev);
}

function quest_right()
{
	initQuestDialog(g_SkipQuest_Slots);
}


g_cnt <- 0;
function gui_GenerateHint()
{
	local alarmsFile = core_GetDataPath("\\text\\alarm_names.lng");
	local hintName=format("HELPHINT%i",core_Rnd(1,21));
	gui_ShowHint(core_Translate(hintName,alarmsFile),10,0);
}















