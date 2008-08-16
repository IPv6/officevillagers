function runActorStepInThread(thisActor)
{
	if(thisActor._StepInThread==0){
		//core_Alert(StepIn[thisActor._StepInThreadMethod]);
		$ifdebug __DebugActor(thisActor,thisActor.Name+": calling "+thisActor._StepInThreadMethod);
		thisActor._StepInThread=::newthread(StepIn[thisActor._StepInThreadMethod]);
		thisActor._StepInThread.call(thisActor);
		return;
	}
	if(thisActor._StepInThread.getstatus()=="suspended"){
		//core_Warning(thisActor._StepInThread);
		$ifdebug __DebugActor(thisActor,thisActor.Name+": waking up "+thisActor._StepInThreadMethod);
		thisActor._StepInThread.wakeup();
		return;
	}
	// ���� ���� ������� ������, �� � ������ ����� ������ idle
	// getstatus -> ("idle","running","suspended")
	// ������������� � ���� �������� ������ �� ������, ������ ������������ �������
	return;
}

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
			$ifdebug {core_Alert(format("Goto failed (too long)!!! %s to %s",actor_GetActor(thisActor).Name,locationName.tostring()));game_SetGameSpeed(0);};
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