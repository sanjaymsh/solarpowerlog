This folder contains some supplemental files which are not (or not mandatory) required to run solarpowerlog.

solarpowerlog.init, solarpowerlog.default
The file solarpowerlog.init contains the init-script file (usually copied to "/etc/init.d/solarpowerlog") to start solarpowerlog in daemon mode. It should support any LSB aware distribution, but might
need modifications on other platforms. The file solarpowerlog.default (stored as "/etc/default/solarpowerlog") sourced by the init-file to load default values for the daemon behaviour.

sputnik-simulator/*
This "simulator" is just a help to test solarpowerlog for Sputnik Inveters without actually need to access to an inverter (e.g at night ;-))
It just a reply of an recorded response, so simulator is little to big wording for it. Read the "README" in that directory for usage directions.


The following files target to help my (coldtobi) to develop solarpowerlog. If you have use for them, good! (Note, they will not be released in the tarball)

make-slp-dist
This script is a convenience script for my use to semi-automatically test the generation of new releases and testing the debian package generation. It helps me to always follow the same procedure when I release a new version.
In a few words, it automates the upstream tar-ball generation, tests the tar-ball and then tries to generate a debian package using pbuilder (to check dependencies completeness) and an additional dpkg-buildpackage using this tarball.

buildbot/*
This files are auxiliary files to maintain my local buildbot (http://trac.buildbot.net/) setup. There's an dedicated README in that folder.


