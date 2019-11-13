// Copyright 2019 François CROLLET

// This file is part of VPS Player.
// VPS Player is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.
// VPS Player is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with VPS Player. If not, see <http://www.gnu.org/licenses/>.

#include <QTime>

#include "tools.h"


// Converts a value in milliseconds to an "HH:mm:ss" format
QString Tools::convertMSecToText(int milliseconds)
{
  if (milliseconds < 0)
    return QStringLiteral("--:--:--");
  else
    return QTime(0, 0).addMSecs(milliseconds).toString(QStringLiteral("HH:mm:ss"));
}
