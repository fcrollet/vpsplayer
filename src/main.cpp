// Copyright 2018-2019 François CROLLET

// This file is part of VPS Player.
// VPS Player is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.
// VPS Player is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with VPS Player. If not, see <http://www.gnu.org/licenses/>.

#include <QApplication>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QIcon>
#include <QString>
#include <QStringList>

#include "Player_window.h"


int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  QCoreApplication::setApplicationName(QStringLiteral("VPS Player"));
  QCoreApplication::setApplicationVersion(QStringLiteral(VERSION_STRING));
  const QIcon app_icon(QStringLiteral(":/vps-64.png"));
  app.setWindowIcon(app_icon);
  
  QCommandLineParser parser;
  parser.setApplicationDescription("High quality Variable Pitch and Speed audio player");
  parser.addHelpOption();
  parser.addVersionOption();
  parser.addPositionalArgument("file", "Audio file to open (optional)", "[file]");
  parser.process(app);
  const QStringList arguments = parser.positionalArguments();
  QString filename;
  if (!arguments.isEmpty())
    filename = arguments.first();
  
  PlayerWindow window(app_icon, filename);
  window.show();
  return app.exec();
}
