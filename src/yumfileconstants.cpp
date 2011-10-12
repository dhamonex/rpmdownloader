/*
RpmDownloader
Copyright (C) 2011  dirk.hartmann@liquid-co.de

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#include "yumfileconstants.h"

namespace Yum {
  const QString filelistGzFileName("filelists.xml.gz");
  const QString filelistFileName("filelists.xml");
  const QString tmpFilelistGzFileName("tmpFilelists.xml.gz");
  const QString tmpFilelistFileName("tmpFilelists.xml");

  const QString otherGzFileName("other.xml.gz");
  const QString otherFileName("other.xml");
  const QString tmpOtherGzFileName("tmpOther.xml.gz");
  const QString tmpOtherFileName("tmpOther.xml");

  const QString primaryGzFileName("primary.xml.gz");
  const QString primaryFileName("primary.xml");
  const QString tmpPrimaryGzFileName("tmpPrimary.xml.gz");
  const QString tmpPrimaryFileName("tmpPrimary.xml");

  const QString repoServerDir("repodata");
  const QString repomdFileName("repomd.xml");
}