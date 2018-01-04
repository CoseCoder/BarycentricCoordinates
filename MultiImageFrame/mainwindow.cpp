#include <QtWidgets>
#include <QComboBox> 
#include <QDebug>
#include "mainwindow.h"
#include "ChildWindow.h"
#include "ImageWidget.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	//ui.setupUi(this);

	mdi_area_ = new QMdiArea;
	mdi_area_->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	mdi_area_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	setCentralWidget(mdi_area_);

	window_mapper_ = new QSignalMapper(this);
	connect(window_mapper_, SIGNAL(mapped(QWidget*)), this, SLOT(SetActiveSubWindow(QWidget*)));

	CreateActions();
	CreateMenus();
	CreateToolBars();
	CreateStatusBar();
}

MainWindow::~MainWindow()
{

}

void MainWindow::CreateActions()
{
	// 	action_new_ = new QAction(QIcon(":/MainWindow/Resources/images/new.png"), tr("&New"), this);
	// 	action_new_->setShortcut(QKeySequence::New);
	// 	action_new_->setStatusTip(tr("Create a new file"));

		// File IO
	action_open_ = new QAction(QIcon(":/MainWindow/Resources/images/open.png"), tr("&Open..."), this);
	action_open_->setShortcuts(QKeySequence::Open);
	action_open_->setStatusTip(tr("Open an existing file"));
	connect(action_open_, SIGNAL(triggered()), this, SLOT(Open()));

	action_save_ = new QAction(QIcon(":/MainWindow/Resources/images/save.png"), tr("&Save"), this);
	action_save_->setShortcuts(QKeySequence::Save);
	action_save_->setStatusTip(tr("Save the document to disk"));
	connect(action_save_, SIGNAL(triggered()), this, SLOT(Save()));

	action_saveas_ = new QAction(tr("Save &As..."), this);
	action_saveas_->setShortcuts(QKeySequence::SaveAs);
	action_saveas_->setStatusTip(tr("Save the document under a new name"));
	connect(action_saveas_, SIGNAL(triggered()), this, SLOT(SaveAs()));

	// Image processing
	action_invert_ = new QAction(tr("Inverse"), this);
	action_invert_->setStatusTip(tr("Invert all pixel value in the image"));
	connect(action_invert_, SIGNAL(triggered()), this, SLOT(Invert()));

	action_mirror_ = new QAction(tr("Mirror"), this);
	action_mirror_->setStatusTip(tr("Mirror image vertically or horizontally"));
	connect(action_mirror_, SIGNAL(triggered()), this, SLOT(Mirror()));

	action_gray_ = new QAction(tr("Grayscale"), this);
	action_gray_->setStatusTip(tr("Gray-scale map"));
	connect(action_gray_, SIGNAL(triggered()), this, SLOT(GrayScale()));

	action_restore_ = new QAction(tr("Restore"), this);
	action_restore_->setStatusTip(tr("Show origin image"));
	connect(action_restore_, SIGNAL(triggered()), this, SLOT(Restore()));

	action_rectangle_ = new QAction(tr("Rectangle"), this);
	action_rectangle_->setStatusTip(tr("Rectangle"));
	connect(action_rectangle_, SIGNAL(triggered()), this, SLOT(ChooseRect()));

	action_paste_ = new QAction(tr("Paste"), this);
	action_paste_->setStatusTip(tr("Paste"));
	connect(action_paste_, SIGNAL(triggered()), this, SLOT(Paste()));

}

void MainWindow::CreateMenus()
{
	menu_file_ = menuBar()->addMenu(tr("&File"));
	menu_file_->setStatusTip(tr("File menu"));
	//	menu_file_->addAction(action_new_);
	menu_file_->addAction(action_open_);
	menu_file_->addAction(action_save_);
	menu_file_->addAction(action_saveas_);

	//	menu_file_->addAction(action_rectangle_);

	menu_edit_ = menuBar()->addMenu(tr("&Edit"));
	menu_edit_->setStatusTip(tr("Edit menu"));
	menu_edit_->addAction(action_invert_);
	menu_edit_->addAction(action_mirror_);
	menu_edit_->addAction(action_gray_);
	menu_edit_->addAction(action_restore_);

	menu_figure_ = menuBar()->addMenu(tr("&Figure"));
	menu_figure_->setStatusTip(tr("Choose an area"));
	menu_figure_->addAction(action_rectangle_);
}

