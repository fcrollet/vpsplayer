// Copyright 2018-2020 François CROLLET

// This file is part of VPS Player.
// VPS Player is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.
// VPS Player is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with VPS Player. If not, see <http://www.gnu.org/licenses/>.

#include <QtDebug>
#include <QtMath>
#include <QAudioDeviceInfo>
#include <QSysInfo>

#include "Audio_player.h"


// Constructor
AudioPlayer::AudioPlayer(QObject *parent) : QObject(parent),
					    status(AudioPlayer::NoFileLoaded),
					    output_volume(1.0),
					    time_ratio(1.0),
					    pitch_scale(1.0),
					    option_formant_preserved(true),
					    option_high_quality(true),
					    target_format(QAudioDeviceInfo::defaultOutputDevice().preferredFormat())
{
  target_format.setCodec(QStringLiteral("audio/pcm"));
  target_format.setSampleType(QAudioFormat::SignedInt);
  target_format.setSampleSize(16);
  if (QSysInfo::ByteOrder == QSysInfo::BigEndian)
    target_format.setByteOrder(QAudioFormat::BigEndian);
  else
    target_format.setByteOrder(QAudioFormat::LittleEndian);
  qDebug() << "Sample rate:" << target_format.sampleRate();
  qDebug() << "Channel count:" << target_format.channelCount();
}


// Destructor
AudioPlayer::~AudioPlayer()
{
  
}


// Cancel current file decoding
void AudioPlayer::cancelDecoding()
{
  if (status != AudioPlayer::Loading)
    return;

  abortDecoding(QAudioDecoder::NoError);
}


// Decode an audio file
void AudioPlayer::decodeFile(const QString &filename)
{
  if (status == AudioPlayer::Loading)
    return;
  
  if ((status == AudioPlayer::Paused) || (status == AudioPlayer::Playing))
    stopPlaying();
  
  status = AudioPlayer::Loading;
  emit statusChanged(status);
  emit readingPositionChanged(-1);
  emit loadingProgressChanged(0);

  decoded_samples = std::make_unique<QVector<QAudioBuffer>>();

  audio_decoder = new QAudioDecoder(this);
  QAudioFormat decode_format(target_format);
  decode_format.setSampleType(QAudioFormat::Float);
  decode_format.setSampleSize(32);
  audio_decoder->setSourceFilename(filename);
  audio_decoder->setAudioFormat(decode_format);

  connect(audio_decoder, &QAudioDecoder::bufferReady, this, &AudioPlayer::readDecoderBuffer);
  connect(audio_decoder, &QAudioDecoder::durationChanged, [=](qint64 duration){ emit durationChanged(static_cast<int>(duration)); });
  connect(audio_decoder, &QAudioDecoder::finished, this, &AudioPlayer::finishDecoding);
  connect(audio_decoder, qOverload<QAudioDecoder::Error>(&QAudioDecoder::error), this, &AudioPlayer::abortDecoding);
    
  audio_decoder->start();
}


// Get current status
AudioPlayer::Status AudioPlayer::getStatus() const
{
  return status;
}


// Move reading position. Parameter: position in milliseconds
void AudioPlayer::moveReadingPosition(int position)
{
  if ((status != AudioPlayer::Playing) && (status != AudioPlayer::Paused))
    return;
  
  reading_index = 0;
  while ((reading_index < nb_audio_buffers) && (static_cast<int>(decoded_samples->at(reading_index).startTime() / 1000) < position))
    reading_index++;

  emit readingPositionChanged(position);
  
  temp_buffer->close();
  temp_buffer->buffer().clear();
  stretcher->reset();
  audio_output->reset();
  output_buffer = audio_output->start();
  if (status == AudioPlayer::Paused)
    audio_output->suspend();
  fillAudioBuffer();
}


// Pause audio playing
void AudioPlayer::pausePlaying()
{
  if (status != AudioPlayer::Playing)
    return;

  status = AudioPlayer::Paused;
  emit statusChanged(status);
  audio_output->suspend();
}


// Resume audio playing
void AudioPlayer::resumePlaying()
{
  if (status != AudioPlayer::Paused)
    return;

  status = AudioPlayer::Playing;
  emit statusChanged(status);
  audio_output->resume();
  fillAudioBuffer();
}


