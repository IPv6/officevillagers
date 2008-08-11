// stdafx.cpp : source file that includes just the standard includes
//	JewelResonance.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"
#include "engineBindings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

void _RunMain()
{
	try {
		CGameImplementation* client=new CGameImplementation();
		RunGameLoop(client);
	}catch(CGameRotorError& error){
		GfxMessageBox(toString("Error occured!\n'%s'",error.getError()));
	}
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	BOOL bStarted=1;
#ifdef USE_CUSTOMCRASH_DLG
	//_CRTIMP _CRT_ALLOC_HOOK __cdecl _CrtSetAllocHook(_CRT_ALLOC_HOOK hookFunctionPtr);
	//set_unexpected(CustomUnexpectedHandler);
	//set_terminate(CustomUnexpectedHandler);
	//_set_se_translator( trans_func );//void trans_func( unsigned int u, EXCEPTION_POINTERS* pExp )
	__try{
		_RunMain();
	}__except(CustomUnexpectedFilter(GetExceptionCode(),GetExceptionInformation())){
		return 1;
	}
#else
	_RunMain();
#endif
	return 0;
}


/*// Make VC Express to build Win32

Step 3: Update the Visual C++ directories in the Projects and Solutions section in the Options dialog box.

Add the paths to the appropriate subsection:

    * Executable files: C:\Program Files\Microsoft Platform SDK for Windows Server 2003 R2\Bin
    * Include files: C:\Program Files\Microsoft Platform SDK for Windows Server 2003 R2\Include
    * Library files: C:\Program Files\Microsoft Platform SDK for Windows Server 2003 R2\Lib

Note: Alternatively, you can update the Visual C++ Directories by modifying the VCProjectEngine.dll.express.config file located in the \vc\vcpackages subdirectory of the Visual C++ Express install location. Please make sure that you also delete the file "vccomponents.dat" located in the "%USERPROFILE%\Local Settings\Application Data\Microsoft\VCExpress\8.0" if it exists before restarting Visual C++ Express Edition.

Step 4: Update the corewin_express.vsprops file.

One more step is needed to make the Win32 template work in Visual C++ Express. You need to edit the corewin_express.vsprops file (found in C:\Program Files\Microsoft Visual Studio 8\VC\VCProjectDefaults) and

Change the string that reads:

AdditionalDependencies="kernel32.lib"

  to

AdditionalDependencies="kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib"

Step 5: Generate and build a Win32 application to test your paths.

In Visual C++ Express, the Win32 Windows Application type is disabled in the Win32 Application Wizard. To enable that type, you need to edit the file AppSettings.htm file located in the folder “%ProgramFiles%\Microsoft Visual Studio 8\VC\VCWizards\AppWiz\Generic\Application\html\1033\".

In a text editor comment out lines 441 - 444 by putting a // in front of them as shown here:

// WIN_APP.disabled=true;
// WIN_APP_LABEL.disabled=true;
// DLL_APP.disabled=true;
// DLL_APP_LABEL.disabled=true;

Save and close the file and open Visual C++ Express.

From the File menu, click New Project. In the New Project dialog box, expand the Visual C++ node in the Product Types tree and then click Win32. Click on the Win32 Console Application template and then give your project a name and click OK. In the Win32 Application Wizard dialog box, make sure that Windows application is selected as the Application type and the ATL is not selected. Click the Finish button to generate the project.

As a final step, test your project by clicking the Start button in the IDE or by pressing F5. Your Win32 application should build and run. 
*/