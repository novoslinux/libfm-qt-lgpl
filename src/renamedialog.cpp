/*
 * Copyright (C) 2013 - 2015  Hong Jen Yee (PCMan) <pcman.tw@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */


#include "renamedialog.h"
#include "ui_rename-dialog.h"
#include <QStringBuilder>
#include <QPushButton>
#include <QDateTime>

#include "core/iconinfo.h"
#include "utilities.h"

#include "core/legacy/fm-config.h"

namespace Fm {

RenameDialog::RenameDialog(const FileInfo &src, const FileInfo &dest, QWidget* parent, Qt::WindowFlags f):
    QDialog(parent, f),
    action_(ActionIgnore),
    applyToAll_(false) {

    ui = new Ui::RenameDialog();
    ui->setupUi(this);

    auto path = dest.path();
    auto srcIcon = src.icon();
    auto destIcon = dest.icon();

    // show info for the source file
    QIcon icon = srcIcon->qicon();
    // FIXME: deprecate fm_config
    QSize iconSize(fm_config->big_icon_size, fm_config->big_icon_size);
    QPixmap pixmap = icon.pixmap(iconSize);
    ui->srcIcon->setPixmap(pixmap);

    QString infoStr;
    // FIXME: deprecate fm_config
    auto disp_size = Fm::formatFileSize(src.size(), fm_config->si_unit);
    auto srcMtime = locale().toString(QDateTime::fromMSecsSinceEpoch(src.mtime() * 1000), QLocale::ShortFormat);
    if(!disp_size.isEmpty()) {
        infoStr = QString(tr("Type: %1\nSize: %2\nModified: %3"))
                  .arg(src.description(),
                       disp_size,
                       srcMtime);
    }
    else {
        infoStr = QString(tr("Type: %1\nModified: %2"))
                  .arg(src.description(), srcMtime);
    }
    ui->srcInfo->setText(infoStr);

    // show info for the dest file
    icon = destIcon->qicon();
    pixmap = icon.pixmap(iconSize);
    ui->destIcon->setPixmap(pixmap);

    disp_size = Fm::formatFileSize(dest.size(), fm_config->si_unit);
    auto destMtime = locale().toString(QDateTime::fromMSecsSinceEpoch(dest.mtime() * 1000), QLocale::ShortFormat);
    if(!disp_size.isEmpty()) {
        infoStr = QString(tr("Type: %1\nSize: %2\nModified: %3"))
                  .arg(dest.description(),
                       disp_size,
                       destMtime);
    }
    else {
        infoStr = QString(tr("Type: %1\nModified: %2"))
                  .arg(dest.description(),
                       destMtime);
    }
    ui->destInfo->setText(infoStr);

    auto basename = path.baseName();
    ui->fileName->setText(QString::fromUtf8(basename.get()));
    int length = ui->fileName->text().lastIndexOf(QStringLiteral("."));
    if(length > 0 && length < ui->fileName->text().size() - 1) {
        ui->fileName->setSelection(0, length);
    }
    else {
        ui->fileName->selectAll();
    }
    oldName_ = QString::fromUtf8(basename.get());
    connect(ui->fileName, &QLineEdit::textChanged, this, &RenameDialog::onFileNameChanged);
    ui->fileName->setFocus(); // needed with Qt >= 6.6.1

    // add "Rename" button
    QAbstractButton* button = ui->buttonBox->button(QDialogButtonBox::Ok);
    button->setText(tr("&Overwrite"));
    // FIXME: there seems to be no way to place the Rename button next to the overwrite one.
    renameButton_ = ui->buttonBox->addButton(tr("&Rename"), QDialogButtonBox::ActionRole);
    connect(renameButton_, &QPushButton::clicked, this, &RenameDialog::onRenameClicked);
    renameButton_->setEnabled(false); // disabled by default

    // do not allow self-overwriting; tell user to choose another name instead
    if(path == src.path()) {
        button->setEnabled(false);
        ui->srcLabel->setVisible(false);
        ui->srcIcon->setVisible(false);
        ui->srcInfo->setVisible(false);
        ui->label->setText(tr("<p><b>The file cannot overwrite itself.</b></p><p>Please select another name.</p>"));
    }

    button = ui->buttonBox->button(QDialogButtonBox::Ignore);
    connect(button, &QPushButton::clicked, this, &RenameDialog::onIgnoreClicked);
}

RenameDialog::~RenameDialog() {
    delete ui;
}

void RenameDialog::onRenameClicked() {
    action_ = ActionRename;
    QDialog::done(QDialog::Accepted);
}

void RenameDialog::onIgnoreClicked() {
    action_ = ActionIgnore;
}

// the overwrite button
void RenameDialog::accept() {
    action_ = ActionOverwrite;
    applyToAll_ = ui->applyToAll->isChecked();
    QDialog::accept();
}

// cancel, or close the dialog
void RenameDialog::reject() {
    action_ = ActionCancel;
    QDialog::reject();
}

void RenameDialog::onFileNameChanged(QString newName) {
    newName_ = newName;
    // FIXME: check if the name already exists in the current dir
    bool hasNewName = (newName_ != oldName_);
    renameButton_->setEnabled(hasNewName);
    renameButton_->setDefault(hasNewName);

    if(!ui->srcInfo->isVisible()) {
        return; // this was a self-overwriting prompt
    }

    // change default button to rename rather than overwrite
    // if the user typed a new filename
    QPushButton* overwriteButton = static_cast<QPushButton*>(ui->buttonBox->button(QDialogButtonBox::Ok));
    overwriteButton->setEnabled(!hasNewName);
    overwriteButton->setDefault(!hasNewName);
}


} // namespace Fm
