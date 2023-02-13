set(AUBIO_SDK_DIRECTORY "${CMAKE_SOURCE_DIR}/dependencies/aubio")
set(AUBIO_SDK_SRC_DIRECTORY "${AUBIO_SDK_DIRECTORY}/src")

file(GLOB AUBIO_IO_SOURCES ${AUBIO_SDK_SRC_DIRECTORY}/io/*.h ${AUBIO_SDK_SRC_DIRECTORY}/io/*.c)
file(GLOB AUBIO_NOTE_SOURCES ${AUBIO_SDK_SRC_DIRECTORY}/notes/*.h ${AUBIO_SDK_SRC_DIRECTORY}/notes/*.c)
file(GLOB AUBIO_ONSET_SOURCES ${AUBIO_SDK_SRC_DIRECTORY}/onset/*.h ${AUBIO_SDK_SRC_DIRECTORY}/onset/*.c)
file(GLOB AUBIO_PITCH_SOURCES ${AUBIO_SDK_SRC_DIRECTORY}/pitch/*.h ${AUBIO_SDK_SRC_DIRECTORY}/pitch/*.c)
file(GLOB AUBIO_SPECTRAL_SOURCES ${AUBIO_SDK_SRC_DIRECTORY}/spectral/*.h ${AUBIO_SDK_SRC_DIRECTORY}/spectral/*.c)
file(GLOB AUBIO_SYNTH_SOURCES ${AUBIO_SDK_SRC_DIRECTORY}/synth/*.h ${AUBIO_SDK_SRC_DIRECTORY}/synth/*.c)
file(GLOB AUBIO_TEMPO_SOURCES ${AUBIO_SDK_SRC_DIRECTORY}/tempo/*.h ${AUBIO_SDK_SRC_DIRECTORY}/tempo/*.c)
file(GLOB AUBIO_TEMPORAL_SOURCES ${AUBIO_SDK_SRC_DIRECTORY}/temporal/*.h ${AUBIO_SDK_SRC_DIRECTORY}/temporal/*.c)
file(GLOB AUBIO_UTILS_SOURCES ${AUBIO_SDK_SRC_DIRECTORY}/utils/*.h ${AUBIO_SDK_SRC_DIRECTORY}/utils/*.c)
file(GLOB AUBIO_SOURCES ${AUBIO_SDK_SRC_DIRECTORY}/*.h ${AUBIO_SDK_SRC_DIRECTORY}/*.c)

include_directories(
    ${AUBIO_SDK_SRC_DIRECTORY} 
    ${AUBIO_SDK_SRC_DIRECTORY}/io 
    ${AUBIO_SDK_SRC_DIRECTORY}/notes
    ${AUBIO_SDK_SRC_DIRECTORY}/onset
    ${AUBIO_SDK_SRC_DIRECTORY}/pitch
    ${AUBIO_SDK_SRC_DIRECTORY}/spectral
    ${AUBIO_SDK_SRC_DIRECTORY}/synth
    ${AUBIO_SDK_SRC_DIRECTORY}/tempo
    ${AUBIO_SDK_SRC_DIRECTORY}/temporal
    ${AUBIO_SDK_SRC_DIRECTORY}/utils
) 

source_group("sources" FILES ${AUBIO_SOURCES})
source_group("sources/io" FILES ${AUBIO_IO_SOURCES})
source_group("sources/note" FILES ${AUBIO_NOTE_SOURCES})
source_group("sources/onset" FILES ${AUBIO_ONSET_SOURCES})
source_group("sources/pitch" FILES ${AUBIO_PITCH_SOURCES})
source_group("sources/spectral" FILES ${AUBIO_SPECTRAL_SOURCES})
source_group("sources/synth" FILES ${AUBIO_SYNTH_SOURCES})
source_group("sources/tempo" FILES ${AUBIO_TEMPO_SOURCES})
source_group("sources/temporal" FILES ${AUBIO_TEMPORAL_SOURCES})
source_group("sources/utils" FILES ${AUBIO_UTILS_SOURCES})

add_library(aubio 
            STATIC 
            ${AUBIO_SOURCES}
            ${AUBIO_IO_SOURCES} 
            ${AUBIO_NOTE_SOURCES} 
            ${AUBIO_ONSET_SOURCES} 
            ${AUBIO_PITCH_SOURCES} 
            ${AUBIO_SPECTRAL_SOURCES} 
            ${AUBIO_SYNTH_SOURCES}
            ${AUBIO_TEMPO_SOURCES}
            ${AUBIO_TEMPORAL_SOURCES}
            ${AUBIO_UTILS_SOURCES}
)

target_compile_definitions(aubio 
                            PRIVATE 
                            HAVE_STDLIB_H=1 
                            HAVE_COMPLEX_H=1 
                            HAVE_MATH_H=1 
                            HAVE_STRING_H=1 
                            HAVE_ERRNO_H=1
                            HAVE_LIMITS_H=1
                            HAVE_STDARG_H=1
                            HAVE_ACCELERATE=1)
