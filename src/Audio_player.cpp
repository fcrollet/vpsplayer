// Copyright 2018-2023 François CROLLET

// This file is part of VPS Player.
// VPS Player is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.
// VPS Player is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with VPS Player. If not, see <http://www.gnu.org/licenses/>.

#include <QtDebug>
#include <QtMath>
#include <QMediaDevices>
#include <QUrl>

#include "Audio_player.h"


// Constructor
AudioPlayer::AudioPlayer(QObject *parent) : QObject(parent),
					    status(AudioPlayer::NoFileLoaded),
					    output_volume(1.0),
					    time_ratio(1.0),
					    pitch_scale(1.0),
					    option_use_r3_engine(true),
					    option_formant_preserved(true),
					    option_high_quality(true),
					    audio_device(QMediaDevices::defaultAudioOutput())
{
  min_channel_count = audio_device.minimumChannelCount();
  max_channel_count = audio_device.maximumChannelCount();
  min_sample_rate = audio_device.minimumSampleRate();
  max_sample_rate = audio_device.maximumSampleRate();
  qDebug() << "Minimum channel_count:" << min_channel_count;
  qDebug() << "Maximum channel count:" << max_channel_count;
  qDebug() << "Minimum sample rate:" << min_sample_rate;
  qDebug() << "Maximum sample rate:" << max_sample_rate;
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

  decoded_samples = std::make_unique<QList<QAudioBuffer>>();

  audio_decoder = std::make_unique<QAudioDecoder>();
  audio_decoder->setSource(QUrl::fromLocalFile(filename));

  connect(audio_decoder.get(), &QAudioDecoder::bufferReady, this, &AudioPlayer::firstDecodedBufferReady);
  connect(audio_decoder.get(), qOverload<QAudioDecoder::Error>(&QAudioDecoder::error), this, &AudioPlayer::abortDecoding);
  
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
  timer->stop();
}


// Resume audio playing
void AudioPlayer::resumePlaying()
{
  if (status != AudioPlayer::Paused)
    return;

  status = AudioPlayer::Playing;
  emit statusChanged(status);
  timer->start();
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
  
  audio_output = new QAudioSink(audio_device, target_format, this);
  audio_output->setBufferSize(static_cast<qsizetype>(target_format.bytesForDuration(100000))); // Audio buffer size should correspond to about 100 ms.
  audio_output->setVolume(output_volume);
  timer = new QTimer(this);
  timer->setInterval(10);
  temp_buffer = new QBuffer(this);
  connect(timer, &QTimer::timeout, this, &AudioPlayer::fillAudioBuffer);
  connect(audio_output, &QAudioSink::stateChanged, this, &AudioPlayer::manageAudioOutputState);
  output_buffer = audio_output->start();
  QAudio::Error error_status = audio_output->error();
  if ((error_status == QAudio::NoError) || (error_status == QAudio::UnderrunError))
    fillAudioBuffer();
  else {
    qDebug() << "Error while opening audio device:" << error_status;
    emit audioOutputError(error_status);
    stopPlaying();
  }
  timer->start();
}


// Stop audio playing
void AudioPlayer::stopPlaying()
{
  if ((status != AudioPlayer::Playing) && (status != AudioPlayer::Paused))
    return;

  status = AudioPlayer::Stopped;
  emit statusChanged(status);
  emit readingPositionChanged(0);

  disconnect(timer, nullptr, nullptr, nullptr);
  timer->deleteLater();
  audio_output->stop();
  disconnect(audio_output, nullptr, nullptr, nullptr);
  audio_output->deleteLater();
  temp_buffer->deleteLater();
  stretcher.reset();
}


