
Debian
====================
This directory contains files used to package utopiacoind/utopiacoin-qt
for Debian-based Linux systems. If you compile utopiacoind/utopiacoin-qt yourself, there are some useful files here.

## utopiacoin: URI support ##


utopiacoin-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install utopiacoin-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your utopiacoin-qt binary to `/usr/bin`
and the `../../share/pixmaps/utopiacoin128.png` to `/usr/share/pixmaps`

utopiacoin-qt.protocol (KDE)

