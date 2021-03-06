#include "indexer.h"

#include <QDirIterator>

const qint64 BUFFER_SIZE = 128 * 1024;
const qint8 SHIFT = 2;
const qint64 MAX_CHAR = 256;
const qint32 MAGIC_TRIGRAMS = 20000;

FileTrigrams::FileTrigrams(qint64 fileSize)
    : fileSize(fileSize)
{}

Indexer::Indexer(QString const &directory, QFileSystemWatcher *watcher)
    : watcher(watcher), directory(directory), needStop(false) {}

void Indexer::indexDirectory(FilesTrigrams &filesTrigrams) {
    emit started();
    QDirIterator it(directory, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QFileInfo fileinfo(it.next());
        size += fileinfo.size();
    }
    if (size == 0) {
        emit updateProgress(100);
        return;
    }

    QDirIterator dirIt(directory, QDir::Files, QDirIterator::Subdirectories);
    while (dirIt.hasNext()) {
        if (needStop) {
            break;
        }
        QFileInfo fileInfo(dirIt.next());
        qint64 fileSize = fileInfo.size();
        curSize += fileSize;
        if (!fileInfo.permission(QFile::ReadUser)) {
            continue;
        }
        FileTrigrams fileTrigrams(fileSize);
        QFile file(fileInfo.absoluteFilePath());
        try {
            indexFile(file, fileTrigrams);
            if (fileTrigrams.trigrams.size() >= MAGIC_TRIGRAMS) {
                continue;
            }
            watcher->addPath(fileInfo.absoluteFilePath());
            filesTrigrams.insert(fileInfo.absoluteFilePath(), std::move(fileTrigrams));
            }
        catch(std::logic_error) {

        }
        progress();
     }
     if (needStop) emit interrupted();
     else emit finished();
}

void Indexer::indexFile(QFile &file, FileTrigrams &fileTrigrams) {
    if (!file.open(QIODevice::ReadOnly)) {
        throw std::logic_error("Can't open file " + file.fileName().toStdString());
    }

    char *buffer = new char[BUFFER_SIZE];
    file.read(buffer, SHIFT);

    while (!file.atEnd()) {
        if (needStop) {
            break;
        }
        qint64 size = SHIFT + file.read(buffer + SHIFT, BUFFER_SIZE - SHIFT);
        if (fileTrigrams.trigrams.size() >= MAGIC_TRIGRAMS) {
            break;
        }
        for (qint64 i = 0; i < size - SHIFT; i++) {
            fileTrigrams.trigrams.insert(hashTrigram(buffer + i));
        }
    }
    delete[] buffer;
    file.close();
}

qint32 Indexer::hashTrigram(char *trigramPointer) {
    qint32 hash = 0;
    for (int i = 0; i < 3; ++i) {
        hash *= MAX_CHAR;
        hash += trigramPointer[i];
    }
    return hash;
}

void Indexer::progress() {
    qint8 percent = static_cast<qint8>(100 * curSize / size);
    if (percent > curPercent) {
        curPercent = percent;
        emit updateProgress(percent);
    }
}

void Indexer::stop() {
    needStop = true;
}