void MainWindow::CreateToolBars()
{
	toolbar_file_ = addToolBar(tr("File"));
	//	toolbar_file_->addAction(action_new_);
	toolbar_file_->addAction(action_open_);
	toolbar_file_->addAction(action_save_);

	// Add separator in toolbar 
	toolbar_file_->addSeparator();
	toolbar_file_->addAction(action_invert_);
	toolbar_file_->addAction(action_mirror_);
	toolbar_file_->addAction(action_gray_);
	toolbar_file_->addAction(action_restore_);


	toolbar_file_->addSeparator();
	/*toolbar_file_->addAction(action_rectangle_);*/
	comboBox_figure_ = new QComboBox();
	toolbar_file_->addWidget(comboBox_figure_);
	comboBox_figure_->addItem(QWidget::tr("Figure"));
	comboBox_figure_->addItem(QWidget::tr("Rectangle"));
	comboBox_figure_->addItem(QWidget::tr("Polygon"));
	connect(comboBox_figure_, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int index) {
		ChildWindow *child = qobject_cast<ChildWindow *>(mdi_area_->activeSubWindow()->widget());
		if (index != 0)
			child->imagewidget_->set_draw_figure(index);
		child->imagewidget_->set_source_window(child_source_); });

	toolbar_file_->addAction(action_paste_);
	comboBox_choose_ = new QComboBox();
	toolbar_file_->addWidget(comboBox_choose_);
	comboBox_choose_->addItem(QWidget::tr("Warp"));
	comboBox_choose_->addItem(QWidget::tr("WP"));
	comboBox_choose_->addItem(QWidget::tr("MV"));
	comboBox_choose_->addItem(QWidget::tr("DH"));

	//connect(comboBox_choose_, SIGNAL(QComboBox::currentIndexChanged(int index_)), this, SLOT(Warp()));
	connect(comboBox_choose_, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int index) {
		ChildWindow *child = qobject_cast<ChildWindow *>(mdi_area_->activeSubWindow()->widget());
		if (index != 0)
			child->imagewidget_->set_draw_status_to_warp(index);
		child->imagewidget_->set_source_window(child_source_); });
}

void MainWindow::CreateStatusBar()
{
	statusBar()->showMessage(tr("Ready"));
}

QMdiSubWindow * MainWindow::FindChild(const QString & filename)
{
	QString canonical_filepath = QFileInfo(filename).canonicalFilePath();

	foreach(QMdiSubWindow *window, mdi_area_->subWindowList())
	{
		ChildWindow *child = qobject_cast<ChildWindow *>(window->widget());
		if (child->current_file() == canonical_filepath)
		{
			return window;
		}
	}

	return 0;
}

void MainWindow::Open()
{
	QString filename = QFileDialog::getOpenFileName(this);
	if (!filename.isEmpty())
	{
		QMdiSubWindow *existing = FindChild(filename);

		if (existing)
		{
			mdi_area_->setActiveSubWindow(existing);
			return;
		}

		ChildWindow *child = CreateChildWindow();
		if (child->LoadFile(filename))
		{
			statusBar()->showMessage(tr("File loaded"), 2000);
			child->show();

			// Change size of the child window so it can fit image size
			int x = child->geometry().x();
			int y = child->geometry().y();
			int width = child->imagewidget_->ImageWidth();
			int height = child->imagewidget_->ImageHeight();
			mdi_area_->activeSubWindow()->setFixedSize(width + 2 * x, height + x + y);
		}
		else
		{
			child->close();
		}
	}
}

void MainWindow::Save()
{
	SaveAs();
}

void MainWindow::SaveAs()
{
	ChildWindow *window = qobject_cast<ChildWindow *>(mdi_area_->activeSubWindow()->widget());
	window->imagewidget_->SaveAs();
}

ChildWindow *MainWindow::CreateChildWindow()
{
	ChildWindow *child = new ChildWindow;
	mdi_area_->addSubWindow(child);

	return child;
}

void MainWindow::SetActiveSubWindow(QWidget* window)
{
	if (!window)
	{
		return;
	}

	mdi_area_->setActiveSubWindow(qobject_cast<QMdiSubWindow *>(window));
}

void MainWindow::Invert()
{
	ChildWindow *window = qobject_cast<ChildWindow *>(mdi_area_->activeSubWindow()->widget());
	window->imagewidget_->Invert();
}

void MainWindow::Mirror()
{
	ChildWindow *window = qobject_cast<ChildWindow *>(mdi_area_->activeSubWindow()->widget());
	window->imagewidget_->Mirror();
}

void MainWindow::GrayScale()
{
	ChildWindow *window = qobject_cast<ChildWindow *>(mdi_area_->activeSubWindow()->widget());
	window->imagewidget_->TurnGray();
}

void MainWindow::Restore()
{
	ChildWindow *window = qobject_cast<ChildWindow *>(mdi_area_->activeSubWindow()->widget());
	window->imagewidget_->Restore();
}

void MainWindow::ChooseRect()
{
	// Set source child window
	ChildWindow *window = qobject_cast<ChildWindow *>(mdi_area_->activeSubWindow()->widget());
	window->imagewidget_->set_draw_status_to_choose();
	child_source_ = window;
}

void MainWindow::Paste()
{
	// Paste image rect region to object image
	ChildWindow *child = qobject_cast<ChildWindow *>(mdi_area_->activeSubWindow()->widget());
	child->imagewidget_->set_draw_status_to_paste();
	child->imagewidget_->set_source_window(child_source_);
}