// Sets pitch shifting engine
void AudioPlayer::updateOptionUseR3Engine(bool option)
{
  option_use_r3_engine = option;
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
  disconnect(audio_decoder.get(), nullptr, nullptr, nullptr);
  audio_decoder.reset();
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
      
      unsigned int nb_input_frames = static_cast<unsigned int>(current_audio_buffer.frameCount());
      unsigned int nb_channels = static_cast<unsigned int>(target_format.channelCount());
      float **stretcher_input = new float*[nb_channels];
      for (unsigned int i = 0; i < nb_channels; i++){
	stretcher_input[i] = new float[nb_input_frames];
	for (unsigned int j = 0; j < nb_input_frames; j++)
	  stretcher_input[i][j] = audio_buffer_data[(nb_channels * j) + i];
      }
      stretcher->process(stretcher_input, static_cast<size_t>(nb_input_frames), reading_index == nb_audio_buffers);
      for (unsigned int i = 0; i < nb_channels; i++)
	delete[] stretcher_input[i];
      delete[] stretcher_input;

      unsigned int nb_output_frames = static_cast<unsigned int>(stretcher->available());

      if (nb_output_frames > 0) {
	temp_buffer->close();
	
	float **stretcher_output = new float*[nb_channels];
	for (unsigned int i = 0; i < nb_channels; i++)
	  stretcher_output[i] = new float[nb_output_frames];
	nb_output_frames = static_cast<unsigned int>(stretcher->retrieve(stretcher_output, static_cast<size_t>(nb_output_frames)));

	unsigned int nb_output_samples = nb_output_frames * nb_channels;
	qint16 *output_samples = new qint16[nb_output_samples];
	for (unsigned int i = 0; i < nb_channels; i++){
	  for (unsigned int j = 0; j < nb_output_frames; j++){
	    float sample = stretcher_output[i][j];
	    unsigned int index = (nb_channels * j) + i;
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

	temp_buffer->setData(reinterpret_cast<char*>(output_samples), static_cast<int>(sizeof(qint16) * nb_output_samples));
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
  disconnect(audio_decoder.get(), nullptr, nullptr, nullptr);
  audio_decoder.reset();
  status = AudioPlayer::Stopped;
  emit readingPositionChanged(0);
  startPlaying();
}


// Reads the first decoded buffer and sets audio format accordingly for further decoding
void AudioPlayer::firstDecodedBufferReady()
{
  const QAudioFormat file_format = audio_decoder->read().format();
  qDebug() << "File format:" << file_format;
  const QUrl file_url = audio_decoder->source();
  audio_decoder->stop();
  disconnect(audio_decoder.get(), nullptr, nullptr, nullptr);

  target_format.setChannelCount(qBound(min_channel_count, file_format.channelCount(), max_channel_count));
  target_format.setSampleRate(qBound(min_sample_rate, file_format.sampleRate(), max_sample_rate));
  target_format.setSampleFormat(QAudioFormat::Int16);
  if (!audio_device.isFormatSupported(target_format)) {
    target_format = audio_device.preferredFormat();
    target_format.setSampleFormat(QAudioFormat::Int16);
    qDebug() << "Format not supported, falling back on default format";
  }
  qDebug() << "Output format:" << target_format;

  audio_decoder = std::make_unique<QAudioDecoder>();
  audio_decoder->setSource(file_url);

  QAudioFormat decode_format(target_format);
  decode_format.setSampleFormat(QAudioFormat::Float);
  audio_decoder->setAudioFormat(decode_format);
  
  connect(audio_decoder.get(), &QAudioDecoder::bufferReady, this, &AudioPlayer::readDecoderBuffer);
  connect(audio_decoder.get(), &QAudioDecoder::durationChanged, [this](qint64 duration){ if (duration > 0) emit durationChanged(static_cast<int>(duration)); });
  connect(audio_decoder.get(), &QAudioDecoder::finished, this, &AudioPlayer::finishDecoding);
  connect(audio_decoder.get(), qOverload<QAudioDecoder::Error>(&QAudioDecoder::error), this, &AudioPlayer::abortDecoding);

  audio_decoder->start();
}


// Returns options' flag that can be passed to the stretcher
RubberBand::RubberBandStretcher::Options AudioPlayer::generateStretcherOptionsFlag() const
{
  RubberBand::RubberBandStretcher::Options options = RubberBand::RubberBandStretcher::DefaultOptions | RubberBand::RubberBandStretcher::OptionProcessRealTime | RubberBand::RubberBandStretcher::OptionThreadingNever;
  if (option_use_r3_engine)
    options |= RubberBand::RubberBandStretcher::OptionEngineFiner;
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
