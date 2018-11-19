#include "BaseExportJob.h"

#include "SynchronizedProgressListener.h"
#include "control/Control.h"
#include "pdf/popplerdirect/PdfExport.h"

#include <i18n.h>

#include <boost/algorithm/string/predicate.hpp>

BaseExportJob::BaseExportJob(Control* control, string name)
 : BlockingJob(control, name)
{
	XOJ_INIT_TYPE(BaseExportJob);
}

BaseExportJob::~BaseExportJob()
{
	XOJ_RELEASE_TYPE(BaseExportJob);
}

bool BaseExportJob::showFilechooser()
{
	XOJ_CHECK_TYPE(BaseExportJob);

	Settings* settings = control->getSettings();
	Document* doc = control->getDocument();

	GtkWidget* dialog = gtk_file_chooser_dialog_new(_C("Export PDF"), (GtkWindow*) *control->getWindow(), GTK_FILE_CHOOSER_ACTION_SAVE,
													_C("_Cancel"), GTK_RESPONSE_CANCEL,
													_C("_Save"), GTK_RESPONSE_OK, NULL);

	gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(dialog), false);

	GtkFileFilter* filterPdf = gtk_file_filter_new();
	gtk_file_filter_set_name(filterPdf, _C("PDF files"));
	gtk_file_filter_add_pattern(filterPdf, "*.pdf");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterPdf);

	path savePath;

	doc->lock();
	if (!doc->getFilename().empty())
	{
		savePath = doc->getFilename();
	}
	else if (!doc->getPdfFilename().empty())
	{
		savePath = doc->getPdfFilename().filename().replace_extension("");
	}
	else
	{
		time_t curtime = time(NULL);
		char stime[128];
		strftime(stime, sizeof(stime), settings->getDefaultSaveName().c_str(), localtime(&curtime));
		savePath /= stime;

		if (savePath.extension() != ".xoj")
		{
			savePath += ".xoj";
		}
	}
	doc->unlock();

	savePath += ".pdf";
		
	GFile* folder = g_file_new_for_path(savePath.parent_path().c_str());
	gtk_file_chooser_set_current_folder_uri(GTK_FILE_CHOOSER(dialog), g_file_get_uri(folder));
	g_object_unref(folder);

	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), savePath.filename().c_str());
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), true);

	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(this->control->getWindow()->getWindow()));
	if (gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_OK)
	{
		gtk_widget_destroy(dialog);
		return false;
	}

	string uri(gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(dialog)));
	if (!ba::starts_with(uri, "file://"))
	{
		// ensure local file
		return false;
	}

	this->filename = path(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog)));
	settings->setLastSavePath(this->filename.parent_path());

	gtk_widget_destroy(dialog);

	return true;
}

void BaseExportJob::afterRun()
{
	XOJ_CHECK_TYPE(BaseExportJob);

	if (!this->errorMsg.empty())
	{
		GtkWindow* win = (GtkWindow*) *control->getWindow();
		GtkWidget* dialog = gtk_message_dialog_new(win, GTK_DIALOG_DESTROY_WITH_PARENT,
												   GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s", this->errorMsg.c_str());
		gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(this->control->getWindow()->getWindow()));
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	}
}

void BaseExportJob::run(bool noThreads)
{
	XOJ_CHECK_TYPE(BaseExportJob);

	SynchronizedProgressListener pglistener(this->control);

	Document* doc = control->getDocument();
	doc->lock();
	PdfExport pdf(doc, &pglistener);
	doc->unlock();

	if (!pdf.createPdf(this->filename))
	{
		if (control->getWindow())
		{
			callAfterRun();
		}
		else
		{
			g_error("%s%s", "Create pdf failed: ", pdf.getLastError().c_str());
		}
	}
}
