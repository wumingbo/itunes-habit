#pragma once


#include "iTunesAPI.h"
#include "iTunesVisualAPI.h"

#include "SQLiteLog.h"


#include <ctime>
#include <string>
#include <vector>
#include <iostream>

using namespace std;


class CVisualPlugin {
public:

    CVisualPlugin(const string & database) : m_dbfilename(database)
    {
        cout << __FUNCTION__ << "\n";
    }

    ~CVisualPlugin()
    {
        cout << __FUNCTION__ << "\n";
    }

    ITAppProcPtr	proc;
    ITTrackInfoV1	trackInfo;
    ITStreamInfoV1	streamInfo;
    ITTrackInfo     trackUniInfo;
    ITStreamInfo    streamUniInfo;

    void *			cookie;
    Boolean	        playing;
    string          m_dbfilename;

};


unsigned int seed();
bool NormalizeCurrentDirectory();

static OSStatus VisualPluginHandler(OSType message,
                                    VisualPluginMessageInfo * messageInfo,
                                    void * refCon);


BOOL CALLBACK DlgProc(HWND dlg,
                      UINT message,
                      WPARAM wParam,
                      LPARAM lParam);

BOOL WINAPI DllMain(HINSTANCE /* instance */,
                    DWORD reason,
                    LPVOID /* reserved */);


extern "C" __declspec(dllexport) OSStatus iTunesPluginMain(
    OSType message,
    PluginMessageInfo * messageInfo,
    void * /* refCon */);
