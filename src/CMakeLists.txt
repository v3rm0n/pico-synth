file(GLOB FILES *.cpp *.hpp)

add_executable(picosynth ${FILES})

target_compile_definitions(picosynth PRIVATE
        PICO_AUDIO_I2S_MONO_INPUT=1
        USE_AUDIO_I2S=1
        )

target_link_libraries(picosynth
        pico_stdlib
        hardware_adc
        hardware_i2c
        pico_audio_i2s
        encoder)

pico_add_extra_outputs(picosynth)
