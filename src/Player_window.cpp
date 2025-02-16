// Copyright 2018-2025 François CROLLET

// This file is part of VPS Player.
// VPS Player is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.
// VPS Player is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with VPS Player. If not, see <http://www.gnu.org/licenses/>.

#include <QtMath>
#include <QAudio>
#include <QApplication>
#include <QDir>
#include <QFileDialog>
#include <QFont>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QSlider>
#include <QStandardPaths>
#include <QStatusBar>
#include <QStringList>
#include <QVBoxLayout>

#include "Player_window.h"
#include "tools.h"


// Constructor
PlayerWindow::PlayerWindow(const QIcon &app_icon, const QString &filename)
{
  audio_player = new AudioPlayer(this);

  const QIcon open_icon(QStringLiteral(":/open-32.png"));
  const QIcon backward_icon(QStringLiteral(":/backward-32.png"));
  const QIcon forward_icon(QStringLiteral(":/forward-32.png"));
  const QFont fixed_font(QStringLiteral("monospace"));
  
  QMenuBar *menu_bar = menuBar();
  QMenu *menu_file = menu_bar->addMenu("&File");
  QMenu *menu_help = menu_bar->addMenu(QStringLiteral("&?"));
  action_open = menu_file->addAction(open_icon, "&Open", QKeySequence(QStringLiteral("Ctrl+O")), this, &PlayerWindow::openFileFromSelector);
  menu_file->addSeparator();
  menu_file->addAction(QIcon(QStringLiteral(":/quit-32.png")), "&Quit", QKeySequence(QStringLiteral("Ctrl+Q")), this, &PlayerWindow::close);
  menu_help->addAction(app_icon, "&About", this, &PlayerWindow::showAbout);
  menu_help->addAction(QIcon(QStringLiteral(":/qt-32.png")), "About Q&t", qApp, &QApplication::aboutQt);
  
  label_status = new QLabel;
  label_loading_progress = new QLabel;
  label_loading_progress->setFont(fixed_font);
  QStatusBar *status_bar = statusBar();
  status_bar->addWidget(label_status, 1);
  status_bar->addPermanentWidget(label_loading_progress, 0);
  
  QLabel *label_pitch = new QLabel("Pitch");
  label_pitch->setAlignment(Qt::AlignRight);
  QLabel *label_speed = new QLabel("Speed");
  label_speed->setAlignment(Qt::AlignRight);
  QLabel *label_volume = new QLabel("Volume");
  label_volume->setAlignment(Qt::AlignRight);
  QSlider *slider_pitch = new QSlider;
  slider_pitch->setOrientation(Qt::Horizontal);
  slider_pitch->setTickPosition(QSlider::TicksAbove);
  slider_pitch->setRange(-12, 12);
  slider_pitch->setSingleStep(1);
  slider_pitch->setPageStep(2);
  slider_pitch->setTickInterval(12);
  QSlider *slider_speed = new QSlider;
  slider_speed->setOrientation(Qt::Horizontal);
  slider_speed->setTickPosition(QSlider::TicksAbove);
  slider_speed->setRange(-12, 12);
  slider_speed->setSingleStep(1);
  slider_speed->setPageStep(4);
  slider_speed->setTickInterval(12);
  QSlider *slider_volume = new QSlider;
  slider_volume->setOrientation(Qt::Horizontal);
  slider_volume->setTickPosition(QSlider::TicksAbove);
  slider_volume->setRange(0, 100);
  slider_volume->setSingleStep(1);
  slider_volume->setPageStep(10);
  slider_volume->setTickInterval(100);
  spinbox_pitch = new QSpinBox;
  spinbox_pitch->setRange(-12, 12);
  spinbox_pitch->setToolTip("semitones");
  label_speed_value = new QLabel;
  label_speed_value->setAlignment(Qt::AlignCenter);
  lcd_volume = new QLCDNumber(3);
  QGridLayout *layout_sliders = new QGridLayout;
  layout_sliders->addWidget(label_pitch, 0, 0);
  layout_sliders->addWidget(label_speed, 1, 0);
  layout_sliders->addWidget(label_volume, 2, 0);
  layout_sliders->addWidget(slider_pitch, 0, 1);
  layout_sliders->addWidget(slider_speed, 1, 1);
  layout_sliders->addWidget(slider_volume, 2, 1);
  layout_sliders->addWidget(spinbox_pitch, 0, 2);
  layout_sliders->addWidget(label_speed_value, 1, 2);
  layout_sliders->addWidget(lcd_volume, 2, 2);

  QLabel *label_engine = new QLabel("Engine");
  combobox_engine = new QComboBox;
  combobox_engine->addItem("Rubber Band R2 (faster)");
  combobox_engine->addItem("Rubber Band R3 (finer)");
  combobox_engine->setToolTip("R3 engine produces higher-quality results than the R2 engine for most material, especially complex mixes, vocals and other sounds that have soft onsets and smooth pitch changes, and music with substantial bass content. However, it uses much more CPU power than the R2 engine.");
  QHBoxLayout *layout_engine = new QHBoxLayout;
  layout_engine->addWidget(label_engine);
  layout_engine->addWidget(combobox_engine);
  layout_engine->addStretch();
  
  check_high_quality = new QCheckBox("High quality (uses more CPU)");
  check_high_quality->setToolTip("Use the highest quality method for pitch shifting. This method may use much more CPU, especially for large pitch shift.");
  QCheckBox *check_formant_preserved = new QCheckBox("Preserve formant shape (spectral envelope)");
  check_formant_preserved->setToolTip("Preserve the spectral envelope of the original signal. This permits shifting the note frequency without so substantially affecting the perceived pitch profile of the voice or instrument.");
  QVBoxLayout *layout_settings = new QVBoxLayout;
  layout_settings->addLayout(layout_sliders);
  layout_settings->addLayout(layout_engine);
  layout_settings->addWidget(check_high_quality);
  layout_settings->addWidget(check_formant_preserved);
  QGroupBox *groupbox_settings = new QGroupBox("Settings");
  groupbox_settings->setLayout(layout_settings);
  
  button_open = new QPushButton(open_icon, "Open file");
  button_open->setToolTip(QStringLiteral("Ctrl+O"));
  button_cancel = new QPushButton(QIcon(QStringLiteral(":/cancel-32.png")), "Cancel");
  button_play = new QPushButton(QIcon(QStringLiteral(":/play-32.png")), "Play");
  button_pause = new QPushButton(QIcon(QStringLiteral(":/pause-32.png")), "Pause");
  button_stop = new QPushButton(QIcon(QStringLiteral(":/stop-32.png")), "Stop");
  QHBoxLayout *layout_buttons = new QHBoxLayout;
  layout_buttons->addWidget(button_open);
  layout_buttons->addWidget(button_cancel);
  layout_buttons->addWidget(button_play);
  layout_buttons->addWidget(button_pause);
  layout_buttons->addWidget(button_stop);

  button_bwd10 = new QPushButton(backward_icon, QStringLiteral("-10s"));
  button_bwd5 = new QPushButton(backward_icon, QStringLiteral("-5s"));
  button_fwd5 = new QPushButton(forward_icon, QStringLiteral("+5s"));
  button_fwd10 = new QPushButton(forward_icon, QStringLiteral("+10s"));
  QHBoxLayout *layout_buttons2 = new QHBoxLayout;
  layout_buttons2->addWidget(button_bwd10);
  layout_buttons2->addWidget(button_bwd5);
  layout_buttons2->addStretch();
  layout_buttons2->addWidget(button_fwd5);
  layout_buttons2->addWidget(button_fwd10);

  progress_playing = new PlayingProgress;
  label_reading_progress = new QLabel;
  label_reading_progress->setFont(fixed_font);
  label_duration = new QLabel;
  label_duration->setFont(fixed_font);
  QHBoxLayout *layout_progress = new QHBoxLayout;
  layout_progress->addWidget(label_reading_progress);
  layout_progress->addWidget(progress_playing);
  layout_progress->addWidget(label_duration);

  QVBoxLayout *layout_player = new QVBoxLayout;
  layout_player->addLayout(layout_buttons);
  layout_player->addLayout(layout_buttons2);
  layout_player->addLayout(layout_progress);
  QGroupBox *groupbox_player = new QGroupBox("Player");
  groupbox_player->setLayout(layout_player);
  
  QVBoxLayout *layout_main = new QVBoxLayout;
  layout_main->addWidget(groupbox_settings);
  layout_main->addWidget(groupbox_player);
  QWidget *widget_main = new QWidget;
  widget_main->setLayout(layout_main);
  setCentralWidget(widget_main);

  slider_pitch->setValue(0);
  updatePitch(0);
  slider_speed->setValue(0);
  updateSpeed(0);
  slider_volume->setValue(100);
  updateVolume(100);
  combobox_engine->setCurrentIndex(1);
  audio_player->updateOptionUseR3Engine(true);
  check_high_quality->setChecked(true);
  audio_player->updateOptionHighQuality(true);
  check_formant_preserved->setChecked(true);
  audio_player->updateOptionFormantPreserved(true);
  updateStatus(audio_player->getStatus());
  updateReadingPosition(-1);
  updateDuration(-1);
  
  adjustSize();
  setMaximumHeight(height());

  connect(button_open, &QPushButton::clicked, this, &PlayerWindow::openFileFromSelector);
  connect(button_cancel, &QPushButton::clicked, audio_player, &AudioPlayer::cancelDecoding);
  connect(button_play, &QPushButton::clicked, this, &PlayerWindow::playAudio);
  connect(button_pause, &QPushButton::clicked, audio_player, &AudioPlayer::pausePlaying);
  connect(button_stop, &QPushButton::clicked, audio_player, &AudioPlayer::stopPlaying);
  connect(button_bwd10, &QPushButton::clicked, [this](){ moveReadingPosition(-10000); });
  connect(button_bwd5, &QPushButton::clicked, [this](){ moveReadingPosition(-5000); });
  connect(button_fwd5, &QPushButton::clicked, [this](){ moveReadingPosition(5000); });
  connect(button_fwd10, &QPushButton::clicked, [this](){ moveReadingPosition(10000); });
  connect(spinbox_pitch, qOverload<int>(&QSpinBox::valueChanged), slider_pitch, &QAbstractSlider::setValue);
  connect(slider_pitch, &QAbstractSlider::valueChanged, this, &PlayerWindow::updatePitch);
  connect(slider_speed, &QAbstractSlider::valueChanged, this, &PlayerWindow::updateSpeed);
  connect(slider_volume, &QAbstractSlider::valueChanged, this, &PlayerWindow::updateVolume);
  connect(combobox_engine, &QComboBox::currentIndexChanged, [this](int index){ audio_player->updateOptionUseR3Engine(index == 1); });
  connect(check_high_quality, &QAbstractButton::toggled, audio_player, &AudioPlayer::updateOptionHighQuality);
  connect(check_formant_preserved, &QAbstractButton::toggled, audio_player, &AudioPlayer::updateOptionFormantPreserved);
  connect(audio_player, &AudioPlayer::statusChanged, this, &PlayerWindow::updateStatus);
  connect(audio_player, &AudioPlayer::loadingProgressChanged, [this](int progress){ label_loading_progress->setText(QStringLiteral("%1 \%").arg(progress)); });
  connect(audio_player, &AudioPlayer::durationChanged, this, &PlayerWindow::updateDuration);
  connect(audio_player, &AudioPlayer::readingPositionChanged, this, &PlayerWindow::updateReadingPosition);
  connect(audio_player, &AudioPlayer::audioDecodingError, this, &PlayerWindow::displayAudioDecodingError);
  connect(audio_player, &AudioPlayer::audioOutputError, this, &PlayerWindow::displayAudioDeviceError);
  connect(progress_playing, &PlayingProgress::barClicked, audio_player, &AudioPlayer::moveReadingPosition);

  const QStringList music_directories = QStandardPaths::standardLocations(QStandardPaths::MusicLocation);
  if (music_directories.isEmpty())
    music_directory = QDir::homePath();
  else
    music_directory = music_directories.first();

  if (!filename.isEmpty()) {
    const QFileInfo file_info(filename);
    
    if (file_info.exists() && file_info.isFile())
      openFile(file_info);
    else
      QMessageBox::critical(this, "File not found", QString("\"%1\" is not a valid file.").arg(file_info.filePath()), QMessageBox::Ok);
  }
}


