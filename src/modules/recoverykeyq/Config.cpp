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
    // 25 chars from a deliberately unambiguous 24-symbol alphabet, grouped
    // 5x5 with dashes. The user reads this off the screen and re-types it at
    // the BLIND LUKS boot prompt (no echo), so every look-alike pair is a
    // silent-rejection trap: dropped are I/L/O/Q/U (read as 1/0/V) and the
    // digits 2/5/6 (read as Z/S/G), leaving only glyphs that survive hand
    // transcription. ~114 bits of CSPRNG entropy — far beyond brute force
    // through LUKS2's argon2 KDF.
    static const char alphabet[] = "ABCDEFGHJKMNPRSTVWXYZ347";
    constexpr int n = sizeof( alphabet ) - 1;  // 24
    QString out;
    for ( int i = 0; i < 25; ++i )
    {
        if ( i > 0 && i % 5 == 0 )
        {
            out += QLatin1Char( '-' );
        }
        out += QLatin1Char( alphabet[ QRandomGenerator::system()->bounded( n ) ] );
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
