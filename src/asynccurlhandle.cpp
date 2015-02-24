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

#include "asynccurlhandle.h"

AsyncCurlHandle::AsyncCurlHandle ( QObject *parent )
  : QThread ( parent ), m_curl( curl_easy_init() )
{

}

void AsyncCurlHandle::reset()
{
  m_curl.reset( curl_easy_init() );
}

void AsyncCurlHandle::run()
{
  m_result = curl_easy_perform( m_curl.get() );
}

