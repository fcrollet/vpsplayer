// Copyright 2018-2025 François CROLLET

// This file is part of VPS Player.
// VPS Player is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.
// VPS Player is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with VPS Player. If not, see <http://www.gnu.org/licenses/>.

#ifndef PLAYER_WINDOW_H
#define PLAYER_WINDOW_H

#include <QMainWindow>
#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QFileInfo>
#include <QIcon>
#include <QLabel>
#include <QLCDNumber>
#include <QPushButton>
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
  QPushButton *button_open;
  QPushButton *button_cancel;
  QPushButton *button_play;
  QPushButton *button_pause;
  QPushButton *button_stop;
  QPushButton *button_bwd10;
  QPushButton *button_bwd5;
  QPushButton *button_fwd5;
  QPushButton *button_fwd10;
  QSpinBox *spinbox_pitch;
  QLabel *label_speed_value;
  QLCDNumber *lcd_volume;
  QComboBox *combobox_engine;
  QCheckBox *check_high_quality;
  QCheckBox *check_channels_together;
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
  void moveReadingPosition(int delta); // Moves reading position backward or forward. Parameter: position change in milliseconds
  void showAbout(); // Displays "About" dialog window
  void updateDuration(int duration); // Updates total file duration
  void updatePitch(int pitch); // Updates the pitch
  void updateReadingPosition(int position); // Updates current reading position
  void updateSpeed(int speed); // Updates the speed
  void updateStatus(AudioPlayer::Status status); // Updates the window based on the player status
  void updateVolume(int volume); // Updates the volume
};

#endif
