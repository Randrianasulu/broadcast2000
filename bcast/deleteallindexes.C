#include <string.h>
#include "deleteallindexes.h"
#include "preferences.h"
#include "preferencesthread.h"


DeleteAllIndexes::DeleteAllIndexes(int x, int y, PreferencesWindow *pwindow)
 : BC_BigButton(x, y, "Delete all indexes"), Thread()
{ this->pwindow = pwindow; }

DeleteAllIndexes::~DeleteAllIndexes() { }

int DeleteAllIndexes::handle_event() { start(); return 0;
}

void DeleteAllIndexes::run()
{
	char string1[1024], string2[1024];
// prepare directory
	strcpy(string1, pwindow->preferences->index_directory);
	FileSystem dir;
	dir.update(pwindow->preferences->index_directory);
	dir.complete_path(string1);
// prepare filter
	const char *filter = ".idx";


	pwindow->disable_window();
	char string[1024];
	sprintf(string, "Delete all indexes in %s?", string1);
	ConfirmDeleteAllIndexes confirm(string);
	confirm.create_objects();

	int result = confirm.run_window();
	if(!result)
	{
		static int i, j, k;

		for(i = 0; i < dir.dir_list.total; i++)
		{
  			result = 1;
			sprintf(string2, "%s%s", string1, dir.dir_list.values[i]->name);
// test filter
			for(j = strlen(string2) - 1, k = strlen(filter) - 1; 
    			j > 0 && k > 0 && string2[j] == filter[k]; j--, k--)
			{
				;
			}
			if(k == 0) result = 0;
			//printf("%s %s %d\n", string2, filter, result);
			if(!result) remove(string2);
		}
	}
	pwindow->redraw_indexes = 1;
	pwindow->enable_window();
}


ConfirmDeleteAllIndexes::ConfirmDeleteAllIndexes(char *string)
 : BC_Window("", MEGREY, ICONNAME ": Delete All Indexes", 340, 140, 340, 140)
{ this->string = string; }

ConfirmDeleteAllIndexes::~ConfirmDeleteAllIndexes()
{
	delete ok;
	delete cancel;
}
	
int ConfirmDeleteAllIndexes::create_objects()
{ 
	add_tool(new BC_Title(5, 5, string));

	add_tool(ok = new DeleteAllIndexesOK(this));
	add_tool(cancel = new DeleteAllIndexesCancel(this));
return 0;
}

DeleteAllIndexesOK::DeleteAllIndexesOK(ConfirmDeleteAllIndexes *window)
 : BC_BigButton(5, 30, "Yes")
{ this->window = window; }

DeleteAllIndexesOK::~DeleteAllIndexesOK() {}
	
int DeleteAllIndexesOK::handle_event() { window->set_done(0); return 0;
}

DeleteAllIndexesCancel::DeleteAllIndexesCancel(ConfirmDeleteAllIndexes *window)
 : BC_BigButton(105, 30, "No")
{ this->window = window; }

DeleteAllIndexesCancel::~DeleteAllIndexesCancel() {}
	
int DeleteAllIndexesCancel::handle_event() { window->set_done(1); return 0;
}




