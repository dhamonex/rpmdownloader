/***************************************************************************
 *   Copyright (C) 2015 by Dirk Hartmann                                   *
 *   2monex@gmx.net                                                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef CURLCLEANUP_H
#define CURLCLEANUP_H

#include <curl/curl.h>

#include <memory>

struct CurlCleanup
{
  void operator()( CURL *curl )
  {
    if ( curl ) {
      curl_easy_cleanup( curl );
    }
  }
};

typedef std::unique_ptr<CURL, CurlCleanup> CurlPtr;

#endif // CURLCLEANUP_H