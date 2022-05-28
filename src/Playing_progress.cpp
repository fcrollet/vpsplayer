// Copyright 2018-2022 François CROLLET

// This file is part of VPS Player.
// VPS Player is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.
// VPS Player is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with VPS Player. If not, see <http://www.gnu.org/licenses/>.

#include <QToolTip>

#include "Playing_progress.h"
#include "tools.h"


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
  else {
    setCursor(Qt::ForbiddenCursor);
    if (underMouse())
      QToolTip::hideText();
  }

  setMouseTracking(clickable);

  is_clickable = clickable;
}


// Reimplementation of QWidget's "mouse moved" event handler
void PlayingProgress::mouseMoveEvent(QMouseEvent *event)
{
  if (is_clickable)
    QToolTip::showText(event->globalPosition().toPoint(), Tools::convertMSecToText(mouseEventPosition(event)));

  event->accept();
}
  

// Reimplementation of QWidget's "mouse button pressed" event handler
void PlayingProgress::mousePressEvent(QMouseEvent *event)
{
  if ((event->button() == Qt::LeftButton) && is_clickable)
    emit barClicked(mouseEventPosition(event));

  event->accept();
}


// Returns the position in milliseconds corresponding to the mouse position on the progress bar where the event occured
int PlayingProgress::mouseEventPosition(const QMouseEvent *event) const
{
  return qRound(event->position().x()) * (maximum() / width());
}
