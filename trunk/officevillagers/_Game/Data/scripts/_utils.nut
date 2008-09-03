function strstr(strIn,strWhat)
{
	/*
	local finder=regexp(strWhat);
	local indexes=finder.search(strIn);
	return indexes != null? true:false;
	*/
	local markPos=strIn.find(strWhat);
	return markPos?true:false;
}

function min(n1,n2)
{
	if(n1<n2){
		return n1;
	}
	return n2;
}

function max(n1,n2)
{
	if(n1>n2){
		return n1;
	}
	return n2;
}

function sgn(n1)
{
	if(n1==0){
		return 0;
	}
	if(n1<0){
		return -1;
	}
	return 1;
}

function escapePath(path)
{
	// заменяет слеши на двойные чтобы можно было передавать пути через строки
	local res="";
	local i=0;
	for(i=0;i<path.len();i++)
	{
		if(path[i]=='\\')
		{
			res+="\\\\";
		}else{
			res+=path[i].tochar();
		}
	}
	return res;
}

function wait(millisToWait)
{
	local WaitTime = core_GetTick();
	while(core_GetTick()-WaitTime<millisToWait){
		::suspend();
	}
};

function nope(params)
{
	// Do nothing! :)
};

function isInArray(inarray,what)
{
	local i=0;
	for(i=0;i<inarray.len();i++){
		if(inarray[i]==what){
			return true;
		}
	}
	return false;
}

function randomizeArray(arrayList)
{
	local i=0,arrlen=arrayList.len();
	local newPlace=0;
	for(i=0;i<arrlen-1;i++)
	{
		newPlace=core_Rnd(0,arrlen).tointeger();
		local tmp=arrayList[i];
		arrayList[i]=arrayList[newPlace];
		arrayList[newPlace]=tmp;
	}
	//local str="";	for(i=0;i<arrayList.len();i++){str+=arrayList[i].tostring()+" ";}core_Alert(str);
}

function compareAskLists(a,b)
{
	if(a.Name<b.Name){
		return 1;
	}
	return -1;
}

function calcDistance2(p1,p2)
{
	return (p1._x-p2._x)*(p1._x-p2._x)+(p1._y-p2._y)*(p1._y-p2._y);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
function utils_SetIntervalThreadLoop(Fiber)
{
	// Возобновляем поток пока он не закончится
	if(Fiber.getstatus()=="suspended"){
		Fiber.wakeup();
		return;
	}
	core_CancelInterval();
}

function utils_SetIntervalThread(inteval,method)
{
	local Fiber = ::newthread(method);
	Fiber.call();
	core_SetInterval(inteval,"utils_SetIntervalThreadLoop",Fiber);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

function charsInText(text,charsingl)
{
	local i=0,res=0;
	for(i=0;i<text.len();i++){
		//core_Alert("*"+text[i].tostring()+"*");
		if(text[i]==charsingl[0]){
			res++;
		}
	}
	return res;
}

function dump(var)
{
	local res="";
	local type=typeof(var);
	if(type=="array" || type=="table"){
		res+=type+"{";
		foreach(name,val in var){
			res+="\n";
			res+=name+"="+dump(val);
		}
		res+="\n};";
		return res;
	}
	if(type!="float" && type!="integer" && type!="bool"){
		res+="'["+type;
		if(!var){
			res+="-"+var.tostring()+"'";
		}
		res+="]';";
	}else{
		res+="'"+var.tostring()+"';";
	}
	return res;
}

/*
Tokenize!!!
        foreach( i,val in ::split(str,",;."))
        {
                ::print(val + "\n");
        }
*/