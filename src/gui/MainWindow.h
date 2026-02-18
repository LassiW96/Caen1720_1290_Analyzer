#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QListWidget>
#include <QLabel>
#include <QProgressBar>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Decode Tab Slots
    void browseInputFile();
    void browseOutputDir();
    void startDecode();

    // Analysis Tab Slots
    void browseAnalysisFile();
    void runAnalysisScript();
    void loadAnalysisScript();
    void loadExampleScript();
    void onHistogramSelected(QListWidgetItem *item);
    void openInteractiveCanvas();

private:
    void setupUi();
    void setupDecodeTab();
    void setupAnalysisTab();

    // Decode Tab Widgets
    QTabWidget *m_tabWidget;
    QLineEdit *m_inputFileEdit;
    QLineEdit *m_outputDirEdit;
    QPushButton *m_decodeBtn;
    QProgressBar *m_progressBar;
    QTextEdit *m_logArea;

    // Analysis Tab Widgets
    QLineEdit *m_analysisFileEdit;
    QTextEdit *m_scriptEditor;
    QListWidget *m_histList;
    QLabel *m_previewLabel;
    
    // Internal state
    QString m_currentRootFile;
};

#endif // MAINWINDOW_H
