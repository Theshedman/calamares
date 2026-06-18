# === This file is part of Calamares - <https://calamares.io> ===
#
#   SPDX-License-Identifier: BSD-2-Clause
#
# ShedOS recovery-key view step: generates a LUKS recovery key, shows it,
# and blocks Next until the user confirms they saved it. The shedos_luks_escrow
# job enrolls the stashed key as a second keyslot in the exec phase.
if(NOT WITH_QML)
    calamares_skip_module( "recoverykeyq (QML is not supported in this build)" )
    return()
endif()

calamares_add_plugin(recoverykeyq
    TYPE viewmodule
    EXPORT_MACRO PLUGINDLLEXPORT_PRO
    SOURCES
        RecoveryKeyViewStep.cpp
        Config.cpp
    RESOURCES
        recoverykeyq.qrc
    SHARED_LIB
)