// Destructor
PlayerWindow::~PlayerWindow()
{
  
}


// Prompt an error popup for an audio decoding error
void PlayerWindow::displayAudioDecodingError(QAudioDecoder::Error error)
{
  QString error_message;

  switch(error) {
  case QAudioDecoder::ResourceError :
    error_message = "File not found";
    break;
  case QAudioDecoder::FormatError :
    error_message = "Format not supported";
    break;
  case QAudioDecoder::AccessDeniedError :
    error_message = "Access denied";
    break;
  case QAudioDecoder::NotSupportedError :
    error_message = "Audio decoding not supported on this platform";
    break;
  default :
    error_message = "Unknown error";
  }

  QMessageBox::critical(this, "Audio decoding error", "Error while decoding audio file:\n" + error_message, QMessageBox::Ok);
}


// Prompt an error popup for an audio device error
void PlayerWindow::displayAudioDeviceError(QAudio::Error error)
{
  QString error_message;

  switch(error) {
  case QAudio::OpenError :
    error_message = "Unable to open audio device";
    break;
  case QAudio::IOError :
    error_message = "Unable to send data to audio device";
    break;
  case QAudio::FatalError :
    error_message = "Audio device not usable";
    break;
  default :
    error_message = "Unknown error";
  }

  QMessageBox::critical(this, "Audio device error", "Error while accessing audio device:\n" + error_message, QMessageBox::Ok);
}