// Start audio playing
void AudioPlayer::startPlaying()
{
  if (status != AudioPlayer::Stopped)
    return;

  status = AudioPlayer::Playing;
  emit statusChanged(status);

  reading_index = 0;
  no_more_data = false;

  stretcher = std::make_unique<RubberBand::RubberBandStretcher>(static_cast<size_t>(target_format.sampleRate()),
								static_cast<size_t>(target_format.channelCount()),
								generateStretcherOptionsFlag(),
								time_ratio,
								pitch_scale);
  int max_sample_count = 0;
  for (QAudioBuffer const& audio_buffer : *decoded_samples) {
    int sample_count = audio_buffer.sampleCount();
    if (sample_count > max_sample_count)
      max_sample_count = sample_count;
  }
  stretcher->setMaxProcessSize(static_cast<size_t>(max_sample_count));
  
  audio_output = new QAudioOutput(target_format, this);
  audio_output->setNotifyInterval(10);
  audio_output->setVolume(output_volume);
  temp_buffer = new QBuffer(this);
  connect(audio_output, &QAudioOutput::notify, this, &AudioPlayer::fillAudioBuffer);
  connect(audio_output, &QAudioOutput::stateChanged, this, &AudioPlayer::manageAudioOutputState);
  output_buffer = audio_output->start();
  QAudio::Error error_status = audio_output->error();
  if ((error_status == QAudio::NoError) || (error_status == QAudio::UnderrunError))
    fillAudioBuffer();
  else {
    qDebug() << "Error while opening audio device:" << error_status;
    emit audioOutputError(error_status);
    stopPlaying();
  }
}


// Stop audio playing
void AudioPlayer::stopPlaying()
{
  if ((status != AudioPlayer::Playing) && (status != AudioPlayer::Paused))
    return;

  status = AudioPlayer::Stopped;
  emit statusChanged(status);
  emit readingPositionChanged(0);
  
  audio_output->stop();
  disconnect(audio_output, 0, 0, 0);
  audio_output->deleteLater();
  temp_buffer->deleteLater();
  stretcher.reset();
}


// Change "formant preserved" option
void AudioPlayer::updateOptionFormantPreserved(bool option)
{
  option_formant_preserved = option;
  if (status == AudioPlayer::Paused || status == AudioPlayer::Playing)
    stretcher->setFormantOption(generateStretcherOptionsFlag());
}


// Change "high quality" option
void AudioPlayer::updateOptionHighQuality(bool option)
{
  option_high_quality = option;
  if (status == AudioPlayer::Paused || status == AudioPlayer::Playing)
    stretcher->setPitchOption(generateStretcherOptionsFlag());
}


// Update pitch
void AudioPlayer::updatePitch(int pitch)
{
  pitch_scale = static_cast<double>(qPow(qreal(2.0), pitch / qreal(12.0)));
  if (status == AudioPlayer::Paused || status == AudioPlayer::Playing)
    stretcher->setPitchScale(pitch_scale);
}


// Update speed
void AudioPlayer::updateSpeed(double speed_ratio)
{
  time_ratio = 1.0 / speed_ratio;
  if (status == AudioPlayer::Paused || status == AudioPlayer::Playing)
    stretcher->setTimeRatio(time_ratio);
}


// Update output volume
void AudioPlayer::updateVolume(qreal volume)
{
  output_volume = volume;
  if (status == AudioPlayer::Paused || status == AudioPlayer::Playing)
    audio_output->setVolume(output_volume);
}


// Abort audio file decoding
void AudioPlayer::abortDecoding(QAudioDecoder::Error error)
{
  disconnect(audio_decoder, 0, 0, 0);
  audio_decoder->deleteLater();
  decoded_samples.reset();
  status = AudioPlayer::NoFileLoaded;
  emit statusChanged(status);
  emit durationChanged(-1);

  if (error != QAudioDecoder::NoError) {
    qDebug() << "Error while decoding audio file:" << error;
    emit audioDecodingError(error);
  }
}


