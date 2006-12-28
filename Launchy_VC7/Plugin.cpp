#include "StdAfx.h"
#include ".\plugin.h"
#include "DiskObject.h"
#include "Launchy.h"
#include "LaunchyDlg.h"
#include "LaunchySmarts.h"


void FreeSearchResult (SearchResult* sr) {

	free(sr->DisplayString);
	free(sr->FullPath);
	free(sr->Location);
	DestroyIcon(sr->DisplayIcon);
}

TCHAR* string2TCHAR(wstring str) {
	TCHAR* dest = (TCHAR*) malloc(sizeof(TCHAR) * (str.length() + 1));
	if (dest == NULL) exit(1);
	_tcscpy(dest, str.c_str());
	return dest;
}

TCHAR* StringArrayToTCHAR( CStringArray& Strings) {
	int size = 0;
	for(int i = 0; i < Strings.GetSize(); i++) {
		size += Strings[i].GetLength() + 1;
	}
	TCHAR* out = (TCHAR*) malloc(sizeof(TCHAR) * size);
	if (out == NULL) exit(1);
	TCHAR* cur = out;
	for(int i = 0; i < Strings.GetSize(); i++) {
		TCHAR* x = Strings[i].GetBuffer();
		_tcscpy(cur, Strings[i].GetBuffer());
		cur += Strings[i].GetLength() + 1;
	}
	return out;
}

vector<wstring> TCHARListToVector(int numStrings,  const TCHAR* Strings) {
	vector<wstring> out;
	const TCHAR* cur = Strings;
	for(int i = 0; i < numStrings; i++) {
		out.push_back(cur);
		cur += out[i].length() + 1;
	}
	return out;
}

Plugin::Plugin(void)
{
	LoadDlls();
	LoadRegExs();
}

Plugin::~Plugin(void)
{
	pfuncs.clear();

	for(int i = 0; i < loadedPlugins.size(); i++) {
		FreeLibrary(loadedPlugins[i].handle);
	}
	loadedPlugins.clear();
}

void Plugin::LoadRegExs() 
{
	for(int i = 0; i < loadedPlugins.size(); i++) {
		if (pfuncs[i].PluginGetRegexs == NULL) continue;

		int numStrings = 0;
		TCHAR* strings = pfuncs[i].PluginGetRegexs(&numStrings);
		vector<wstring> vstrings = TCHARListToVector(numStrings, strings);
		for(int j = 0; j < vstrings.size(); j++) {
			wstring x = vstrings[j];
			boost::wregex tmp(vstrings[j],boost::regex::perl|boost::regex::icase);
			loadedPlugins[i].regexs.push_back(tmp);
		}
		pfuncs[i].PluginFreeStrings(strings);
	}
}

/*
	Determine if one of the plugins owns the searchTxt by 
	a regular expression match
*/
int Plugin::IsSearchOwned(CString search) 
{
	wstring searchS = search.GetBuffer();
	for(int i = 0; i < loadedPlugins.size(); i++) {
		for(int j = 0; j < loadedPlugins[i].regexs.size(); j++) {
			boost::wsmatch what;
			if(boost::regex_match(searchS, what, loadedPlugins[i].regexs[j])) {
				return i;
			}
		}
	}
	return -1;
}





void Plugin::LoadDlls() {
	CDiskObject disk;
	CStringArray files;
	disk.EnumFilesInDirectoryWithFilter(_T("*.dll"), _T("Plugins/"), files);
	for(int i = 0; i < files.GetCount(); i++) {
		CString path = _T("Plugins/") + files[i];
		HINSTANCE LoadMe;
		LoadMe = LoadLibrary(path);
		 
		// Check to see if the library was loaded successfully 
		if (LoadMe == 0) {
			CString out;
			out.Format(_T("Plugin (%s) failed to load!"), files[i]);
			AfxMessageBox(out);
			continue;
		}
		struct DLLInstance di;
		di.handle = LoadMe;

		loadedPlugins.push_back(di);
		PluginFunctions funcs;

		funcs.PluginGetRegexs = (PLUGINGETREGEXS)GetProcAddress(LoadMe,"PluginGetRegexs");
		funcs.PluginGetIndexItems = (PLUGINGETINDEXITEMS)GetProcAddress(LoadMe,"PluginGetIndexItems");
		funcs.PluginUpdateSearch = (PLUGINUPDATESEARCH)GetProcAddress(LoadMe,"PluginUpdateSearch");
		funcs.PluginFileOptions = (PLUGINFILEOPTIONS)GetProcAddress(LoadMe,"PluginFileOptions");
		funcs.PluginDoAction = (PLUGINDOACTION)GetProcAddress(LoadMe,"PluginDoAction");
		funcs.PluginGetIdentifiers = (PLUGINGETIDENTIFIERS)GetProcAddress(LoadMe,"PluginGetIdentifiers");
		funcs.PluginFreeResults = (PLUGINFREERESULTS)GetProcAddress(LoadMe,"PluginFreeResults");
		funcs.PluginFreeStrings = (PLUGINFREESTRINGS)GetProcAddress(LoadMe,"PluginFreeStrings");
		pfuncs.push_back(funcs);
	}
}

