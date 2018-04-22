// Copyright (c) 2011-2014 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef UTOPIACOIN_QT_UTOPIACOINADDRESSVALIDATOR_H
#define UTOPIACOIN_QT_UTOPIACOINADDRESSVALIDATOR_H

#include <QValidator>

/** Base58 entry widget validator, checks for valid characters and
 * removes some whitespace.
 */
class UtopiacoinAddressEntryValidator : public QValidator
{
    Q_OBJECT

public:
    explicit UtopiacoinAddressEntryValidator(QObject *parent);

    State validate(QString &input, int &pos) const;
};

/** Utopiacoin address widget validator, checks for a valid utopiacoin address.
 */
class UtopiacoinAddressCheckValidator : public QValidator
{
    Q_OBJECT

public:
    explicit UtopiacoinAddressCheckValidator(QObject *parent);

    State validate(QString &input, int &pos) const;
};

#endif // UTOPIACOIN_QT_UTOPIACOINADDRESSVALIDATOR_H
