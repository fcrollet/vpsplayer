// Copyright 2018-2019 François CROLLET

// This file is part of VPS Player.
// VPS Player is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.
// VPS Player is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with VPS Player. If not, see <http://www.gnu.org/licenses/>.

#ifndef AUDIO_PLAYER_H
#define AUDIO_PLAYER_H

#include <rubberband/RubberBandStretcher.h>
#include <memory>
#include <QAudioBuffer>
#include <QAudioDecoder>
#include <QAudioFormat>
#include <QAudioOutput>
#include <QBuffer>
#include <QIODevice>
#include <QObject>
#include <QString>
#include <QVector>


class AudioPlayer : public QObject
{
  Q_OBJECT

public:
  enum Status
    {
     NoFileLoaded = 0,
     Loading = 1,
     Stopped = 2,
     Paused = 3,
     Playing = 4
    };

private:
  AudioPlayer::Status status;
  qreal output_volume;
  double time_ratio;
  double pitch_scale;
  bool option_formant_preserved;
  bool option_high_quality;
  QAudioFormat target_format;
  QAudioDecoder *audio_decoder;
  std::unique_ptr<QVector<QAudioBuffer>> decoded_samples;
  int nb_audio_buffers;
  QAudioOutput *audio_output;
  QIODevice *output_buffer;
  QBuffer *temp_buffer;
  int reading_index;
  bool no_more_data;
  std::unique_ptr<RubberBand::RubberBandStretcher> stretcher;
  
public:
  AudioPlayer(QObject *parent = nullptr); // Constructor
  ~AudioPlayer(); // Destructor
  void cancelDecoding(); // Cancel current file decoding
  void decodeFile(const QString &filename); // Decode an audio file
  AudioPlayer::Status getStatus() const; // Get current status
  void moveReadingPosition(int position); // Move reading position. Parameter: position in milliseconds
  void pausePlaying(); // Pause audio playing
  void resumePlaying(); // Resume audio playing
  void startPlaying(); // Start audio playing
  void stopPlaying(); // Stop audio playing
  void updateOptionFormantPreserved(bool option); // Change "formant preserved" option
  void updateOptionHighQuality(bool option); // Change "high quality" option
  void updatePitch(int pitch); // Update pitch
  void updateSpeed(double speed_ratio); // Update speed
  void updateVolume(qreal volume); // Update output volume

private:
  void abortDecoding(QAudioDecoder::Error error); // Abort audio file decoding
  void fillAudioBuffer(); // Fill output audio buffer
  void finishDecoding(); // End audio file decoding
  RubberBand::RubberBandStretcher::Options generateStretcherOptionsFlag() const; // Returns options' flag that can be passed to the stretcher
  void manageAudioOutputState(QAudio::State state); // Handle changes of audio output's state
  void readDecoderBuffer(); // Read buffer from the decoder
  
signals:
  void audioDecodingError(QAudioDecoder::Error); // This signal is emitted if an error occurs while tring to decode audio file
  void audioOutputError(QAudio::Error); // This signal is emitted if an error occurs while trying to access audio device
  void durationChanged(int); // This signal is emitted each time the total duration of the file changes. Parameter: duration in milliseconds (-1 if no valid audio file loaded)
  void loadingProgressChanged(int); // This signal is emitted to indicate the current loading progress. Parameter: progress between 0 and 100
  void readingPositionChanged(int); // This signal is emitted each time the reading position changes. Parameter: position in milliseconds (-1 if no valid audio file loaded)
  void statusChanged(AudioPlayer::Status); // This signal is emitted each time the status changes.
};

#endif
