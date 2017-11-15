#include "vfile.h"

#include <QDir>
#include <QTextEdit>
#include <QFileInfo>
#include "utils/vutils.h"

VFile::VFile(QObject *p_parent,
             const QString &p_name,
             FileType p_type,
             bool p_modifiable,
             QDateTime p_createdTimeUtc,
             QDateTime p_modifiedTimeUtc)
    : QObject(p_parent),
      m_name(p_name),
      m_opened(false),
      m_docType(VUtils::docTypeFromName(p_name)),
      m_type(p_type),
      m_modifiable(p_modifiable),
      m_createdTimeUtc(p_createdTimeUtc),
      m_modifiedTimeUtc(p_modifiedTimeUtc)
{
}

VFile::~VFile()
{
}

bool VFile::open()
{
    if (m_opened) {
        return true;
    }

    Q_ASSERT(!m_name.isEmpty());
    Q_ASSERT(m_content.isEmpty());

    QString filePath = fetchPath();
    Q_ASSERT(QFileInfo::exists(filePath));
    m_content = VUtils::readFileFromDisk(filePath);
    m_lastModified = QFileInfo(filePath).lastModified();
    m_opened = true;
    return true;
}

void VFile::close()
{
    if (!m_opened) {
        return;
    }

    m_content.clear();
    m_opened = false;
}

bool VFile::save()
{
    Q_ASSERT(m_opened);
    Q_ASSERT(m_modifiable);
    bool ret = VUtils::writeFileToDisk(fetchPath(), m_content);
    if (ret) {
        m_lastModified = QFileInfo(fetchPath()).lastModified();
        m_modifiedTimeUtc = QDateTime::currentDateTimeUtc();
    }

    return ret;
}

QUrl VFile::getBaseUrl() const
{
    // Need to judge the path: Url, local file, resource file.
    QUrl baseUrl;
    QString basePath = fetchBasePath();
    QFileInfo pathInfo(basePath);
    if (pathInfo.exists()) {
        if (pathInfo.isNativePath()) {
            // Local file.
            baseUrl = QUrl::fromLocalFile(basePath + QDir::separator());
        } else {
            // Resource file.
            baseUrl = QUrl("qrc" + basePath + QDir::separator());
        }
    } else {
        // Url.
        baseUrl = QUrl(basePath + QDir::separator());
    }

    return baseUrl;
}

bool VFile::isInternalImageFolder(const QString &p_path) const
{
    return VUtils::equalPath(VUtils::basePathFromPath(p_path),
                             fetchBasePath())
           || VUtils::equalPath(p_path, fetchImageFolderPath());
}

bool VFile::isChangedOutside() const
{
    QDateTime lm = QFileInfo(fetchPath()).lastModified();
    return  lm != m_lastModified;
}

void VFile::reload()
{
    Q_ASSERT(m_opened);

    QString filePath = fetchPath();
    Q_ASSERT(QFileInfo::exists(filePath));
    m_content = VUtils::readFileFromDisk(filePath);
    m_lastModified = QFileInfo(filePath).lastModified();
}
