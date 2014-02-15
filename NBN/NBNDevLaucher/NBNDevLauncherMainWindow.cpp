#include "NBNDevLauncherMainWindow.h"
#include "ui_NBNDevLauncherMainWindow.h"

#include <QApplication>
#include <QProcess>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QClipboard>
#include <QDebug>


class NBNIPathWidgetItem : public QWidget
{
    Q_OBJECT
public:

    NBNIPathWidgetItem(NBNDevLauncherMainWindow *ipLauncherWindow, const QString &iVarName, const QString &iDefaultText) :
        m_VarName(iVarName)
    {
        QHBoxLayout *pLayout(new QHBoxLayout);

        m_pPathLineEdit = new QLineEdit;
        m_pPathLineEdit->setText(iDefaultText);
        pLayout->addWidget(m_pPathLineEdit);

        m_pPathButton = new QToolButton;
        m_pPathButton->setText("...");
        pLayout->addWidget(m_pPathButton);
        setLayout(pLayout);

        pLayout->setMargin(0);
        setContentsMargins(0, 0, 0, 0);

        connect(m_pPathButton, SIGNAL(clicked()), this, SLOT(OnPathButtonClicked()));
        connect(m_pPathLineEdit, SIGNAL(textChanged(QString)), this, SLOT(OnVariableChanged(QString)));
        connect(this, SIGNAL(VariableChangedSig(QString,QString)), ipLauncherWindow, SLOT(OnVariableChanged(QString,QString)));
    }

signals:

    void VariableChangedSig(QString, QString);


protected:

    virtual QString GetPath() = 0;

private slots:

    void OnPathButtonClicked()
    {
        QString p(QDir::cleanPath(GetPath()));
        if ( ! p.isEmpty() )
        {
            m_pPathLineEdit->setText(p);
        }
    }

    void OnVariableChanged(QString iNewValue)
    {
        emit VariableChangedSig(m_VarName, iNewValue);
    }


protected:

    QLineEdit *m_pPathLineEdit;
    QString m_VarName;

private:

    QToolButton *m_pPathButton;
};



class NBNPathWidgetItem : public NBNIPathWidgetItem
{
public:

    NBNPathWidgetItem(NBNDevLauncherMainWindow *ipLauncherWindow, const QString &iVarName, const QString &iDefaultText) :
        NBNIPathWidgetItem(ipLauncherWindow, iVarName, iDefaultText)
    {
    }

protected:

    virtual QString GetPath()
    {
        return QFileDialog::getExistingDirectory(this, "Specify directory path", m_pPathLineEdit->text());
    }
};

class NBNFilePathWidgetItem : public NBNIPathWidgetItem
{
public:

    NBNFilePathWidgetItem(NBNDevLauncherMainWindow *ipLauncherWindow, const QString &iVarName, const QString &iDefaultText) :
        NBNIPathWidgetItem(ipLauncherWindow, iVarName, iDefaultText)
    {
    }

protected:

    virtual QString GetPath()
    {
        return QFileDialog::getExistingDirectory(this, "Specify file path", m_pPathLineEdit->text());
    }
};


#include "NBNDevLauncherMainWindow.moc"

NBNDevLauncherMainWindow::NBNDevLauncherMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::NBNDevLauncherMainWindow)
{
    ui->setupUi(this);
    ui->treeWidgetVariables->setRootIsDecorated(false);

    InitCommons();

#ifdef Q_OS_WIN
    InitWindows();
#elif defined Q_OS_LINUX
    InitLinux();
#endif

    CreateBuildPath();
}

NBNDevLauncherMainWindow::~NBNDevLauncherMainWindow()
{
    delete ui;
}

void NBNDevLauncherMainWindow::AddNbnVariable(const QString &iName, const QString &iValue, EVariableType iType)
{
    m_NbnVariablesMap.insert(iName, iValue);

    QTreeWidgetItem *pTwi(new QTreeWidgetItem(ui->treeWidgetVariables, QStringList()
                                              << iName
                                              ));

    switch (iType)
    {
    case EVariableType_Path:
    {
        ui->treeWidgetVariables->setItemWidget(pTwi, 1, new NBNPathWidgetItem(this, iName, iValue));
        break;
    }
    case EVariableType_FilePath:
    {
        ui->treeWidgetVariables->setItemWidget(pTwi, 1, new NBNFilePathWidgetItem(this, iName, iValue));
        break;
    }
    default:
    {
        pTwi->setText(1, iValue);
        break;
    }
    }

    ui->treeWidgetVariables->addTopLevelItem(pTwi);
    ui->treeWidgetVariables->sortByColumn(0);
    ui->treeWidgetVariables->resizeColumnToContents(0);
    ui->treeWidgetVariables->resizeColumnToContents(1);

}

