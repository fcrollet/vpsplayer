// Copyright 2018-2019 François CROLLET

// This file is part of VPS Player.
// VPS Player is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.
// VPS Player is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with VPS Player. If not, see <http://www.gnu.org/licenses/>.

#include "Playing_progress.h"


// Constructor
PlayingProgress::PlayingProgress(QWidget *parent) : QProgressBar(parent),
						    is_clickable(false)
{
  setTextVisible(false);
}


// Destructor
PlayingProgress::~PlayingProgress()
{

}


// Sets whether the progress bar is clickable
void PlayingProgress::setClickable(bool clickable)
{
  if (clickable)
    setCursor(Qt::PointingHandCursor);
  else
    setCursor(Qt::ForbiddenCursor);

  is_clickable = clickable;
}


// Reimplementation of QWidget's "mouse button pressed" event handler
void PlayingProgress::mousePressEvent(QMouseEvent *event)
{
  if ((event->button() == Qt::LeftButton) && is_clickable)
    emit barClicked(event->x() * (maximum() / width()));

  event->accept();
}
