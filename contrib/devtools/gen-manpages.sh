#!/bin/bash

TOPDIR=${TOPDIR:-$(git rev-parse --show-toplevel)}
SRCDIR=${SRCDIR:-$TOPDIR/src}
MANDIR=${MANDIR:-$TOPDIR/doc/man}

UTOPIACOIND=${UTOPIACOIND:-$SRCDIR/utopiacoind}
UTOPIACOINCLI=${UTOPIACOINCLI:-$SRCDIR/utopiacoin-cli}
UTOPIACOINTX=${UTOPIACOINTX:-$SRCDIR/utopiacoin-tx}
UTOPIACOINQT=${UTOPIACOINQT:-$SRCDIR/qt/utopiacoin-qt}

[ ! -x $UTOPIACOIND ] && echo "$UTOPIACOIND not found or not executable." && exit 1

# The autodetected version git tag can screw up manpage output a little bit
UTCVER=($($UTOPIACOINCLI --version | head -n1 | awk -F'[ -]' '{ print $6, $7 }'))

# Create a footer file with copyright content.
# This gets autodetected fine for utopiacoind if --version-string is not set,
# but has different outcomes for utopiacoin-qt and utopiacoin-cli.
echo "[COPYRIGHT]" > footer.h2m
$UTOPIACOIND --version | sed -n '1!p' >> footer.h2m

for cmd in $UTOPIACOIND $UTOPIACOINCLI $UTOPIACOINTX $UTOPIACOINQT; do
  cmdname="${cmd##*/}"
  help2man -N --version-string=${UTCVER[0]} --include=footer.h2m -o ${MANDIR}/${cmdname}.1 ${cmd}
  sed -i "s/\\\-${UTCVER[1]}//g" ${MANDIR}/${cmdname}.1
done

rm -f footer.h2m
