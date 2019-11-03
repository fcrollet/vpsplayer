// Copyright 2018-2019 François CROLLET

// This file is part of VPS Player.
// VPS Player is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.
// VPS Player is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with VPS Player. If not, see <http://www.gnu.org/licenses/>.

#ifndef PLAYER_WINDOW_H
#define PLAYER_WINDOW_H

#include <QMainWindow>
#include <QAction>
#include <QCheckBox>
#include <QFileInfo>
#include <QIcon>
#include <QLabel>
#include <QLCDNumber>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QString>

#include "Audio_player.h"
#include "Playing_progress.h"


class PlayerWindow : public QMainWindow
{
  Q_OBJECT

private:
  AudioPlayer *audio_player;
  QAction *action_open;
  QAction *action_quit;
  QAction *action_about;
  QAction *action_about_qt;
  QPushButton *button_open;
  QPushButton *button_cancel;
  QPushButton *button_play;
  QPushButton *button_pause;
  QPushButton *button_stop;
  QSlider *slider_pitch;
  QSlider *slider_speed;
  QSlider *slider_volume;
  QSpinBox *spinbox_pitch;
  QLabel *label_speed_value;
  QLCDNumber *lcd_volume;
  QCheckBox *check_high_quality;
  QCheckBox *check_formant_preserved;
  PlayingProgress *progress_playing;
  QLabel *label_reading_progress;
  QLabel *label_duration;
  QLabel *label_status;
  QLabel *label_loading_progress;
  QString music_directory;
  
public:
  PlayerWindow(const QIcon &app_icon, const QString &filename = QString()); // Constructor
  ~PlayerWindow(); // Destructor

private:
  void displayAudioDecodingError(QAudioDecoder::Error error); // Prompt an error popup for an audio decoding error
  void displayAudioDeviceError(QAudio::Error error); // Prompt an error popup for an audio device error
  void openFile(const QFileInfo &file_info); // Open file given in parameter
  void openFileFromSelector(); // Open a new file (chosen with a file selector)
  void playAudio(); // Start or resume audio playing
  void showAbout(); // Displays "About" dialog window
  void updateDuration(int duration); // Updates total file duration
  void updatePitch(int pitch); // Updates the pitch
  void updateReadingPosition(int position); // Updates current reading position
  QString convertMSecToText(int milliseconds) const; // Converts a value in milliseconds to an "HH:mm:ss" format
  void updateSpeed(int speed); // Updates the speed
  void updateStatus(AudioPlayer::Status status); // Updates the window based on the player status
  void updateVolume(int volume); // Updates the volume
};

#endif
