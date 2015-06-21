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

#ifndef ASYNCCURLHANDLE_H
#define ASYNCCURLHANDLE_H

#include "curlcleanup.h"

#include <QtCore/QThread>

class AsyncCurlHandle : public QThread
{
  Q_OBJECT
  
  public:
    explicit AsyncCurlHandle( QObject* parent = 0 );
    
    CURL *get() { return m_curl.get(); }
    
    void reset();
    
    void perform() { start(); }
    CURLcode result() { return m_result; }
    
  protected:
    virtual void run();
    
  private:
    CurlPtr m_curl;
    CURLcode m_result;
};

#endif // ASYNCCURLHANDLE_H
