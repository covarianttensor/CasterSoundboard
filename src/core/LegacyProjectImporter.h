#ifndef LEGACYPROJECTIMPORTER_H
#define LEGACYPROJECTIMPORTER_H

#include <QByteArray>
#include <QJsonObject>
#include <QString>

/**
 * @brief LegacyProjectImporter decodes v1 binary .caster project files (Qt 5 QDataStream format)
 * into modern v2 JSON project schemas, preserving sounds, cues, volume, hotkeys, gradients,
 * and fork-specific actions (Push-To-Talk and Mute).
 *
 * NOTE: Application window geometry is intentionally never imported to avoid overriding
 * the user's current display layout.
 */
class LegacyProjectImporter
{
public:
    struct ImportResult {
        bool success = false;
        QString errorMessage;
        QJsonObject projectJson;
        bool hasPttProfiles = false;
        int importedPttProfilesCount = 0;
    };

    /**
     * @brief Checks if the data begins with a Qt 5 QDataStream QString length header (binary format)
     * rather than JSON text.
     */
    static bool isLegacyBinary(const QByteArray &data);

    /**
     * @brief Checks if a file on disk is a legacy binary .caster project file.
     */
    static bool isLegacyFile(const QString &filePath);

    /**
     * @brief Decodes binary .caster stream data into a v2 QJsonObject root schema.
     */
    static ImportResult importLegacyProject(const QByteArray &data);
};

#endif // LEGACYPROJECTIMPORTER_H