// Fill output audio buffer
void AudioPlayer::fillAudioBuffer()
{
  int bytes_needed = audio_output->bytesFree();
  int bytes_processed = 0;

  while (bytes_processed < bytes_needed) {
    while (temp_buffer->atEnd()){
      if (reading_index >= nb_audio_buffers) {
	no_more_data = true;
	return;
      }
      
      const QAudioBuffer &current_audio_buffer = decoded_samples->at(reading_index);
      reading_index++;
      const float *audio_buffer_data = current_audio_buffer.constData<float>();
      
      int nb_input_frames = current_audio_buffer.frameCount();
      int nb_channels = target_format.channelCount();
      float **stretcher_input = new float*[nb_channels];
      for (int i = 0; i < nb_channels; i++){
	stretcher_input[i] = new float[nb_input_frames];
	for (int j = 0; j < nb_input_frames; j++)
	  stretcher_input[i][j] = audio_buffer_data[(nb_channels * j) + i];
      }
      stretcher->process(stretcher_input, static_cast<size_t>(nb_input_frames), reading_index == nb_audio_buffers);
      for (int i = 0; i < nb_channels; i++)
	delete[] stretcher_input[i];
      delete[] stretcher_input;

      int nb_output_frames = stretcher->available();

      if (nb_output_frames > 0) {
	temp_buffer->close();
	
	float **stretcher_output = new float*[nb_channels];
	for (int i = 0; i < nb_channels; i++)
	  stretcher_output[i] = new float[nb_output_frames];
	nb_output_frames = static_cast<int>(stretcher->retrieve(stretcher_output, static_cast<size_t>(nb_output_frames)));

	int nb_output_samples = nb_output_frames * nb_channels;
	qint16 *output_samples = new qint16[nb_output_samples];
	for (int i = 0; i < nb_channels; i++){
	  for (int j = 0; j < nb_output_frames; j++){
	    float sample = stretcher_output[i][j];
	    int index = (nb_channels * j) + i;
	    if (sample > 1.0f)
	      output_samples[index] = qint16(32767);
	    else if (sample < -1.0f)
	      output_samples[index] = qint16(-32767);
	    else
	      output_samples[index] = static_cast<qint16>(qRound(sample * 32767.0f));
	  }
	  delete[] stretcher_output[i];
	}
	delete[] stretcher_output;

	temp_buffer->setData(reinterpret_cast<char*>(output_samples), sizeof(qint16) * nb_output_samples);
	delete[] output_samples;
      
	temp_buffer->open(QIODevice::ReadOnly);
	temp_buffer->seek(0);
      }

      emit readingPositionChanged(static_cast<int>(current_audio_buffer.startTime() / 1000));
    }

    qint64 size_to_write = qMin(temp_buffer->size() - temp_buffer->pos(), static_cast<qint64>(bytes_needed - bytes_processed));
    qint64 actually_written = output_buffer->write(temp_buffer->read(size_to_write));
    bytes_processed += static_cast<int>(actually_written);
    if (actually_written < size_to_write) { // Should not happen, but does happen on Windows 10!
      temp_buffer->seek(temp_buffer->pos() - size_to_write + actually_written);
      return;
    }
  }
}


// End audio file decoding
void AudioPlayer::finishDecoding()
{
  decoded_samples->squeeze();
  nb_audio_buffers = decoded_samples->size();

  emit loadingProgressChanged(100);
  disconnect(audio_decoder, 0, 0, 0);
  audio_decoder->deleteLater();
  status = AudioPlayer::Stopped;
  emit readingPositionChanged(0);
  startPlaying();
}


// Returns options' flag that can be passed to the stretcher
RubberBand::RubberBandStretcher::Options AudioPlayer::generateStretcherOptionsFlag() const
{
  RubberBand::RubberBandStretcher::Options options = RubberBand::RubberBandStretcher::DefaultOptions | RubberBand::RubberBandStretcher::OptionProcessRealTime | RubberBand::RubberBandStretcher::OptionThreadingNever;
  if (option_formant_preserved)
    options |= RubberBand::RubberBandStretcher::OptionFormantPreserved;
  if (option_high_quality)
    options |= RubberBand::RubberBandStretcher::OptionPitchHighQuality;
  return options;
}


// Handle changes of audio output's state
void AudioPlayer::manageAudioOutputState(QAudio::State state)
{
  if ((state == QAudio::IdleState) && no_more_data)
    stopPlaying();
}


// Read buffer from the decoder
void AudioPlayer::readDecoderBuffer()
{
  decoded_samples->append(audio_decoder->read());
  emit loadingProgressChanged(static_cast<int>((100 * audio_decoder->position()) / audio_decoder->duration()));
}
