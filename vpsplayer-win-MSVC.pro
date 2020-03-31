TEMPLATE = app
CONFIG += qt warn_off release c++14
QT += widgets multimedia
INCLUDEPATH += rubberband rubberband/src
DEFINES += VERSION_STRING=\\\"1.0.3\\\"
DEFINES += USE_KISSFFT USE_SPEEX NO_THREADING __MSVC__
HEADERS = src/Audio_player.h \
          src/Player_window.h \
          src/Playing_progress.h \
          src/tools.h \
          rubberband/src/StretcherChannelData.h \
          rubberband/src/float_cast/float_cast.h \
          rubberband/src/StretcherImpl.h \
          rubberband/src/StretchCalculator.h \
          rubberband/src/base/Profiler.h \
          rubberband/src/base/RingBuffer.h \
          rubberband/src/base/Scavenger.h \
          rubberband/src/dsp/AudioCurveCalculator.h \
          rubberband/src/audiocurves/CompoundAudioCurve.h \
          rubberband/src/audiocurves/ConstantAudioCurve.h \
          rubberband/src/audiocurves/HighFrequencyAudioCurve.h \
          rubberband/src/audiocurves/PercussiveAudioCurve.h \
          rubberband/src/audiocurves/SilentAudioCurve.h \
          rubberband/src/audiocurves/SpectralDifferenceAudioCurve.h \
          rubberband/src/dsp/Resampler.h \
          rubberband/src/dsp/FFT.h \
          rubberband/src/dsp/MovingMedian.h \
          rubberband/src/dsp/SincWindow.h \
          rubberband/src/dsp/Window.h \
          rubberband/src/system/Allocators.h \
          rubberband/src/system/Thread.h \
          rubberband/src/system/VectorOps.h \
          rubberband/src/system/sysutils.h \
          rubberband/src/kissfft/_kiss_fft_guts.h \
          rubberband/src/kissfft/kiss_fft.h \
          rubberband/src/kissfft/kiss_fftr.h \
          rubberband/src/speex/speex_resampler.h
SOURCES = src/main.cpp \
          src/Audio_player.cpp \
          src/Player_window.cpp \
          src/Playing_progress.cpp \
          src/tools.cpp \
          rubberband/src/RubberBandStretcher.cpp \
          rubberband/src/StretcherProcess.cpp \
          rubberband/src/StretchCalculator.cpp \
          rubberband/src/base/Profiler.cpp \
          rubberband/src/dsp/AudioCurveCalculator.cpp \
          rubberband/src/audiocurves/CompoundAudioCurve.cpp \
          rubberband/src/audiocurves/SpectralDifferenceAudioCurve.cpp \
          rubberband/src/audiocurves/HighFrequencyAudioCurve.cpp \
          rubberband/src/audiocurves/SilentAudioCurve.cpp \
          rubberband/src/audiocurves/ConstantAudioCurve.cpp \
          rubberband/src/audiocurves/PercussiveAudioCurve.cpp \
          rubberband/src/dsp/Resampler.cpp \
          rubberband/src/dsp/FFT.cpp \
          rubberband/src/system/Allocators.cpp \
          rubberband/src/system/sysutils.cpp \
          rubberband/src/system/Thread.cpp \
          rubberband/src/StretcherChannelData.cpp \
          rubberband/src/StretcherImpl.cpp \
          rubberband/src/kissfft/kiss_fft.c \
          rubberband/src/kissfft/kiss_fftr.c \
          rubberband/src/speex/resample.c
RESOURCES = icons.qrc
RC_FILE = vpsplayer-win.rc
TARGET = vpsplayer
