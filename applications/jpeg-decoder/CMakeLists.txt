include_directories(include)

set(JPEG_DECODER_SRCS src/fourier.cpp src/commands.cpp src/decoder.cpp)

add_library(jpeg-decoder ${JPEG_DECODER_SRCS})

target_link_libraries(jpeg-decoder byte-streams ${FFTW_LIBRARIES})
target_include_directories(jpeg-decoder SYSTEM PUBLIC ${FFTW_INCLUDES})

add_subdirectory(tests)
