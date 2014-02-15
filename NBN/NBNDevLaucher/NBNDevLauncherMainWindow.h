#ifndef NBNDEVLAUNCHERMAINWINDOW_H
#define NBNDEVLAUNCHERMAINWINDOW_H

#include <QMainWindow>
#include <QMap>

namespace Ui {
class NBNDevLauncherMainWindow;
}

class NBNIPathWidgetItem;
class NBNDevLauncherMainWindow : public QMainWindow
{
    Q_OBJECT

    friend class NBNIPathWidgetItem;

public:
    explicit NBNDevLauncherMainWindow(QWidget *parent = 0);
    ~NBNDevLauncherMainWindow();

    enum EVariableType
    {
        EVariableType_String = 0,
        EVariableType_Path,
        EVariableType_FilePath,
        EVariableType_Count
    };

    void AddNbnVariable(const QString &iName, const QString &iValue, EVariableType iType);

    QString GetOptDir() const;
    QString GetQtBaseDir() const;
    QString GetQtSelectedDir() const;
    QString GetQtCeator() const;


private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

    void on_toolButtonOpenProject_clicked();

    void on_comboBoxCompiler_currentTextChanged(const QString &arg1);

    void on_comboBoxArchitecture_currentTextChanged(const QString &arg1);

    void on_comboBoxBuildType_currentTextChanged(const QString &arg1);

    void OnVariableChanged(const QString &iVarName, const QString &iValue);

    void on_pushButtonCMakeGui_clicked();

    void on_pushButtonSaveCMakeListsTxt_clicked();

    void on_toolButtonClipboard_clicked();

    void on_pushButtonQtCreator_clicked();

    void on_comboBoxQtVersions_currentTextChanged(const QString &arg1);

private:

    void InitCommons();
    void InitWindows();
    void InitLinux();
    void CreateBuildPath();



    Ui::NBNDevLauncherMainWindow *ui;

    QMap<QString, QString> m_NbnVariablesMap;
};

#endif // NBNDEVLAUNCHERMAINWINDOW_H
