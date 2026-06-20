/* === This file is part of Calamares - <https://calamares.io> ===
 *
 *   SPDX-FileCopyrightText: 2016 Teo Mrnjavac <teo@kde.org>
 *   SPDX-FileCopyrightText: 2020 Adriaan de Groot <groot@kde.org>
 *   SPDX-FileCopyrightText: 2023 Evan James <dalto@fastmail.com>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 *   Calamares is Free Software: see the License-Identifier above.
 *
 */

#include "EncryptWidget.h"

#include "ui_EncryptWidget.h"

#include "Branding.h"
#include "utils/Gui.h"
#include "utils/Retranslator.h"

#include <QShowEvent>
#include <QTimer>

constexpr int ZFS_MIN_LENGTH = 8;

/** @brief Does this system support whole-disk encryption?
 *
 * Returns @c true if the system is likely to support encryption
 * with sufficient performance to be usable. A machine that can't
 * doe hardware-assisted AES is **probably** too slow, so we could
 * warn the user that ticking the "encrypt system" box is a bad
 * idea.
 *
 * Since we don't have an oracle that can answer that question,
 * just pretend every system can do it.
 */
static inline bool
systemSupportsEncryptionAcceptably()
{
    return true;
}

EncryptWidget::EncryptWidget( QWidget* parent )
    : QWidget( parent )
    , m_ui( new Ui::EncryptWidget )
    , m_state( Encryption::Disabled )
{
    m_ui->setupUi( this );

    m_ui->m_iconLabel->setFixedWidth( m_ui->m_iconLabel->height() );
    m_ui->m_passphraseLineEdit->hide();
    m_ui->m_confirmLineEdit->hide();
    m_ui->m_iconLabel->hide();
    // TODO: this deserves better rendering, an icon or something, but that will
    //       depend on having a non-bogus implementation of systemSupportsEncryptionAcceptably
    if ( systemSupportsEncryptionAcceptably() )
    {
        m_ui->m_encryptionUnsupportedLabel->hide();
    }
    else
    {
        // This is really ugly, but the character is unicode "unlocked"
        m_ui->m_encryptionUnsupportedLabel->setText( QStringLiteral( "🔓" ) );
        m_ui->m_encryptionUnsupportedLabel->show();
    }

    connect(
        m_ui->m_encryptCheckBox, Calamares::checkBoxStateChangedSignal, this, &EncryptWidget::onCheckBoxStateChanged );
    connect( m_ui->m_passphraseLineEdit, &QLineEdit::textEdited, this, &EncryptWidget::onPassphraseEdited );
    connect( m_ui->m_confirmLineEdit, &QLineEdit::textEdited, this, &EncryptWidget::onPassphraseEdited );

    setFixedHeight( m_ui->m_passphraseLineEdit->height() );  // Avoid jumping up and down
    updateState();

    CALAMARES_RETRANSLATE_SLOT( &EncryptWidget::retranslate );
}

bool
EncryptWidget::isEncryptionCheckboxChecked()
{
    return m_ui->m_encryptCheckBox->isChecked();
}

void
EncryptWidget::setEncryptionCheckbox( bool preCheckEncrypt )
{
    m_ui->m_encryptCheckBox->setChecked( preCheckEncrypt );
}

void
EncryptWidget::reset( bool checkVisible )
{
    m_ui->m_passphraseLineEdit->clear();
    m_ui->m_confirmLineEdit->clear();

    m_ui->m_encryptCheckBox->setChecked( false );

    m_ui->m_encryptCheckBox->setVisible( checkVisible );
    m_ui->m_passphraseLineEdit->setVisible( !checkVisible );
    m_ui->m_confirmLineEdit->setVisible( !checkVisible );
}

EncryptWidget::Encryption
EncryptWidget::state() const
{
    Encryption newState = Encryption::Unconfirmed;

    if ( m_ui->m_encryptCheckBox->isChecked() || !m_ui->m_encryptCheckBox->isVisible() )
    {
        if ( !m_ui->m_passphraseLineEdit->text().isEmpty()
             && m_ui->m_passphraseLineEdit->text() == m_ui->m_confirmLineEdit->text() )
        {
            newState = Encryption::Confirmed;
        }
        else
        {
            newState = Encryption::Unconfirmed;
        }
    }
    else
    {
        newState = Encryption::Disabled;
    }

    return newState;
}

void
EncryptWidget::setText( const QString& text )
{
    m_ui->m_encryptCheckBox->setText( text );
}

QString
EncryptWidget::passphrase() const
{
    if ( m_state == Encryption::Confirmed )
    {
        return m_ui->m_passphraseLineEdit->text();
    }
    return QString();
}

void
EncryptWidget::retranslate()
{
    m_ui->retranslateUi( this );
    onPassphraseEdited();  // For the tooltip
}

