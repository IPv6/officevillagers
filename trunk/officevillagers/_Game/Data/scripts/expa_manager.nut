function addExpa(thisActor, howMany)
{
	local prof2set=thisActor.ProfessionN;
	local teacher=actor_GetTeacher(thisActor);
	if(teacher){
		teacher=actor_GetActor();
		prof2set=teacher.ProfessionN;
	}
	actor_SetAttribute(thisActor, format("exp_%s", prof2set), actor_GetAttributeN(thisActor,format("exp_%s", prof2set)+howMany));
}

function getExpaOnProf(thisActor,profN)
{
	return actor_GetAttributeN(thisActor, format("exp_%s", profN));
}

function getExpa(thisActor)
{
	local teacher=actor_GetTeacher(thisActor);
	if(teacher){
		teacher=actor_GetActor();
		return getExpaOnProf(thisActor, teacher.ProfessionN);
	}
	return getExpaOnProf(thisActor, thisActor.ProfessionN);
}

function isProfExpaOk(thisActor)
{
	if(getExpa(thisActor)>=getGlobal("StagerExpMax")){
		return true;
	}
	return false;
}

function isProfExpaOnProfOk(thisActor,profN)
{
	if(getExpaOnProf(thisActor,profN)>=getGlobal("StagerExpMax")){
		return true;
	}
	return false;
}

