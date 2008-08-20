function quest_conditions(conditions)
{
	if("_fromDrop" in conditions)
	{// Проверки квестов, вызывающиеся из дроп-методов
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
	{// Из дропа на локацию
		if((actor_GetActorProfession(conditions._actorFrom)=="JANITOR" || actor_GetActorProfession(conditions._actorFrom)=="JANITOR-EXPERT")
		&& conditions._locName == "FurniDrops::BARRICADA"
		&& nloc_CheckWalkablePath("FurniDrops::BARR_FROM","Heaps.Barricada")!=false){
			if(gui_QuestClose(6)){
				gui_StartCutscene("Quest7_Cutscene");
			}
		}
	}
	
	if("_fromSetProf" in conditions){// Проверки квестов, вызывающиеся из смиены профессии
		if (conditions.who.Name=="ACTOR.AGATA" && conditions.prof.find("FINANCIER")!= null ){
			if(gui_QuestClose(2)){
				gui_QuestOpen(3);
			}
		}
	}
	
	if("Quest4" in conditions)
	{// четвертый квест
		if(gui_QuestClose(4)){
			gui_StartCutscene("Quest4_Cutscene");
		}
    }
}