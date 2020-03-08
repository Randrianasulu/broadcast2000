#include <string.h>
#include "assetedit.h"
#include "assetmanager.h"
#include "assets.h"
#include "cache.h"
#include "errorbox.h"
#include "indexfile.h"
#include "mainwindow.h"
#include "progressbox.h"
#include "question.h"


AssetManager::AssetManager(MainWindow *mwindow)
 : BC_MenuItem("Assets...")
{
	this->mwindow = mwindow;
	thread = new AssetManagerThread(mwindow);
	thread->create_objects();
}

AssetManager::~AssetManager()
{
	delete thread;
}

int AssetManager::interrupt_indexes()
{
	thread->interrupt_indexes();
return 0;
}

int AssetManager::handle_event()
{
	thread->start();
return 0;
}




AssetManagerThread::AssetManagerThread(MainWindow *mwindow)
 : Thread()
{
	this->mwindow = mwindow;
	window = 0;
}

AssetManagerThread::~AssetManagerThread()
{
	delete index_thread;
}

int AssetManagerThread::create_objects()
{
	index_thread = new AssetIndexThread(this);
return 0;
}

int AssetManagerThread::interrupt_indexes()
{
	index_thread->interrupt_indexes();
	if(window) window->set_done(0);
return 0;
}

void AssetManagerThread::run()
{
	generate_list();

	window = new AssetManagerWindow(this);
	window->create_objects();
	window->run_window();
	delete window;
	window = 0;

// delete list
	int i;
	for(i = 0; i < assets.total; i++) delete assets.values[i];
	assets.remove_all();
}

Asset* AssetManagerThread::get_selection()
{
	int current_selection = asset_list->get_selection_number();

	if(current_selection > -1)
		return assets.values[current_selection]->asset;
	else
		return 0;
}

int AssetManagerThread::generate_list()
{
// delete old list
	int i;
	for(i = 0; i < assets.total; i++) delete assets.values[i];
	assets.remove_all();

// create new list
	Asset *asset = mwindow->assets->first;
	
	char string[1024];
	FileSystem fs;

	for(int i = 0; asset && i < 1000; i++, asset = asset->next)
	{
		if(!asset->silence)
		{
			fs.extract_name(string, asset->path);
			assets.append(new AssetManagerItem(string, asset));
		}
	}
return 0;
}

int AssetManagerThread::update_list()
{
	generate_list();
	asset_list->set_contents((ArrayList<BC_ListBoxItem*> *)&assets, 0, 1);
return 0;
}







AssetManagerItem::AssetManagerItem(char *string, Asset *asset)
 : BC_ListBoxItem(string, BLACK)
{
	this->asset = asset;
}


AssetManagerWindow::AssetManagerWindow(AssetManagerThread *thread)
 : BC_Window("", MEGREY, ICONNAME ": Assets", 490, 340, 0, 0)
{
	this->thread = thread;
	this->mwindow = thread->mwindow;
	this->assets = mwindow->assets;
}

AssetManagerWindow::~AssetManagerWindow()
{
}

int AssetManagerWindow::create_objects()
{
	int y = 10, x = 10;

	add_tool(thread->asset_list = new AssetManagerList(thread, x, y));

	x = 320;
	add_tool(new AssetManagerDeleteDisk(thread, x, y));
	y += 30;
	add_tool(new AssetManagerDeleteProject(thread, x, y));
	y += 30;
	add_tool(new AssetManagerImport(thread, x, y));
	y += 30;
	add_tool(thread->edit_button = new AssetManagerEdit(thread, x, y));
	y += 30;
	add_tool(new AssetManagerIndex(thread, x, y));
	y += 100;
	add_tool(new AssetManagerDone(thread, x, y));
return 0;
}



AssetIndexThread::AssetIndexThread(AssetManagerThread *thread)
 : Thread()
{
	this->thread = thread;
	this->asset = 0;
	index = new IndexFile(thread->mwindow);
}

AssetIndexThread::~AssetIndexThread()
{
	delete index;
}

int AssetIndexThread::build_index(Asset *asset)
{
	this->asset = asset;
	completion_lock.lock();
	start();
return 0;
}


int AssetIndexThread::interrupt_indexes()
{
	index->interrupt_index();
	completion_lock.lock();
	completion_lock.unlock();
return 0;
}

void AssetIndexThread::run()
{
	if(asset)
	{
		thread->window->disable_window();
		ProgressBox progress("", "Rebuilding Index...", 1, 1);
		progress.start();
		index->create_index(thread->mwindow, asset, &progress);
		progress.stop_progress();
		thread->window->enable_window();
	}
	completion_lock.unlock();
}




AssetManagerEdit::AssetManagerEdit(AssetManagerThread *thread, int x, int y)
 : BC_BigButton(x, y, "Get Info")
{
	this->thread = thread;
	edit_thread = new AssetEdit(thread);
}

AssetManagerEdit::~AssetManagerEdit()
{
	delete edit_thread;
}

int AssetManagerEdit::handle_event()
{
	edit_thread->set_asset(thread->get_selection());
	edit_thread->start();
return 0;
}

int AssetManagerEdit::keypress_event()
{
	if(get_keypress() == 13) { handle_event(); return 1; }
	return 0;
return 0;
}