QString NBNDevLauncherMainWindow::GetOptDir() const
{
#ifdef _WIN32
    return "C:/opt";
#else
    return "/opt";
#endif
}

QString NBNDevLauncherMainWindow::GetQtBaseDir() const
{
    return QString("%1/Qt").arg(GetOptDir());
}

QString NBNDevLauncherMainWindow::GetQtSelectedDir() const
{
    return QString("%1/%2").arg(GetQtBaseDir()).arg(ui->comboBoxQtVersions->currentText());
}

QString NBNDevLauncherMainWindow::GetQtCeator() const
{
    return QString("%1/Tools/QtCreator/bin/qtcreator%2")
            .arg(GetQtSelectedDir())
        #ifdef _WIN32
            .arg(".exe")
        #else
            .arg("")
        #endif
            ;
}

void NBNDevLauncherMainWindow::on_buttonBox_accepted()
{
}

void NBNDevLauncherMainWindow::on_buttonBox_rejected()
{
    qApp->exit(0);
}

void NBNDevLauncherMainWindow::on_toolButtonOpenProject_clicked()
{
    QString cmakeListsTxt(QFileDialog::getOpenFileName(this, "Open CMakeLists.txt", m_NbnVariablesMap.value("NBN_BASE_SRC_DIR"), "CMakeLists.txt"));
    if ( cmakeListsTxt.isEmpty() )
    {
        return;
    }

    ui->lineEditCMakeListsTxt->setText(cmakeListsTxt);


    QFile f(cmakeListsTxt);
    if ( ! f.open(QIODevice::Text | QIODevice::ReadOnly) )
    {
        QMessageBox::warning(this, tr("Could not open file"), tr("Could not open file %1 for reading").arg(cmakeListsTxt));
        return;
    }

    ui->textEditCMakeListsTxt->setText(f.readAll());

    CreateBuildPath();
}

void NBNDevLauncherMainWindow::InitCommons()
{
    QString home(QString("%1/src").arg(QDir::cleanPath(qgetenv("HOME"))));

#ifdef _WIN32
    QFileInfo fi(home);
    if ( ! fi.exists() )
    {
        home += ".lnk";
    }
    fi = QFileInfo(home);
    if ( fi.exists() && fi.isSymLink() )
    {
        home = fi.symLinkTarget();
    }
#endif

    AddNbnVariable("NBN_BASE_SRC_DIR",
                   home,
                   EVariableType_Path);

    QDir qtDir(GetQtBaseDir());
    ui->comboBoxQtVersions->addItems(qtDir.entryList(QStringList() << "Qt*"));
    ui->comboBoxQtVersions->addItem("nonQt");

}

void NBNDevLauncherMainWindow::InitWindows()
{
    ui->dockWidget->setMinimumWidth(300);
    ui->comboBoxCompiler->clear();
    ui->comboBoxCompiler->addItem("vc2012");
    ui->comboBoxCompiler->addItem("vc2010");

    if ( QString(qgetenv("PROCESSOR_ARCHITECTURE")) == "x86" )
    {
        ui->comboBoxArchitecture->setCurrentText("x86");
    }
    else if ( QString(qgetenv("PROCESSOR_ARCHITECTURE")) == "AMD64" )
    {
        ui->comboBoxArchitecture->setCurrentText("x64");
    }
    else
    {
        ui->comboBoxArchitecture->setCurrentText("UnknownArch");
    }


    ui->lineEditCurrentOS->setText(QString("Windows-%1").arg(ui->comboBoxArchitecture->currentText()));

    AddNbnVariable("NBN_BASE_BUILD_DIR", "C:/opt/nbn/builds",
                   EVariableType_Path);

}

