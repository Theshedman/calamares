/* === This file is part of Calamares - <https://calamares.io> ===
 *
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 *   ShedOS LUKS recovery-key escrow — gated view step.
 */

#ifndef RECOVERYKEYQVIEWSTEP_H
#define RECOVERYKEYQVIEWSTEP_H

#include "Config.h"

#include "DllMacro.h"
#include "utils/PluginFactory.h"
#include "viewpages/QmlViewStep.h"

#include <QObject>

class PLUGINDLLEXPORT RecoveryKeyViewStep : public Calamares::QmlViewStep
{
    Q_OBJECT

public:
    explicit RecoveryKeyViewStep( QObject* parent = nullptr );

    QString prettyName() const override;

    bool isNextEnabled() const override;
    bool isBackEnabled() const override;
    bool isAtBeginning() const override;
    bool isAtEnd() const override;

    void onActivate() override;

    QObject* getConfig() override { return m_config; }

private:
    Config* m_config;
};

CALAMARES_PLUGIN_FACTORY_DECLARATION( RecoveryKeyViewStepFactory )

#endif  // RECOVERYKEYQVIEWSTEP_H
