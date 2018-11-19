/*
 * Xournal++
 *
 * Dialog with export settings
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "control/jobs/ExportFormtType.h"
#include "control/settings/Settings.h"
#include "gui/GladeGui.h"

#include <PageRange.h>
#include <XournalType.h>

#include <boost/filesystem/path.hpp>
using boost::filesystem::path;

class ExportDialog : public GladeGui
{
public:
	ExportDialog(GladeSearchpath* gladeSearchPath, Settings* settings, int pageCount, int currentPage);
	virtual ~ExportDialog();

public:
	virtual void show(GtkWindow* parent);

	PageRangeVector getRange();
	int getPngDpi();
	ExportFormtType getFormatType();

	string getFilePath();

private:
	bool validate();
	void handleData();

	/**
	 * Callback for a changed selection of an output file
	 */
	static void selectionChanged(GtkFileChooser* chooser, ExportDialog* dlg);

	static gboolean rangeFocused(GtkWidget* widget, GdkEvent* event, ExportDialog* dlg);

	static void fileTypeSelected(GtkTreeView* treeview, ExportDialog* dlg);

	void addFileType(string typeDesc, string pattern, gint type = 0,
					 string filterName = "", bool select = false);

	void setupModel();

	bool validFilename();
	bool validExtension();

	bool fileTypeByExtension();

private:
	XOJ_TYPE_ATTRIB;


	int pageCount;
	int currentPage;
	int resolution;
	ExportFormtType type;

	PageRangeVector range;

	Settings* settings;
	GtkListStore* typesModel;
	GtkTreeView* typesView;

	enum ColIndex
	{
		COL_FILEDESC = 0,
		COL_EXTENSION = 1,
		COL_TYPE = 2
	};
};
