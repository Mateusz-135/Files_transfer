#include "widget.h"
#include "./ui_widget.h"

#include <QFileDialog>
#include <QFile>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , source_file_dialog{new QFileDialog(this, "Frytki?", "C:/")}
    , destination_file_dialog{new QFileDialog(this, "Frytki?", "C:/")}
{
    ui->setupUi(this);

    source_file_dialog->setFileMode(QFileDialog::Directory);
    destination_file_dialog->setFileMode(QFileDialog::Directory);

    //---------------------------------------------------------------------------------------------------- source path
    connect(ui->source_dir_button, &QToolButton::clicked, this, [this](){
        source_file_dialog->show();
    });

    connect(source_file_dialog, &QFileDialog::fileSelected, this, [this](){
        ui->source_dir_line_edit->setText(source_file_dialog->directory().path());
        display_info();
    });

    connect(ui->source_dir_line_edit, &QLineEdit::textChanged, this, [this](){
        if(ui->source_dir_line_edit->text().isEmpty())
            ui->remove_empty_button->setEnabled(false);
        else
            ui->remove_empty_button->setEnabled(true);
    });

    //---------------------------------------------------------------------------------------------------- destination path
    connect(ui->destination_dir_button, &QToolButton::clicked, this, [this](){
        destination_file_dialog->show();
    });

    connect(destination_file_dialog, &QFileDialog::fileSelected, this, [this](){
        ui->destination_dir_line_edit->setText(destination_file_dialog->directory().path());
        display_info();
    });

    //---------------------------------------------------------------------------------------------------- file filters
    connect(ui->file_filters_line_edit, &QLineEdit::textChanged, this, [this](){
        ui->confirm_filters_button->setEnabled(true);
    });

    connect(ui->confirm_filters_button, &QPushButton::clicked, this, [this](){ // capture filters
        ui->confirm_filters_button->setEnabled(false);

        capture_filters();

        display_info();
    });

    //---------------------------------------------------------------------------------------------------- transfer files to destination
    connect(ui->tranfer_button, &QPushButton::clicked, this, [this](){
        if(ui->remove_empty_checkbox->isChecked())
            remove_empty( transfer_files(source_file_dialog->directory(), destination_file_dialog->directory()) ); // first transfer then clean off the empty folders
        else
            transfer_files(source_file_dialog->directory(), destination_file_dialog->directory()); // just transfer
    });

    //---------------------------------------------------------------------------------------------------- remove empty folders inside source directory
    connect(ui->remove_empty_button, &QPushButton::clicked, this, [this](){
        remove_empty(source_file_dialog->directory());
    });

    //---------------------------------------------------------------------------------------------------- clear logs
    connect(ui->clear_logs_button, &QPushButton::clicked, ui->logs_list, &QListWidget::clear);
}

Widget::~Widget(){
    delete ui;
}

//------------------------------------------------------------------------------------------------------------------------------------ display_info
void Widget::display_info() const{
    ui->info_list->clear();
    ui->info_list->addItem("Source path: " + source_file_dialog->directory().path());
    ui->info_list->addItem("Destination path: " + destination_file_dialog->directory().path());
    QString filters{"File Filters: "};
    for(const QString &file_filter : file_filters)
        filters.append(file_filter);
    ui->info_list->addItem(filters);
}

//------------------------------------------------------------------------------------------------------------------------------------ transfer_files
QDir Widget::transfer_files(QDir source_dir, QDir destination_dir, bool copy_flag) const{
    QString folder_name{source_dir.dirName()}; // name of the folder to which files will be copied
    if(copy_flag)
        folder_name += "_copy";

    QString destination_path{destination_dir.path() + "/" + folder_name};
    destination_dir.mkdir(folder_name); // create a new folder
    destination_dir.cd(destination_path); // and change destination directory to that folder

    QStringList source_files_list = source_dir.entryList(this->file_filters, QDir::Files | QDir::NoDotAndDotDot); // list of files to copy
    for(const QString &file_name : source_files_list){

        QString file_dir{source_dir.path() + "/" + file_name};

        ui->logs_list->addItem("Attempt at copying " + file_dir);

        QFile file{file_dir};
        file.open(QFile::ReadOnly);

        if( file.copy(destination_dir.path() + "/" + file_name) )
            ui->logs_list->addItem(file_dir + " was copied successfully\n");

        file.close();
    }


    QStringList source_dirs_list = source_dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot); // repeat the process inside inner directories
    for(const QString &dir_name : source_dirs_list){
        QDir sub_source_dir{source_dir.path() + "/" + dir_name};
        QDir sub_dest_dir{destination_dir.path()};
        transfer_files(sub_source_dir, sub_dest_dir, false);
    }

    return QDir(destination_path);
}

//------------------------------------------------------------------------------------------------------------------------------------ capture_filters
void Widget::capture_filters(){
    file_filters.clear();
    QString filters = ui->file_filters_line_edit->text();
    QTextStream text_stream{QTextStream(&filters)};
    QString filter;
    do{
        text_stream >> filter;
        if(filter == "")
            break;

        file_filters.push_back(filter);
    }while(filter != "");
}

//------------------------------------------------------------------------------------------------------------------------------------ remove_empty
bool Widget::remove_empty(QDir who) const
{
    QStringList source_dirs_list = who.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    bool sub_sub_dir_removed{false};
    bool sub_dir_removed{false};

    for(const QString &dir_name : source_dirs_list){
        QDir sub_dir{who.path() + "/" + dir_name};
        if(sub_dir.isEmpty()){
            if(sub_dir.rmdir(sub_dir.path())){
                ui->logs_list->addItem(sub_dir.path() + " removed");
                sub_dir_removed = true;
            }
        }
        else{
            sub_sub_dir_removed = remove_empty(sub_dir); // if any sub directory of sub directory was removed, then there's a chance that any other
        }
    }

    if(sub_sub_dir_removed) // because there's a chance that some directory is now empty, try to clean again
        remove_empty(who);

    return sub_dir_removed; // return true if any sub directory was removed (if it was removed, there's a chance that this directory is empty now)
}