void NBNDevLauncherMainWindow::InitLinux()
{
    ui->dockWidget->setMinimumWidth(400);
    ui->comboBoxCompiler->clear();

    QProcess gcc_version;
    gcc_version.start("gcc", QStringList() << "--version");
    gcc_version.waitForFinished();
    QString gcc_name(QString(gcc_version.readAllStandardOutput()).split('\n').at(0)
                     .split( '[' ).at(1) // NBN_TODO output based assumption... find a better way!
                     .split("-branch").at(0)
                     );

    ui->comboBoxCompiler->addItem(gcc_name);

    if ( QString(qgetenv("CPU")) == "x86" )
    {
        ui->comboBoxArchitecture->setCurrentText("x86");
    }
    else if ( QString(qgetenv("CPU")) == "x86_64" )
    {
        ui->comboBoxArchitecture->setCurrentText("x64");
    }
    else
    {
        ui->comboBoxArchitecture->setCurrentText("unknownArch");
    }

    ui->lineEditCurrentOS->setText(QString("Linux-%1").arg(ui->comboBoxArchitecture->currentText()));

    AddNbnVariable("NBN_BASE_BUILD_DIR", "/opt/nbn/builds",
                   EVariableType_Path);
}

void NBNDevLauncherMainWindow::CreateBuildPath()
{
    QString projectPath(QDir::cleanPath(ui->lineEditCMakeListsTxt->text()));
    projectPath.remove("/CMakeLists.txt");
    projectPath.remove(m_NbnVariablesMap.value("NBN_BASE_SRC_DIR"));

    if ( projectPath.isEmpty() )
    {
        projectPath = "UnknownProject";
    }


    ui->lineEditBuildPath->setText(QDir::cleanPath(QString("%1/%2/%3/%4/%5/%6")
                                                   .arg(m_NbnVariablesMap.value("NBN_BASE_BUILD_DIR"))
                                                   .arg(projectPath)
                                                   .arg(ui->comboBoxCompiler->currentText())
                                                   .arg(ui->comboBoxArchitecture->currentText())
                                                   .arg(ui->comboBoxQtVersions->currentText())
                                                   .arg(ui->comboBoxBuildType->currentText())
                                                   ));
}

void NBNDevLauncherMainWindow::on_comboBoxCompiler_currentTextChanged(const QString &arg1)
{
    CreateBuildPath();
}

void NBNDevLauncherMainWindow::on_comboBoxArchitecture_currentTextChanged(const QString &arg1)
{
    CreateBuildPath();
}

void NBNDevLauncherMainWindow::on_comboBoxBuildType_currentTextChanged(const QString &arg1)
{
    CreateBuildPath();
}

void NBNDevLauncherMainWindow::OnVariableChanged(const QString &iVarName, const QString &iValue)
{
    m_NbnVariablesMap.insert(iVarName, iValue);
    CreateBuildPath();
}

void NBNDevLauncherMainWindow::on_pushButtonCMakeGui_clicked()
{
    QProcess::startDetached("cmake-gui", QStringList() << ui->lineEditBuildPath->text());
}

void NBNDevLauncherMainWindow::on_pushButtonSaveCMakeListsTxt_clicked()
{
    QFile fi(ui->lineEditCMakeListsTxt->text());
    if ( ! fi.open(QIODevice::Text | QIODevice::WriteOnly) )
    {
        QMessageBox::warning(this, tr("Could not save file"), tr("Could not open file %1 for writing").arg(ui->lineEditCMakeListsTxt->text()));
        return;
    }

    fi.write(ui->textEditCMakeListsTxt->toPlainText().toLatin1());
}


void NBNDevLauncherMainWindow::on_toolButtonClipboard_clicked()
{
    QApplication::clipboard()->setText(ui->lineEditBuildPath->text());

}


void NBNDevLauncherMainWindow::on_pushButtonQtCreator_clicked()
{
    qDebug() << GetQtCeator();
    QProcess::startDetached(GetQtCeator(), QStringList() << ui->lineEditCMakeListsTxt->text());
}

void NBNDevLauncherMainWindow::on_comboBoxQtVersions_currentTextChanged(const QString &arg1)
{
    CreateBuildPath();
}
