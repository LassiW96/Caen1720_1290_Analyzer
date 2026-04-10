#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QProgressBar>
#include <QPushButton>
#include <QTextEdit>

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
  void startTdcDecode();
  void startTiDecode();

  // Analysis Tab Slots
  void browseAnalysisFile();
  void runAnalysisScript();
  void loadAnalysisScript();
  void loadExampleScript();
  void onHistogramSelected(QListWidgetItem *item);

private:
  void setupUi();
  void setupDecodeTab();
  void setupAnalysisTab();

  // Decode Tab Widgets
  QTabWidget *m_tabWidget;
  QLineEdit *m_inputFileEdit;
  QLineEdit *m_outputDirEdit;
  QPushButton *m_decodeBtn;
  QPushButton *m_tdcDecodeBtn;
  QPushButton *m_tiDecodeBtn;
  QProgressBar *m_progressBar;
  QTextEdit *m_logArea;

  // Analysis Tab Widgets
  QLineEdit *m_analysisFileEdit;
  QTextEdit *m_scriptEditor;
  QListWidget *m_histList;
  QLabel *m_previewLabel;

  // Internal state
  QString m_currentRootFile;
  QString m_currentScriptPath;
  QTimer *m_rootTimer;
};

#endif // MAINWINDOW_H
