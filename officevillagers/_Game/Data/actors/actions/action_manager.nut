function eventManager(thisActor,eventsMask)
{
	// �������� ����� ��� ������ ���������� �� ����� � ������� �� �� ��� ���
	// ���� ��������� �� ������ ������
	// ����� ������ ������������� ����� ������ �� ���
	// ���� �������������� �� ����
	local actions=event_GetEvents({ _byName=eventsMask, _randomize=true });
	local i;
	for(i=0;i<actions.len();i++)
	{
		if(game_GetTickCount()-actions[i].LastActivationTime<10*1000){
			// ���-�� ������� ����������� �������...
			continue;
		}
		if(actions[i].UniqueUsage>0 && actions[i].ActionUsageCount>=actions[i].UniqueUsage){
			// � ��� ����� �����...
			continue;
		}
		// ����� ��� ��������!
		actor_SwitchToAction(thisActor,actions[i].Name);
		return true;
	}
	return false;
}

function oldestPers(a,b)
{
	if(a.CreationTime<b.CreationTime){//���� ����� �������� ������ - ���� ������
		return 1;
	}
	return -1;
}

function getOldestPersons()
{
	local i=0;
	local objectsArray=actor_GetActors( {_byType=1, _randomize=true });
	local res = [];
	for(i=0;i<objectsArray.len();i++){
		if(objectsArray[i].Profession != "AUTHOR" && objectsArray[i].Profession != "AUTHOR-EXPERT"
		&& objectsArray[i].Profession != "CREATOR" && objectsArray[i].Profession != "CREATOR-EXPERT"
		&& objectsArray[i].Profession != "FINANCIER" && objectsArray[i].Profession != "FINANCIER-EXPERT"){
			continue;
		}
		if(objectsArray[i].BodyLock>0){// �������� ����� �� �����
			continue;
		}
		res.append(objectsArray[i]);
	}
	res.sort(oldestPers);
	return res;
}
