#ifndef INDEXER_H
#define INDEXER_H

#include <QFile>
#include <QFileSystemWatcher>
#include <QObject>
#include <QSet>
#include <QMap>

struct FileTrigrams
{
    FileTrigrams() = delete;
    FileTrigrams(qint64 fileSize);
    qint64 fileSize;
    QSet<qint32> trigrams;
};

using FilesTrigrams = QMap<QString, FileTrigrams>;

class Indexer : public QObject {
    Q_OBJECT

public:
    explicit Indexer(QString const &dir, QFileSystemWatcher *watcher);

    void indexDirectory(FilesTrigrams &filesTrigrams);
    void indexFile(QFile &file, FileTrigrams &fileTrigrams);
    qint32 hashTrigram(char *trigramPointer);


signals:
    void started();
    void finished();
    void interrupted();
    void updateProgress(qint8 progress);

public slots:
    void stop();

private:
    void progress();
    QFileSystemWatcher *watcher;
    QString directory;
    qint64 size = 0, curSize = 0;
    qint8 curPercent = 0;
    bool needStop;
};

#endif // INDEXER_H
