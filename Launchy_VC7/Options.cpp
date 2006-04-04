/*
Launchy: Application Launcher
Copyright (C) 2005  Josh Karlin

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "StdAfx.h"
#include "Options.h"
#include "LaunchyDlg.h"
#include "LaunchySmarts.h"
#include ".\options.h"





Options::Options(void) : ini(new CIniFile())
, vkey(0)
, mod_key(0)
{
	firstRun = false;
	TCHAR name[256];
	DWORD size = 256;
	GetUserName(name, &size);
	userDir.Format(_T("Users\\%s\\"), name);
	CDiskObject disk;
	disk.CreateDirectory(userDir);
	userDir += _T("\\launchy.ini");
	ini->SetPath(userDir);
	if (!ini->ReadFile()) {
		firstRun = true;
	}
	ParseIni();
	LoadSkins();
	UpgradeCleanup();
}



Options::~Options(void)
{
	Store();
}

vector<CString> DeSerializeStringArray(CString input) {
	vector<CString> output;

	CString tmp;
	int cur = 0;
	while(true) {
		int c = input.Find(_T(";"), cur);
		if (c > 0 && c < input.GetLength()) {
			tmp = input.Mid(cur,c-cur);
			output.push_back(tmp);
		} else {
			break;
		}
		cur = c+1;
	}
	return output;
}

CString SerializeStringArray(vector<CString> input) {
	CString output = _T("");
	for(uint i = 0; i < input.size(); i++) {
		output.Append(input[i]);
		output.Append(_T(";"));
	}
	return output;
}

void Options::ParseIni(void)
{
	CString DefaultDirs;
	CString myMenu, allMenus;
	LaunchySmarts::GetShellDir(CSIDL_COMMON_STARTMENU, allMenus);
	LaunchySmarts::GetShellDir(CSIDL_STARTMENU, myMenu);
	DefaultDirs.Format(_T("%s;%s;"), myMenu, allMenus);

	ver = ini->GetValueI(L"Launchy Information", L"Version", 0);

	posX = ini->GetValueI(L"Position", L"pos_x");
	posY = ini->GetValueI(L"Position", L"pos_y");

	mod_key =  ini->GetValueI(L"Hotkey", L"mod_key", MOD_ALT);
	vkey =  ini->GetValueI(L"Hotkey", L"vkey", VK_SPACE);

	skinName = ini->GetValue(L"Skin", L"name", L"Default").c_str();

	Directories = DeSerializeStringArray(ini->GetValue(L"General", L"Directories", DefaultDirs.GetBuffer()).c_str());
	Types = DeSerializeStringArray(ini->GetValue(L"General", L"Types", L".lnk;").c_str());


}

void Options::Store(void)
{
	CWinApp* pApp = AfxGetApp();
	if (pApp == NULL) return;
	RECT location;
	pApp->GetMainWnd()->GetWindowRect(&location);

	ini->SetValueI(L"Launchy Information", L"Version", LAUNCHY_VERSION);
	ini->SetValueI(L"Position", L"pos_x", location.left);
	ini->SetValueI(L"Position", L"pos_y", location.top);

	ini->SetValueI(L"Hotkey", L"mod_key", mod_key);
	ini->SetValueI(L"Hotkey", L"vkey", vkey);

	ini->SetValue(L"Skin", L"name", skinName.GetBuffer());

	ini->SetValue(L"General", L"Directories", SerializeStringArray(Directories).GetBuffer());
	ini->SetValue(L"General", L"Types", SerializeStringArray(Types).GetBuffer());

	ini->WriteFile();
}

void Options::Associate(CString entry, CString destination)
{
	ini->SetValue(L"Associations", std::wstring(entry.GetBuffer()), std::wstring(destination.GetBuffer()));
}


CString Options::GetAssociation(CString query)
{
	wstring res = ini->GetValue(L"Associations", std::wstring(query.GetBuffer()));

	if (res != L"") {
		CString x = CString(res.c_str());
		return x;
	}
	return _T("");
}

void Options::LoadSkins(void)
{
	wchar_t buffer[_MAX_PATH];
	
	/* Get the current working directory: */
	_wgetcwd( buffer, _MAX_PATH );

	CString dir = buffer;
	dir += "\\Skins\\";

	CDiskObject disk;
	CStringArray skinDirs;
	disk.EnumAllDirectories(dir, skinDirs);

	INT_PTR count = skinDirs.GetCount();
	for(int i = 0; i < count; i++) {
		shared_ptr<Skin> x(new Skin(skinDirs[i]));
		if (x->name == _T("")) continue;
		skins.push_back(x);
		if (x->name == skinName) {
			skin = x;
		}
	}


}

void Options::UpgradeCleanup(void)
{
	if (firstRun) {
/*		AfxMessageBox(_T("Welcome to Launchy!\n \
			Launchy is currently running in the background and can be brought forward by pressing Alt+Space\n \
			Enjoy!"));
			*/
	} 

	// Ver == 0 for all versions below 0.91 (ver wasn't added until 0.91)
	if (ver == 0) {
		// Remove the old configuration directories if they exist
		CString dir;
		LaunchySmarts::GetShellDir(CSIDL_LOCAL_APPDATA, dir);
		dir += _T("\\Launchy");
		CDiskObject disk;
		disk.RemoveDirectory(dir);
	}
}
