#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H

class AssetManagerIndex;
class AssetManagerList;
class AssetManagerEdit;
class AssetManagerWindow;
class AssetDeleteDisk;
class AssetDeleteProject;
class AssetIndexThread;

#include "arraylist.h"
#include "assetedit.inc"
#include "assetmanager.inc"
#include "assets.inc"
#include "bcbase.h"
#include "indexfile.inc"
#include "mainwindow.inc"

class AssetManager : public BC_MenuItem
{
public:
	AssetManager(MainWindow *mwindow);
	~AssetManager();
	
	int handle_event();
	int interrupt_indexes();

	MainWindow *mwindow;
	AssetManagerThread *thread;
};



class AssetManagerItem : public BC_ListBoxItem
{
public:
	AssetManagerItem(char *string, Asset *asset);

	Asset *asset;
};


class AssetManagerThread : public Thread
{
public:
	AssetManagerThread(MainWindow *mwindow);
	~AssetManagerThread();

	friend AssetManagerIndex;

	int create_objects();
	void run();
	Asset* get_selection();
	int update_list();
	int interrupt_indexes();

	AssetManagerList *asset_list;
	AssetManagerEdit *edit_button;

	ArrayList<AssetManagerItem*> assets;
	MainWindow *mwindow;
	AssetManagerWindow *window;

private:
	int generate_list();
	AssetIndexThread *index_thread;
};



class AssetManagerWindow : public BC_Window
{
public:
	AssetManagerWindow(AssetManagerThread *thread);
	~AssetManagerWindow();

	int create_objects();

	AssetManagerThread *thread;
	MainWindow *mwindow;
	Assets *assets;
};


class AssetManagerEdit : public BC_BigButton
{
public:
	AssetManagerEdit(AssetManagerThread *thread, int x, int y);
	~AssetManagerEdit();

	int handle_event();
	int keypress_event();

	AssetEdit *edit_thread;
	AssetManagerThread *thread;
};

class AssetManagerImport : public BC_BigButton
{
public:
	AssetManagerImport(AssetManagerThread *thread, int x, int y);
	~AssetManagerImport();

	int handle_event();
	int keypress_event();

	AssetManagerThread *thread;
};

class AssetManagerPaste : public BC_BigButton
{
public:
	AssetManagerPaste(AssetManagerThread *thread, int x, int y);
	~AssetManagerPaste();

	int handle_event();
	int keypress_event();

	AssetManagerThread *thread;
};

class AssetIndexThread : public Thread
{
public:
	AssetIndexThread(AssetManagerThread *thread);
	~AssetIndexThread();

	int build_index(Asset *asset);
	int interrupt_indexes();

	void run();
	AssetManagerThread *thread;
	Asset *asset;
	IndexFile *index;
	Mutex completion_lock;
};

class AssetManagerIndex : public BC_BigButton
{
public:
	AssetManagerIndex(AssetManagerThread *thread, int x, int y);
	~AssetManagerIndex();

	int handle_event();

	AssetManagerThread *thread;
};




class AssetManagerDeleteDisk : public BC_BigButton
{
public:
	AssetManagerDeleteDisk(AssetManagerThread *thread, int x, int y);
	~AssetManagerDeleteDisk();

	int handle_event();

	AssetManagerThread *thread;
	AssetDeleteDisk *delete_thread;
};

class AssetManagerDeleteProject : public BC_BigButton
{
public:
	AssetManagerDeleteProject(AssetManagerThread *thread, int x, int y);
	~AssetManagerDeleteProject();

	int handle_event();

	AssetManagerThread *thread;
	AssetDeleteProject *delete_thread;
};

class AssetManagerDone : public BC_BigButton
{
public:
	AssetManagerDone(AssetManagerThread *thread, int x, int y);
	~AssetManagerDone();

	int handle_event();
	int keypress_event();

	AssetManagerThread *thread;
};


class AssetManagerList : public BC_ListBox
{
public:
	AssetManagerList(AssetManagerThread *thread, int x, int y);
	~AssetManagerList();
	
	int handle_event();
	int selection_changed();
	
	AssetManagerThread *thread;
	int selection;
};








class AssetDeleteDisk : public Thread
{
public:
	AssetDeleteDisk(AssetManagerThread *thread);
	~AssetDeleteDisk();

	int set_asset(Asset *asset);
	void run();

	Asset *asset;
	AssetManagerThread *thread;
};

class AssetDeleteProject : public Thread
{
public:
	AssetDeleteProject(AssetManagerThread *thread);
	~AssetDeleteProject();

	int set_asset(Asset *asset);
	void run();

	Asset *asset;
	AssetManagerThread *thread;
};


#endif
