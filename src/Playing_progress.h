// Copyright 2018-2019 François CROLLET

// This file is part of VPS Player.
// VPS Player is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.
// VPS Player is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with VPS Player. If not, see <http://www.gnu.org/licenses/>.

#ifndef PLAYING_PROGRESS_H
#define PLAYING_PROGRESS_H

#include <QMouseEvent>
#include <QProgressBar>


class PlayingProgress : public QProgressBar
{
  Q_OBJECT

private:
  bool is_clickable;

public:
  PlayingProgress(QWidget *parent = nullptr); // Constructor
  ~PlayingProgress(); // Destructor
  void setClickable(bool clickable); // Sets whether the progress bar is clickable

protected:
  void mouseMoveEvent(QMouseEvent *event); // Reimplementation of QWidget's "mouse moved" event handler
  void mousePressEvent(QMouseEvent *event); // Reimplementation of QWidget's "mouse button pressed" event handler

private:
  int convertPixelsToMSec(int pixels) const; // Convert a number of pixels (part of the progress bar's width) into the corresponding duration in milliseconds
  
signals:
  void barClicked(int); // This signal is emitted when the user clicks on the progress bar (as long as it is clickable). Parameter : progression corresponding to the position where the click occured
};

#endif
