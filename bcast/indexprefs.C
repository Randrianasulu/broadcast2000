#include <string.h>
#include "deleteallindexes.h"
#include "indexprefs.h"
#include "preferences.h"

IndexPrefs::IndexPrefs(PreferencesWindow *pwindow)
 : PreferencesDialog(pwindow)
{
}

IndexPrefs::~IndexPrefs()
{
	delete ipath;
	delete ipathtext;
	delete isize;
	delete csize;
	delete deleteall;
//	delete smp;
	delete icount;
}

int IndexPrefs::create_objects()
{
	int x = 10, y = 10;
	char string[1024];

	add_border(get_resources()->get_bg_shadow1(),
		get_resources()->get_bg_shadow2(),
		get_resources()->get_bg_color(),
		get_resources()->get_bg_light2(),
		get_resources()->get_bg_light1());
	add_tool(new BC_Title(x, y, "Performance", LARGEFONT, BLACK));

	y += 35;
	add_tool(new BC_Title(x, y, "Index files go here:", MEDIUMFONT, BLACK));
	y += 20;
	add_tool(ipathtext = new IndexPathText(x, y, pwindow, pwindow->preferences->index_directory));
	add_tool(ipath = new IndexPath(x + 250, y, pwindow, ipathtext, pwindow->preferences->index_directory));
	y += 30;
	add_tool(new BC_Title(x, y + 5, "Size of index file:", MEDIUMFONT, BLACK));
	sprintf(string, "%ld", pwindow->preferences->index_size);
	add_tool(isize = new IndexSize(240, y, pwindow, string));
	y += 30;
	add_tool(new BC_Title(x, y + 5, "Number of index files to keep:", MEDIUMFONT, BLACK));
	sprintf(string, "%ld", pwindow->preferences->index_count);
	add_tool(icount = new IndexCount(240, y, pwindow, string));
	y += 30;
	add_tool(deleteall = new DeleteAllIndexes(x, y, pwindow));
	y += 30;
	add_tool(new BC_Title(x, y + 5, "Cache items:", MEDIUMFONT, BLACK));
	sprintf(string, "%ld", pwindow->preferences->cache_size);
	add_tool(csize = new CacheSize(240, y, pwindow, string));
	y += 30;
// 	add_tool(new BC_Title(x, y + 5, "Number of CPUs to use:"));
// 	add_tool(smp = new PrefsSMP(240, y, pwindow));
return 0;
}






IndexPath::IndexPath(int x, int y, PreferencesWindow *pwindow, BC_TextBox *textbox, char *text)
 : BrowseButton(x, y, textbox, text, "Index Path", "Select the directory for index files", 1)
{ this->pwindow = pwindow; }

IndexPath::~IndexPath() {}

IndexPathText::IndexPathText(int x, int y, PreferencesWindow *pwindow, char *text)
 : BC_TextBox(x, y, 240, text)
{ this->pwindow = pwindow; }

IndexPathText::~IndexPathText() {}

int IndexPathText::handle_event()
{
	strcpy(pwindow->preferences->index_directory, get_text());
return 0;
}




IndexSize::IndexSize(int x, int y, PreferencesWindow *pwindow, char *text)
 : BC_TextBox(x, y, 100, text)
{ this->pwindow = pwindow; }

int IndexSize::handle_event()
{
  static long result;
  
  result = atol(get_text());
  if(result < 64000) result = 64000;
  //if(result < 500000) result = 500000;
  pwindow->preferences->index_size = result;
return 0;
}



IndexCount::IndexCount(int x, int y, PreferencesWindow *pwindow, char *text)
 : BC_TextBox(x, y, 100, text)
{ this->pwindow = pwindow; }

int IndexCount::handle_event()
{
  static long result;
  
  result = atol(get_text());
  if(result < 1) result = 1;
  pwindow->preferences->index_count = result;
return 0;
}



PrefsSMP::PrefsSMP(int x, int y, PreferencesWindow *pwindow)
 : BC_TextBox(x, y, 100, pwindow->preferences->smp + 1)
{ this->pwindow = pwindow; }

PrefsSMP::~PrefsSMP()
{
}

int PrefsSMP::handle_event()
{
	pwindow->preferences->smp = atol(get_text()) - 1;
	if(pwindow->preferences->smp < 0) pwindow->preferences->smp = 0;
return 0;
}


CacheSize::CacheSize(int x, int y, PreferencesWindow *pwindow, char *text)
 : BC_TextBox(x, y, 100, text)
{ this->pwindow = pwindow; }

int CacheSize::handle_event()
{
  static long result;
  
  result = atol(get_text());
  pwindow->preferences->cache_size = result;
return 0;
}