///@brief Give @p label the @p pixmap from the standard-pixmaps
static void
applyPixmap( QLabel* label, Calamares::ImageType pixmap )
{
    label->setFixedWidth( label->height() );
    label->setPixmap( Calamares::defaultPixmap( pixmap, Calamares::Original, label->size() ) );
}

void
EncryptWidget::updateState( const bool notify )
{
    // The affordance lives on the inputs: a red border whenever encryption is
    // on but the passphrase isn't a confirmed match. A row-internal label gets
    // squeezed to nothing in the single fixed-height row, so it could never be
    // the cue; the border makes the empty-and-required state obvious before the
    // user even types. Catppuccin red to match the installer theme; an empty
    // sheet reverts to the theme default once the fields match.
    static const QString invalidEditQss = QStringLiteral(
        "QLineEdit { border: 2px solid #f38ba8; border-radius: 3px; padding: 2px; }" );
    static const QString validEditQss = QString();

    if ( m_ui->m_passphraseLineEdit->isVisible() )
    {
        QString p1 = m_ui->m_passphraseLineEdit->text();
        QString p2 = m_ui->m_confirmLineEdit->text();

        if ( p1.isEmpty() && p2.isEmpty() )
        {
            applyPixmap( m_ui->m_iconLabel, Calamares::StatusWarning );
            m_ui->m_iconLabel->setToolTip( tr( "Please enter the same passphrase in both boxes.", "@tooltip" ) );
            m_ui->m_passphraseLineEdit->setStyleSheet( invalidEditQss );
            m_ui->m_confirmLineEdit->setStyleSheet( invalidEditQss );
        }
        else if ( m_filesystem == FileSystem::Zfs && p1.length() < ZFS_MIN_LENGTH )
        {
            applyPixmap( m_ui->m_iconLabel, Calamares::StatusError );
            m_ui->m_iconLabel->setToolTip(
                tr( "Password must be a minimum of %1 characters.", "@tooltip" ).arg( ZFS_MIN_LENGTH ) );
            m_ui->m_passphraseLineEdit->setStyleSheet( invalidEditQss );
            m_ui->m_confirmLineEdit->setStyleSheet( invalidEditQss );
        }
        else if ( p1 == p2 )
        {
            applyPixmap( m_ui->m_iconLabel, Calamares::StatusOk );
            m_ui->m_iconLabel->setToolTip( QString() );
            m_ui->m_passphraseLineEdit->setStyleSheet( validEditQss );
            m_ui->m_confirmLineEdit->setStyleSheet( validEditQss );
        }
        else
        {
            applyPixmap( m_ui->m_iconLabel, Calamares::StatusError );
            m_ui->m_iconLabel->setToolTip( tr( "Please enter the same passphrase in both boxes.", "@tooltip" ) );
            m_ui->m_passphraseLineEdit->setStyleSheet( invalidEditQss );
            m_ui->m_confirmLineEdit->setStyleSheet( invalidEditQss );
        }
    }

    Encryption newState = state();

    m_state = newState;
    if ( notify )
    {
        Q_EMIT stateChanged( m_state );
    }
}

void
EncryptWidget::onPassphraseEdited()
{
    if ( !m_ui->m_iconLabel->isVisible() )
    {
        m_ui->m_iconLabel->show();
    }

    updateState();
}

void
EncryptWidget::onCheckBoxStateChanged( Calamares::checkBoxStateType checked )
{
    const bool visible = ( checked != Calamares::checkBoxUncheckedValue );
    m_ui->m_passphraseLineEdit->setVisible( visible );
    m_ui->m_confirmLineEdit->setVisible( visible );
    m_ui->m_iconLabel->setVisible( visible );
    m_ui->m_passphraseLineEdit->clear();
    m_ui->m_confirmLineEdit->clear();

    // Don't clear the icon: updateState() repaints the warning pixmap right
    // below, and blanking it first leaves the row without the cue that the
    // empty fields still need a passphrase.
    updateState();
}

void
EncryptWidget::setFilesystem( const FileSystem::Type fs )
{
    m_filesystem = fs;
    if ( m_state != Encryption::Disabled )
    {
        updateState( false );
    }
}

void
EncryptWidget::setPassphraseFocus()
{
    if ( m_ui->m_passphraseLineEdit->isVisible() )
    {
        m_ui->m_passphraseLineEdit->setFocus( Qt::OtherFocusReason );
    }
}

void
EncryptWidget::showEvent( QShowEvent* event )
{
    QWidget::showEvent( event );
    // Focus the passphrase field the moment the page is shown — but one
    // event-loop turn late, so it wins against the focus Calamares' ViewManager
    // puts on the page/Next button when the step activates. Self-correcting on
    // every show (Back→Next re-focuses); no-op when the field is hidden.
    if ( m_ui->m_passphraseLineEdit->isVisible() )
    {
        QTimer::singleShot( 0, this, [ this ] { setPassphraseFocus(); } );
    }
}
