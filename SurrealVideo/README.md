# SurrealVideo

This project is a fork of ffmpeg that strips down the codebase to only contain the
indeo5 codec used by Klingon Honor Guard.

As a fork of ffmpeg, SurrealVideo is licensed under [GNU Lesser General Public License (LGPL) version 2.1](http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html) or later.

Please note that while SurrealEngine itself is using a ZLIB style license (along
some BSD/MIT licenses for other dependencies), anything within this folder falls
under the LGPL.

SurrealEngine's own source code is not affected by the LGPL as it is just a project linking
up against the SurrealVideo dynamic library. SurrealEngine could have linked against
ffmpeg as well, but that library is literally a 100 MB binary. SurrealVideo is about
100 KB.

We only need ffmpeg to play that one video codec.
