/* === This file is part of Calamares - <https://calamares.io> ===
 *
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 *   ShedOS LUKS recovery-key escrow — view-step config.
 */

#ifndef RECOVERYKEYQ_CONFIG_H
#define RECOVERYKEYQ_CONFIG_H

#include <QObject>
#include <QString>

class Config : public QObject
{
    Q_OBJECT
    Q_PROPERTY( bool encrypted READ encrypted NOTIFY stateChanged )
    Q_PROPERTY( QString recoveryKey READ recoveryKey NOTIFY stateChanged )
    Q_PROPERTY( bool acknowledged READ acknowledged WRITE setAcknowledged NOTIFY acknowledgedChanged )

public:
    explicit Config( QObject* parent = nullptr );

    bool encrypted() const { return m_encrypted; }
    QString recoveryKey() const { return m_recoveryKey; }
    bool acknowledged() const { return m_acknowledged; }

    // Next is allowed once an unencrypted install (nothing to save) or an
    // encrypted one whose key the user confirmed saving.
    bool isReady() const { return !m_encrypted || m_acknowledged; }

    void setAcknowledged( bool value );

    // Re-read the encryption choice (globalStorage "luksPassphrase", set on
    // the partition page that precedes this step); on the first encrypted
    // activation, generate the recovery key and stash it in globalStorage
    // as "shedos_recovery_key" for the escrow job. Idempotent.
    void refresh();

signals:
    void stateChanged();
    void acknowledgedChanged();
    void readyChanged( bool );

private:
    static QString generateKey();

    bool m_encrypted = false;
    bool m_acknowledged = false;
    QString m_recoveryKey;
};

#endif  // RECOVERYKEYQ_CONFIG_H
