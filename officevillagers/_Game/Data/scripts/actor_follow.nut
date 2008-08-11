function actor_GetFollower(thisActor)
{
	local isFollower=actor_GetAttribute(thisActor,"FOLLOWER");
	if(isFollower && isFollower.len()>0){
		return isFollower;
	}
	return null;
}

function actor_GetTeacher(thisActor)
{
	local isTeacher=actor_GetAttribute(thisActor,"TEACHER");
	if(isTeacher && isTeacher.len()>0){
		return isTeacher;
	}
	return null;
}

function actor_SetTeacher(thisActor,teacherActor,bAdd)
{
	if(bAdd){
		thisActor = actor_GetActor(thisActor);
		teacherActor = actor_GetActor(teacherActor);
		local teacher2Name=actor_GetAttributeS(thisActor,"TEACHER");
		if(teacher2Name.len()>0){// Мы уже привязаны к комуто
			actor_UnsetTeacher(thisActor);
		}
		if(actor_SetFollowerMode(thisActor,teacherActor)){
			actor_AddAttribute(thisActor,"TEACHER",teacherActor.Name);
			actor_AddAttribute(teacherActor,"FOLLOWER",thisActor.Name);
		}
	}else{
		thisActor = actor_GetActor(thisActor);
		if(actor_SetFollowerMode(thisActor,null)){
			local teacherName=actor_GetAttributeS(thisActor,"TEACHER");
			actor_AddAttribute(thisActor,"TEACHER","");
			if(teacherName!=""){
				actor_AddAttribute(teacherName,"FOLLOWER","");
			}
		}
	}
}

function actor_UnsetTeacher(thisActor)
{
	actor_SetTeacher(thisActor,null,false);
}