// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__714F9F45_E10B_456C_A98A_306C71B3CEB2__INCLUDED_2)
#define AFX_STDAFX_H__714F9F45_E10B_456C_A98A_306C71B3CEB2__INCLUDED_2

#define chSTR2(x)	#x
#define chSTR(x)	chSTR2(x)
#define TODO(desc)	message(__FILE__ "(" chSTR(__LINE__) ")://TODO:" #desc)
#define ALLOWINLINE	__forceinline
#define DEPRECATIONS_CHECK
//#define MY_TIME_STAMP __TIMESTAMP__

#endif
