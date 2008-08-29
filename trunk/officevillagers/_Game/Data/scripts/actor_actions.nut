function actor_SetMovementsPause(thisActor,isPause,checkSlot)
{
	thisActor = actor_GetActor(thisActor);
	if(!thisActor){
		return;
	}
	checkSlot = "pl_" + checkSlot;
	local res=false;
	local afterPauser=999;
	if(isPause){
		if(!(checkSlot in thisActor) || !thisActor[checkSlot]){
			afterPauser=actor_PauseMovements(thisActor,true);
			thisActor[checkSlot] <- true;
			res=true;
		}
	}else{
		if((checkSlot in thisActor) && thisActor[checkSlot]){
			thisActor[checkSlot] <- false;
			afterPauser=actor_PauseMovements(thisActor,false);
			res=true;
		}
	}
	//core_Warning(format("Pausing %s->%s [%s, afterc=%i], reason=%s",thisActor.Name,isPause?"pause":"unpause",res?"t":"f",afterPauser,checkSlot));
	return res;
}

function actor_NewPerson(templateName,showInterface)
{
	local newOne=actor_CreateActor(templateName);
	location_JumpActor(newOne);
	if(showInterface){
		openStagerInterfaceOnPerson(newOne);
	}
}

function gui_GetIssueProgressCur()
{
	return actor_GetAttributeN("Office","IssueProgressCur");//level_Progress(null,null);
}

function gui_GetIssueProgressMax()
{
	return actor_GetAttributeN("Office","IssueProgressMax");
}

function gui_SetIssueProgressMax(number)
{
	actor_SetAttribute("Office","IssueProgressMax", number);
	return level_Progress(null,number);
}

function gui_SetIssueProgressCur(number)
{
	actor_SetAttribute("Office","IssueProgressCur", number);
	return level_Progress(number,null);
}

function gui_GetTimeFraction()
{
	return level_Time();
}

function gui_SetCrunchFraction(number)
{
	return level_Crunch(number);
}

function gui_GetCrunchFraction()
{
	return level_Crunch();
}

function gui_IsCrunch()
{
	return level_Crunch(true);
}

function actor_GetActorEnabled(thisActor)
{
	return thisActor.Enabled;
}

function actor_GetActorName(thisActor)
{
	return thisActor.Name;
}

function actor_GetActorAction(thisActor)
{
	return thisActor.Action;
}

function actor_GetActorActorID(thisActor)
{
	return thisActor.ActorID;
}

function actor_GetActorProfession(thisActor)
{
	return thisActor.Profession;
}

function actor_GetActorEventManuality(thisActor)
{
	if(thisActor.ManualEvent==1){
		return true;
	}
	return false;
}

function actor_SetActorVisible(thisActor, isVisible)
{
	//core_Warning(format(">>> actor setVisible %s-%s",thisActor.Name,isVisible?"t":"f"));
	local actorNode=core_GetNode(thisActor.ActorID);
	core_EnableNode(actorNode,isVisible);
}

function actor_SetProfessionStager(thisActor,profName)
{
	actor_SetProfession_i(thisActor,profName,true);
}

function actor_SetProfessionRaw(thisActor,profName)
{
	thisActor=actor_GetActor(thisActor);
	if(actor_GetActorProfession(thisActor)==profName){
		// Не меняем!
		return false;
	}
	actor_UnsetTeacher(thisActor);
	actor_MakePocketEmpty(thisActor);
	return actor_SetProfession_i(thisActor,profName,false);
}

function actor_SetProfession(thisActor,profName)
{
	if(actor_SetProfessionRaw(thisActor,profName)){
		actor_SwitchToAction(thisActor,"");
	}
}

function actor_StartTimer(thisActor,timerName)
{
	thisActor[timerName] <- getTime();
}

function actor_GetTimer(thisActor,timerName)
{
	if(!(timerName in thisActor)){
		return 0.0;
	}
	return getTime()-thisActor[timerName];
}