// Open file given in parameter
void PlayerWindow::openFile(const QFileInfo &file_info)
{
  setWindowTitle(QStringLiteral("VPS Player [%1]").arg(file_info.fileName()));
  music_directory = file_info.canonicalPath();

  audio_player->decodeFile(file_info.canonicalFilePath());
}


// Open a new file (chosen with a file selector)
void PlayerWindow::openFileFromSelector()
{
  QString audio_files_filter("Common audio files (*.aac *.flac *.m4a *.mp3 *.ogg *.wav *.wma)");
  const QString selected_file = QFileDialog::getOpenFileName(this, "Select audio file", music_directory, audio_files_filter + ";;All files (*)", &audio_files_filter);

  if (!selected_file.isEmpty())
    openFile(QFileInfo(selected_file));
}


// Start or resume audio playing
void PlayerWindow::playAudio()
{
  if (audio_player->getStatus() == AudioPlayer::Stopped)
    audio_player->startPlaying();
  else
    audio_player->resumePlaying();
}


// Moves reading position backward or forward. Parameter: position change in milliseconds
void PlayerWindow::moveReadingPosition(int delta)
{
  int new_position = progress_playing->value() + delta;
  if (new_position >= progress_playing->maximum())
    audio_player->stopPlaying();
  else
    audio_player->moveReadingPosition(qMax(0, new_position));
}


