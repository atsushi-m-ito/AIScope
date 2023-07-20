
#ifndef __winmain_h__
#define __winmain_h__

#pragma once

#define _CRT_SECURE_NO_DEPRECATE
#include "targetver.h"
#include <windows.h>
#include <commdlg.h>

#include "resource.h"
#include "filelist.h"


class AIScopeProperty;
class AIScope;

int WinCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, AIScope& g_aiscope, AIScopeProperty* aiproperty);

void ResetWindowText(HWND hWnd);
int PlayMovie();
//int SetModuleDirPath(const TCHAR* filename, TCHAR* returnpath, DWORD nbuf);
//int SetCurrentDirPath(const TCHAR* filename, TCHAR* returnpath, DWORD nbuf);
void ResetCurrentFileMenu(HWND hWnd, FILETYPE filetype);


//LoadSettingFile.cpp=========================================//
int LoadSettingFile(const TCHAR* filepath);
int SeekFilePath(const TCHAR* filename, TCHAR* filepath, DWORD nbuflength);
void ApplySetting(AIScopeProperty* aiproperty, int num_atoms = 1);


//WinMain.cpp=========================================//
int MyGetFileTime(const TCHAR* filepath, FILETIME* tm);
int GetAtomicNumber(const TCHAR* atomicsymbol);

class UIPanel;
void InitializeUI(UIPanel& uipanel);

class NotifyDistributer;
void InitializewinMenu(HMENU hMenu, NotifyDistributer* distributer);
void TerminateMenu();

void SetCrossSectionMode(int and_or1, int and_or2, int and_or3);
void SetCrossSectionTarget(int target);
#endif	//!__winmain_h__