function actor_GetActorsByProfAttribute(profMask,atrrName,attrVal)
{
	local objectsArray=actor_GetActors( {_byProfession=profMask});
	local i=0;
	local objectsArrayOut=[];
	if(atrrName && atrrName.len()>0){
		for(i=0;i<objectsArray.len();i++)
		{
			local attr=actor_GetAttribute(objectsArray[i],atrrName);
			if(!attr){
				attr=0;
			}
			if(attr==attrVal){
				//core_Warning("Found %s",objectsArray[i].Name);
				objectsArrayOut.append(objectsArray[i]);
			}
		}
	}else{
		objectsArrayOut=objectsArray;
	}
	if(objectsArrayOut.len()==0){
		return null;
	}
	local rndIndex=core_Rnd()*(objectsArrayOut.len());
	return objectsArrayOut[rndIndex.tointeger()];
}

function actor_GetActorsByAttribute(nameMask,atrrName,attrVal)
{
	local objectsArray=actor_GetActors( {_byName=nameMask});
	local i=0;
	local objectsArrayOut=[];
	if(atrrName && atrrName.len()>0){
		for(i=0;i<objectsArray.len();i++)
		{
			local attr=actor_GetAttribute(objectsArray[i],atrrName);
			if(!attr){
				attr=0;
			}
			if(attr==attrVal){
				//core_Warning("Found %s",objectsArray[i].Name);
				objectsArrayOut.append(objectsArray[i]);
			}
		}
	}else{
		objectsArrayOut=objectsArray;
	}
	if(objectsArrayOut.len()==0){
		return null;
	}
	local rndIndex=core_Rnd()*(objectsArrayOut.len());
	return objectsArrayOut[rndIndex.tointeger()];
}

function actor_GetActorsByAttribute2(nameMask,atrrName1,attrVal1,atrrName2,attrVal2)
{
	local objectsArray=actor_GetActors( {_byName=nameMask});
	local i=0;
	local objectsArrayOut=[];
	if((atrrName1 && atrrName1.len()>0) || (atrrName2 && atrrName2.len()>0)){
		for(i=0;i<objectsArray.len();i++)
		{
			local attr1=actor_GetAttribute(objectsArray[i],atrrName1);
			if(!attr1){
				attr1=0;
			}
			local attr2=actor_GetAttribute(objectsArray[i],atrrName2);
			if(!attr2){
				attr2=0;
			}
			if(attr1==attrVal1 && attr2==attrVal2){
				//core_Warning("Found %s",objectsArray[i].Name);
				objectsArrayOut.append(objectsArray[i]);
			}
		}
	}else{
		objectsArrayOut=objectsArray;
	}
	if(objectsArrayOut.len()==0){
		return null;
	}
	local rndIndex=core_Rnd()*(objectsArrayOut.len());
	return objectsArrayOut[rndIndex.tointeger()];
}

function actor_InitTalking(pers1, pers2, sTalkPoint)
{
	if(pers1.Name==pers2.Name){
		// Фигня... не дложно быть так
		$ifdebug core_Alert("talk wtf!-1");
		return;
	}
	if(sTalkPoint==""){
		$ifdebug core_Alert("talk wtf!-2");
		return;
	}
	if(actor_GetActor(pers1).Action=="Talking" || actor_GetActor(pers2).Action=="Talking"){
		// Они уже!
		$ifdebug core_Alert("talk wtf!-already");
		return;
	}
	actor_SetAttribute(pers1,"TalkPoint",sTalkPoint);// Куда идти
	actor_SetAttribute(pers2,"TalkPoint",sTalkPoint);// Куда идти
	actor_SetAttribute(pers1,"TalkPartner",pers2.Name);// С кем говорить
	actor_SetAttribute(pers2,"TalkPartner",pers1.Name);// С кем говорить
	actor_SetAttribute(pers1,"TalkInitiator",1);// Кто Инициатор
	actor_SetAttribute(pers2,"TalkInitiator",0);// Кто Инициатор
	actor_SwitchToAction(pers1,"Talking");
	actor_SwitchToAction(pers2,"Talking");
}