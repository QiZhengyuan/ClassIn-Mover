/*
    ClassIn Mover - A program to move ClassIn classroom window in order to
    exit from focused learning mode.
    Copyright (C) 2020  Weiqi Gao, Jize Guo

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <cstdio>
#include <cstring>
#include <ctime>
#include <windows.h>
#include <tlhelp32.h>
#include <Psapi.h>
#pragma comment (lib, "Psapi.lib")

HWND ClassroomHwnd;
bool FoundWindow;

BOOL CALLBACK RefreshWindow(HWND hwnd, LPARAM Title)
{
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(pe32);
    time_t tmp = time(0);
    tm *now = localtime(&tmp);
    TCHAR caption[1024];
    memset(caption, 0, sizeof(caption));
    GetWindowText(hwnd, caption, 1024);
    DWORD pid;
    if(strlen(caption) && strstr(caption, "Classroom_") && !FoundWindow)
    {
        char FormattedTime[9];
        strftime(FormattedTime, 9, "%H:%M:%S", now);
        GetWindowThreadProcessId(hwnd, &pid);
        HANDLE hProcessSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if(hProcessSnap == INVALID_HANDLE_VALUE)
        {
            printf("[%s] Failed to excute CreateToolhelp32Snapshot\n", FormattedTime);
            return TRUE;
        }
        BOOL bMore = ::Process32First(hProcessSnap, &pe32);
        bool flag = false;
        while(bMore)
        {
            if(DWORD(pe32.th32ProcessID) == pid)
            {
                flag = true;
                break;
            }
            bMore = ::Process32Next(hProcessSnap, &pe32);
        }
        char path[MAX_PATH];
        memcpy(path, pe32.szExeFile, MAX_PATH);
        strlwr(path);
        if(!flag || !strstr(path, "classin"))
            return TRUE;
        printf("[%s] Classroom window is found: HWND=%d title=%s\n", FormattedTime, hwnd, caption);
        ClassroomHwnd = hwnd;
        FoundWindow = true;
    }
    return TRUE;
}

void SleepUntilNext(int ms) {
    timeval tmp;
    mingw_gettimeofday(&tmp, NULL);
    long long now = tmp.tv_sec * 1000 + tmp.tv_usec / 1000;
    Sleep(ms - now % ms);
}

int main()
{
    puts("ClassIn Mover v1.0.1");
    puts("Copyright (C) 2020  Weiqi Gao, Jize Guo");
    puts("Visit https://github.com/CarlGao4/ClassIn-Mover for more information.\n");
    RECT rect;
    HWND hwnd = GetDesktopWindow();
    GetWindowRect(hwnd, &rect);
    WINDOWPLACEMENT wp;
    wp.length = sizeof(WINDOWPLACEMENT);
    while(true)
    {
        FoundWindow = false;
        EnumWindows(RefreshWindow, NULL);
        time_t tmp = time(0);
        tm *now = localtime(&tmp);
        if(!FoundWindow)
        {
            char FormattedTime[9];
            strftime(FormattedTime, 9, "%H:%M:%S", now);
            printf("[%s] Cannot find classroom window\n", FormattedTime);
            SleepUntilNext(1000);
            continue;
        }
        for(int i = 0; i < 10; i++)
        {
            GetWindowRect(ClassroomHwnd, &rect);
            SetWindowPos(ClassroomHwnd, HWND_NOTOPMOST, rect.left, rect.top,
                rect.right - rect.left, rect.bottom - rect.top, SWP_NOSENDCHANGING);
            GetWindowPlacement(ClassroomHwnd, &wp);
            if (wp.showCmd != SW_MAXIMIZE && wp.showCmd != SW_MINIMIZE)
            {
                ShowWindow(ClassroomHwnd, SW_MINIMIZE);
                ShowWindow(ClassroomHwnd, SW_MAXIMIZE);
            }
            SleepUntilNext(100);
        }
    }
}
