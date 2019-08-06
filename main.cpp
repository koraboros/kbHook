/***********************************************************
** kbhook.cpp                                              **
**                                                        **
**  Copyright 2009 koraboros@gmail.com,                   **
**   Free Usage granted to the public domain.             **
**                                                        **
***********************************************************/

#ifndef WINVER
#define WINVER 0x0501
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif                        
#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0410
#endif
#ifndef _WIN32_IE
#define _WIN32_IE 0x0600
#endif

#include "Windows.h"
#include "winuser.h"

#define _EXIT_KBHOOK_        0xff

typedef struct SVKMap 
{
    BYTE bVkToReplace;
    BYTE bVkReplaceWith;
}*PSVKMap;

int _iVKMapElemCnt = 0;
int _iVKMapElemSize = 0;

// define here key mapping, it's only thing you have to do
// VK_KEY macros are defined in winuser.h
SVKMap _aVKMap[] =
{
    {VK_BROWSER_FORWARD, VK_VOLUME_UP},
    {VK_LAUNCH_MAIL, VK_VOLUME_UP},
    {VK_BROWSER_BACK, VK_VOLUME_DOWN},
    {VK_BROWSER_SEARCH, VK_VOLUME_DOWN},
    {VK_BROWSER_HOME, VK_VOLUME_MUTE},
    {VK_BROWSER_REFRESH, _EXIT_KBHOOK_}
};

int CompareVKMap( const void* pVkMap1, const void* pVkMap2 )
{
    return ((PSVKMap)pVkMap1)->bVkToReplace - ((PSVKMap)pVkMap2)->bVkToReplace;
}

LRESULT CALLBACK LowLevelKeyboardProc( int nCode,
                                       WPARAM wParam,
                                       LPARAM lParam)
{
    if (nCode == HC_ACTION )
    {
        PSVKMap pVKMap = (PSVKMap)bsearch( &((PKBDLLHOOKSTRUCT)lParam)->vkCode, _aVKMap, _iVKMapElemCnt, _iVKMapElemSize, CompareVKMap );
                
        if( pVKMap != NULL )
        {
            if( pVKMap->bVkReplaceWith == _EXIT_KBHOOK_ )
            {
                PostQuitMessage( 0 );
            }
            else
            {
                keybd_event( pVKMap->bVkReplaceWith, 0, wParam == WM_KEYUP || wParam == WM_SYSKEYUP ? KEYEVENTF_KEYUP : 0, 0 );
            }
            return -1;
        }
    }

    return CallNextHookEx( NULL, nCode, wParam, lParam );
}


int WINAPI WinMain( HINSTANCE hinstExe, 
                    HINSTANCE hPrevInstance,
                    PTSTR pszCmdLine,
                    int iCmdShow )
{
    
    MSG sMSG;
    int iRet = 0;

    _iVKMapElemSize = sizeof(_aVKMap[0]);
    _iVKMapElemCnt = sizeof(_aVKMap) / _iVKMapElemSize ;
    qsort( _aVKMap, _iVKMapElemCnt, _iVKMapElemSize, CompareVKMap );

    HHOOK hhkLowLevelKybd = SetWindowsHookEx( WH_KEYBOARD_LL,
                                                LowLevelKeyboardProc,
                                                hinstExe,
                                                NULL );

    if( hhkLowLevelKybd != NULL )
    {
        while( (iRet = GetMessage( &sMSG, 0, 0, 0 ) ) != 0 )
        { 
            if( iRet == -1 )
            {
                PostQuitMessage( GetLastError() );
                break;
            }
            else
            {
                TranslateMessage(&sMSG); 
                DispatchMessage(&sMSG); 
            }
        }

        if( !UnhookWindowsHookEx( hhkLowLevelKybd ) )
        {
            sMSG.wParam = GetLastError();
        }
    }
    else
    {
        sMSG.wParam = GetLastError();    
    }

    return( (int)sMSG.wParam );
}