AssetManagerImport::AssetManagerImport(AssetManagerThread *thread, int x, int y)
 : BC_BigButton(x, y, "Append")
{
	this->thread = thread;
}

AssetManagerImport::~AssetManagerImport()
{
}

int AssetManagerImport::handle_event()
{
	Asset *asset = thread->get_selection();
	if(asset) thread->mwindow->load(asset->path, 1);
return 0;
}

int AssetManagerImport::keypress_event()
{
	if(get_keypress() == 'i') { handle_event(); return 1; }
	return 0;
return 0;
}






AssetManagerIndex::AssetManagerIndex(AssetManagerThread *thread, int x, int y)
 : BC_BigButton(x, y, "Rebuild Index")
{
	this->thread = thread;
}

AssetManagerIndex::~AssetManagerIndex()
{
}

int AssetManagerIndex::handle_event()
{
	thread->index_thread->build_index(thread->get_selection());
return 0;
}





AssetManagerDeleteDisk::AssetManagerDeleteDisk(AssetManagerThread *thread, int x, int y)
 : BC_BigButton(x, y, "Delete from disk")
{
	this->thread = thread;
	delete_thread = new AssetDeleteDisk(thread);
}

AssetManagerDeleteDisk::~AssetManagerDeleteDisk()
{
	delete delete_thread;
}

int AssetManagerDeleteDisk::handle_event()
{
	delete_thread->set_asset(thread->get_selection());
	delete_thread->start();
return 0;
}




AssetManagerDeleteProject::AssetManagerDeleteProject(AssetManagerThread *thread, int x, int y)
 : BC_BigButton(x, y, "Delete from project")
{
	this->thread = thread;
	delete_thread = new AssetDeleteProject(thread);
}

AssetManagerDeleteProject::~AssetManagerDeleteProject()
{
	delete delete_thread;
}

int AssetManagerDeleteProject::handle_event()
{
	delete_thread->set_asset(thread->get_selection());
	delete_thread->start();
return 0;
}







AssetManagerDone::AssetManagerDone(AssetManagerThread *thread, int x, int y)
 : BC_BigButton(x, y, "Close")
{
	this->thread = thread;
}

AssetManagerDone::~AssetManagerDone()
{
}

int AssetManagerDone::handle_event()
{
	set_done(0);
return 0;
}

int AssetManagerDone::keypress_event()
{
	if(get_keypress() == ESC) { handle_event();  return 1; }
	return 0;
return 0;
}








AssetManagerList::AssetManagerList(AssetManagerThread *thread, int x, int y)
 : BC_ListBox(x, y, 280, 300, (ArrayList<BC_ListBoxItem*> *)&(thread->assets), 0, 1)
{
	this->thread = thread;
	stay_highlighted();
	selection = 0;
}

AssetManagerList::~AssetManagerList()
{
}

int AssetManagerList::handle_event()
{
	if(get_keypress() == ESC) set_done(0);
	else
	{
		thread->edit_button->handle_event();
		//printf("%s\n", get_selection());
	}
return 0;
}

int AssetManagerList::selection_changed()
{
	if(get_selection_number() != -1) selection = get_selection_number();
return 0;
}


AssetDeleteDisk::AssetDeleteDisk(AssetManagerThread *thread)
 : Thread()
{
	this->thread = thread;
	this->asset = 0;
}

AssetDeleteDisk::~AssetDeleteDisk()
{
}

	
int AssetDeleteDisk::set_asset(Asset *asset)
{
	this->asset = asset;
return 0;
}

void AssetDeleteDisk::run()
{
//printf("%x\n", asset);
	if(asset)
	{
		int result;
		thread->window->disable_window();
		
		{
			QuestionWindow window("");
			window.create_objects("Delete this asset from disk forever?", 0, 0);
			result = window.run_window();
		}

		if(result == 2)
		{
			char index_path[1024];
			IndexFile index(thread->mwindow, asset);
			index.get_index_filename(index_path, asset->path);
			
			thread->mwindow->purge_asset(asset->path);
			remove(index_path);
			remove(asset->path);
			thread->update_list();
		}
//		else
//		{
//			ErrorBox window("");
//			window.create_objects("Asset not deleted.");
//			window.run_window();
//		}
		thread->window->enable_window();
	}
}





AssetDeleteProject::AssetDeleteProject(AssetManagerThread *thread)
 : Thread()
{
	this->thread = thread;
	this->asset = 0;
}

AssetDeleteProject::~AssetDeleteProject()
{
}
	
int AssetDeleteProject::set_asset(Asset *asset)
{
	this->asset = asset;
return 0;
}

void AssetDeleteProject::run()
{
	if(asset)
	{
		int result;
		thread->window->disable_window();
		
		{
			QuestionWindow window("");
			window.create_objects("Delete this asset from project?", 0, 0);
			result = window.run_window();
		}

		if(result == 2)
		{
			thread->mwindow->purge_asset(asset->path);
			thread->update_list();
		}
// 		else
// 		{
// 			ErrorBox window("");
// 			window.create_objects("Asset not deleted.");
// 			window.run_window();
// 		}
		thread->window->enable_window();
	}
}