// Displays "About" dialog window
void PlayerWindow::showAbout()
{
  QMessageBox::about(this,
                     "About VPS Player",
                     QString("<h3>VPS Player</h3><p>High quality Variable Pitch and Speed audio player<br>Release <b>%1</b></p><p><a href=\"https://github.com/fcrollet/vpsplayer\">https://github.com/fcrollet/vpsplayer</a></p><p>Developed by François CROLLET</p><p>This program makes use of the Rubber Band Library<br><a href=\"https://www.breakfastquay.com/rubberband/\">https://www.breakfastquay.com/rubberband/</a></p>").arg(VERSION_STRING));
}


// Updates total file duration
void PlayerWindow::updateDuration(int duration)
{
  if (duration == -1)
    progress_playing->setValue(0);
  else
    progress_playing->setMaximum(duration);
  label_duration->setText(Tools::convertMSecToText(duration));
}


// Updates the pitch
void PlayerWindow::updatePitch(int pitch)
{
  spinbox_pitch->setValue(pitch);
  audio_player->updatePitch(pitch);
}


// Updates current reading position
void PlayerWindow::updateReadingPosition(int position)
{
  if (position == -1)
    progress_playing->setValue(0);
  else{
    progress_playing->setValue(position);
    progress_playing->setFormat(QString::number(position)); // to force progress bar refresh
  }
  label_reading_progress->setText(Tools::convertMSecToText(position));
}


// Updates the speed
void PlayerWindow::updateSpeed(int speed)
{
  qreal speed_ratio = qPow(qreal(2.0), speed / qreal(12.0));
  label_speed_value->setText(QStringLiteral("x %1").arg(speed_ratio, 0, 'f', 2));
  audio_player->updateSpeed(static_cast<double>(speed_ratio));
}


// Updates the window based on the player status
void PlayerWindow::updateStatus(AudioPlayer::Status status)
{
  auto set_controls = [this](const QString &status_text, bool enable_open, bool decoding, bool playback_begun, bool enable_play, bool enable_pause, bool enable_options) {
    label_status->setText(status_text);
    action_open->setEnabled(enable_open);
    button_open->setEnabled(enable_open);
    button_cancel->setEnabled(decoding);
    button_stop->setEnabled(playback_begun);
    button_bwd10->setEnabled(playback_begun);
    button_bwd5->setEnabled(playback_begun);
    button_fwd5->setEnabled(playback_begun);
    button_fwd10->setEnabled(playback_begun);
    button_play->setEnabled(enable_play);
    button_pause->setEnabled(enable_pause);
    combobox_engine->setEnabled(enable_options);
    check_high_quality->setEnabled(enable_options);
    progress_playing->setClickable(playback_begun);
  };

  switch(status) {
  case AudioPlayer::NoFileLoaded :
    set_controls("No file loaded", true, false, false, false, false, true);
    setWindowTitle(QStringLiteral("VPS Player"));
    label_loading_progress->clear();
    break;
  case AudioPlayer::Loading :
    set_controls("Loading file", false, true, false, false, false, false);
    break;
  case AudioPlayer::Stopped :
    set_controls("Stopped", true, false, false, true, false, true);
    break;
  case AudioPlayer::Paused :
    set_controls("Paused", true, false, true, true, false, false);
    break;
  case AudioPlayer::Playing :
    set_controls("Playing", true, false, true, false, true, false);
    break;
  }
}


// Updates the volume
void PlayerWindow::updateVolume(int volume)
{
  lcd_volume->display(volume);
  audio_player->updateVolume(QAudio::convertVolume(volume / qreal(100.0), QAudio::LogarithmicVolumeScale, QAudio::LinearVolumeScale));
}