vector<FileRecordPtr> Plugin::GetIdentifiers() {
	vector<FileRecordPtr> PluginRecords;
	for(int i = 0; i < loadedPlugins.size(); i++) {
		if (pfuncs[i].PluginGetIdentifiers == NULL)
			continue;
		int NumResults;
		SearchResult* res = pfuncs[i].PluginGetIdentifiers(&NumResults);
		SearchResult* cur = res;
		for(int j = 0; j < NumResults; j++) {
			FileRecordPtr rec(new FileRecord());
			rec->croppedName = cur->DisplayString;
			rec->fullPath = cur->FullPath;
			rec->isHistory = false;
			rec->lowName = rec->croppedName;
			rec->lowName.MakeLower();
			rec->usage = 1000;
			rec->owner = (short) i;

			PluginRecords.push_back(rec);

			cur++;
		}
		pfuncs[i].PluginFreeResults(res, NumResults);
	}

	return PluginRecords;
}

shared_ptr<vector<FileRecordPtr> > Plugin::GetSearchOptions(int owner)
{
	shared_ptr<vector<FileRecordPtr> > out;
	out.reset(new vector<FileRecordPtr>);


	if (owner == -1) {
		// This file isn't owned by anyone, we want to get a suite of options
		// from the available utilities such as "open, enqueue, send email" etc..

		for(int i = 0; i < loadedPlugins.size(); i++) {
			if (pfuncs[i].PluginUpdateSearch == NULL) continue;
			int NumResults;
			TCHAR* szStrings = StringArrayToTCHAR(SearchStrings);
			SearchResult* res = pfuncs[i].PluginFileOptions( TabbedMatch.fullPath, SearchStrings.GetCount(), szStrings, searchTxt, &NumResults);
			free(szStrings);
			SearchResult* cur = res;
			for(int j = 0; j < NumResults; j++) {
				FileRecordPtr rec(new FileRecord());
				rec->croppedName = cur->DisplayString;
				rec->fullPath = cur->FullPath;
				rec->isHistory = false;
				rec->lowName = rec->croppedName;
				rec->lowName.MakeLower();
				rec->usage = 0;
				rec->owner = (short) i;

				out->push_back(rec);
				cur++;
			}	
			pfuncs[i].PluginFreeResults(res, NumResults);
		}
	} else {

		if (pfuncs[owner].PluginUpdateSearch == NULL) return out;


		int NumResults;

		TCHAR* szStrings = StringArrayToTCHAR(SearchStrings);
		SearchResult* res = pfuncs[owner].PluginUpdateSearch(SearchStrings.GetCount(), szStrings, searchTxt, &NumResults);
		free(szStrings);

		SearchResult* cur = res;
		for(int j = 0; j < NumResults; j++) {
			FileRecordPtr rec(new FileRecord());
			rec->croppedName = cur->DisplayString;
			rec->fullPath = cur->FullPath;
			rec->isHistory = false;
			rec->lowName = rec->croppedName;
			rec->lowName.MakeLower();
			rec->usage = 0;
			rec->owner = (short) owner;

			out->push_back(rec);
			cur++;
		}	

		pfuncs[owner].PluginFreeResults(res, NumResults);
	}
	return out;
}



void Plugin::Launch(short PluginID, TCHAR* fullPath) 
{
	TCHAR* szStrings = StringArrayToTCHAR(SearchStrings);

	pfuncs[PluginID].PluginDoAction(SearchStrings.GetCount(), szStrings, searchTxt, fullPath);
	free(szStrings);
}