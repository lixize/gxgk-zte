// Luzj_ZTE.h : main header file for the LUZJ_ZTE application
//

#if !defined(AFX_LUZJ_ZTE_H__B5301871_D3FA_47F7_BB26_279E58038108__INCLUDED_)
#define AFX_LUZJ_ZTE_H__B5301871_D3FA_47F7_BB26_279E58038108__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CLuzj_ZTEApp:
// See Luzj_ZTE.cpp for the implementation of this class
//

class CLuzj_ZTEApp : public CWinApp
{
public:
	CLuzj_ZTEApp();

	BOOL CheckCommandLine();
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLuzj_ZTEApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CLuzj_ZTEApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LUZJ_ZTE_H__B5301871_D3FA_47F7_BB26_279E58038108__INCLUDED_)
