#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QDir>
#include <QFileDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private:
    Ui::Widget *ui;

    QFileDialog* source_file_dialog;
    QFileDialog* destination_file_dialog;

    QStringList file_filters;

    QDir transfer_files(QDir source_dir, QDir destination_dir, bool copy_flag = true) const;
    bool remove_empty(QDir who) const;

private slots:
    void display_info() const;
    void capture_filters();

};
#endif // WIDGET_H
