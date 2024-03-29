function actorWait_GoTo(thisActor,locationName)
{
	if(actor_SendToLocation(thisActor,locationName)!=true){
		$ifdebug {core_Alert(format("Goto failed!!! %s to %s",actor_GetActor(thisActor).Name,locationName.tostring()));game_SetGameSpeed(0);b=b123/b;};
		actor_SwitchToAction(thisActor,"",true);// �� ����������
		::suspend();
		return false;
	}
	local lStartTime=getTime();
	// ���� ����� ������ �� ���� � ������� - ���� ���������! ��� ��������� ��� ���������� - ������� ����� �������������� ���������� �������� �����
	while(actor_IsMovingToLocation(thisActor,locationName)==true && actor_IsInLocation(thisActor,locationName)==false){
		::suspend();
		if(getTime()-lStartTime>500){
			// ��� ������ �� ����������� ���� ���� �� ����� � ������� ���� ����� ������� � ���� - ���������� ��� ���, ������! �� �����
			$ifdebug if(!game_IsFastForward()){core_Alert(format("Goto failed (too long)!!! %s to %s",actor_GetActor(thisActor).Name,locationName.tostring()));game_SetGameSpeed(0);};
			$ifdebug $ifdebug __DebugActor(thisActor,"walk drop to nones");
			actor_SwitchToAction(thisActor,"",true);// �� ����������
			::suspend();
			return false;
		}
	}
	return true;
}

// ���� ���� ��������� ��...
function actorWait_WhileProfNotIn(thisActor,profName)
{
	while(actor_GetActor(thisActor).Profession!=profName){
		::suspend();
	}
}

function actorWait_WaitToActionStart(thisActor,actionName)
{
	local ActorName=thisActor.Name;
	while(actor_GetActor(ActorName).Action != actionName){
		::suspend();
	}
}

function actorWait_SwitchToAction(thisActor,actionName)
{
	actor_CancelWalking(thisActor);
	actor_SwitchToAction(thisActor,actionName);
	actorWait_WaitToActionStart(thisActor,actionName);
}

// ���� ������������ ����� ������� ������ (�� ���������� �������� - � ��� ���� ������)
function actorWait_Timeout(timeout)
{
	local curTime=game_GetTickCount();
	while(curTime+timeout*1000 > game_GetTickCount()){
		::suspend();
	}
}

function ReplaceOfficeAttrs(str,attrs)
{
	local i;
	for(i=0;i<attrs.len();i++){
		str = core_StrReplace(str,"@"+attrs[i],actor_GetAttributeS("Office",attrs[i]));
	}
	return str;
}