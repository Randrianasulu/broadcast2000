#ifndef MAINUNDO_H
#define MAINUNDO_H


#include "filehtal.inc"
#include "mainwindow.inc"
#include "undostack.h"

class MainUndo
{
public:
	MainUndo(MainWindow *mwindow);
	~MainUndo();

	int update_undo_all(const char *description = "", int after = 1);            // saves info on everything
	int update_undo_audio(const char *description = "", int after = 1);
	int update_undo_edits(const char *description = "", int after = 1);
	int update_undo_patches(const char *description = "", int after = 1);
	int update_undo_console(const char *description = "", int after = 1);
	int update_undo_timebar(const char *description = "", int after = 1);
	int update_undo_automation(const char *description = "", int after = 1);

	int undo();
	int redo();

private:
	int load_from_undo(FileHTAL *html, const char *type);    // loads undo from the stringfile to the project
	int update_undo(const char *description, const char *type, int after);

	UndoStack undo_stack;
	UndoStackItem* current_entry; // for setting the after buffer
	MainWindow *mwindow;
};

#endif
