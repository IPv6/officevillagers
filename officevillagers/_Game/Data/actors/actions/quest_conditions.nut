function quest_conditions(conditions)
{
	if("_fromDrop" in conditions)
	{// �������� �������, ������������ �� ����-�������
		//core_Warning("From drop!");
		if (conditions._actorFrom.BodyLock!=1 && conditions._actorTo.Name=="ACTOR.AGATA"){
			if(gui_QuestClose(3)){
				gui_QuestOpen(4);
			}
		}
		//core_Warning("QUEST!!! actor="+conditions._actorTo.Name+"; actor2="+conditions._actorFrom.Name);
		if ((conditions._actorFrom.Name=="ACTOR.BOB" && conditions._actorTo.Name=="ACTOR.AGATA")||(conditions._actorFrom.Name=="ACTOR.AGATA" && conditions._actorTo.Name=="ACTOR.BOB")){
			if(gui_QuestClose(5)){
				gui_StartCutscene("Quest5_Cutscene");
			}
		}
	}
	
	if("_fromDropLocation" in conditions)
	{// �� ����� �� �������
	}
	
	if("_fromDropTrash" in conditions)	
	{//���� �� �����
		if((actor_GetActorProfession(conditions._actorFrom)=="JANITOR" || actor_GetActorProfession(conditions._actorFrom)=="JANITOR-EXPERT")
		&& conditions._locName.Name == "Heaps.Barricada" 
		&& nloc_CheckWalkablePath("Furniture::DOT16","FurniDrops::BARRICADA!")!=false){
		core_Warning("2");
			if(gui_QuestClose(7)){
			core_Warning("3");
				gui_StartCutscene("Quest7_Cutscene");
			}
		}
	}
	
	if("_fromSetProf" in conditions){// �������� �������, ������������ �� ������ ���������
		if (conditions.who.Name=="ACTOR.AGATA" && conditions.prof.find("FINANCIER")!= null ){
			if(gui_QuestClose(2)){
				gui_QuestOpen(3);
			}
		}
	}
	
	if("Quest4" in conditions)
	{// ��������� �����
		if(gui_QuestClose(4)){
			gui_StartCutscene("Quest4_Cutscene");
		}
    }
	if("_fromTech" in conditions)//�������� �������, ���������� �� ������ ����������
	{
	if(conditions.newTech=="MUSCLES"){
		if(gui_QuestClose(8)){
			actor_SetAttribute(actor_GetActor("Heaps.Barricada"),"__BUSY",0);
			gui_StartCutscene("Quest8_Cutscene");
			}		
		}
	}

	if("_fromDelHeap" in conditions)//�������� �������, ���������� ��� �������� ���� ������
	 {
		if (conditions.heapName=="Heaps.Barricada" && gui_QuestClose(9)){
			gui_StartCutscene("Alpha_Last_Cutscene");
			}
	 }
	}














