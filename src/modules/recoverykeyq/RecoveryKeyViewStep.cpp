/* === This file is part of Calamares - <https://calamares.io> ===
 *
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 *   ShedOS LUKS recovery-key escrow — gated view step.
 */

#include "RecoveryKeyViewStep.h"

CALAMARES_PLUGIN_FACTORY_DEFINITION( RecoveryKeyViewStepFactory, registerPlugin< RecoveryKeyViewStep >(); )

RecoveryKeyViewStep::RecoveryKeyViewStep( QObject* parent )
    : Calamares::QmlViewStep( parent )
    , m_config( new Config( this ) )
{
    connect( m_config, &Config::readyChanged, this, &RecoveryKeyViewStep::nextStatusChanged );
    emit nextStatusChanged( isNextEnabled() );
}

QString
RecoveryKeyViewStep::prettyName() const
{
    return tr( "Recovery Key" );
}

bool
RecoveryKeyViewStep::isNextEnabled() const
{
    return m_config->isReady();
}

bool
RecoveryKeyViewStep::isBackEnabled() const
{
    return true;
}

bool
RecoveryKeyViewStep::isAtBeginning() const
{
    return true;
}

bool
RecoveryKeyViewStep::isAtEnd() const
{
    return true;
}

void
RecoveryKeyViewStep::onActivate()
{
    Calamares::QmlViewStep::onActivate();
    // The encryption choice + passphrase are recorded on the partition page,
    // which precedes this step — detect and generate the key now that it
    // is known, and re-evaluate whether Next should be enabled.
    m_config->refresh();
    emit nextStatusChanged( isNextEnabled() );
}
