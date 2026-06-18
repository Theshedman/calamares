/* === This file is part of Calamares - <https://calamares.io> ===
 *
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 *   ShedOS LUKS recovery-key escrow — view-step config.
 */

#include "Config.h"

#include "GlobalStorage.h"
#include "JobQueue.h"

#include <QRandomGenerator>

Config::Config( QObject* parent )
    : QObject( parent )
{
}

void
Config::setAcknowledged( bool value )
{
    if ( m_acknowledged == value )
    {
        return;
    }
    m_acknowledged = value;
    emit acknowledgedChanged();
    emit readyChanged( isReady() );
}

QString
Config::generateKey()
{
    // 25 chars of RFC4648 base32 (A-Z, 2-7), grouped 5x5 = 125 bits of
    // CSPRNG entropy, dash-separated so it can be written down and re-typed.
    static const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
    QString out;
    for ( int i = 0; i < 25; ++i )
    {
        if ( i > 0 && i % 5 == 0 )
        {
            out += QLatin1Char( '-' );
        }
        out += QLatin1Char( alphabet[ QRandomGenerator::system()->bounded( 32 ) ] );
    }
    return out;
}

void
Config::refresh()
{
    auto* gs = Calamares::JobQueue::instance()->globalStorage();
    // The partition page stores the (obscured) passphrase here when the
    // user keeps "Encrypt system" ticked; empty means encryption is off.
    const bool nowEncrypted = !gs->value( "luksPassphrase" ).toString().isEmpty();

    if ( nowEncrypted && m_recoveryKey.isEmpty() )
    {
        m_recoveryKey = generateKey();
        gs->insert( "shedos_recovery_key", m_recoveryKey );
    }
    m_encrypted = nowEncrypted;

    emit stateChanged();
    emit readyChanged( isReady() );
}
