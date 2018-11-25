cmake_minimum_required(VERSION 3.11)
project(mimiclib_unit_test C)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -lm -I./include -I./source/SPTK/include -I./source/MultiPulse -Wall -std=c11 -DWIN_TEST")
add_executable(mp_encode.exe
  ./source/main.c
  ./source/LPC/LPAnalyzer.c
  ./source/wav/wav.c
  ./source/SPTK/src/lpc2lsp/_lpc2lsp.c
  ./source/SPTK/src/lsp2lpc/_lsp2lpc.c
  ./source/SPTK/src/lpc/_lpc.c
  ./source/SPTK/src/lpc2c/_lpc2c.c
  ./source/SPTK/src/acorr/_acorr.c
  ./source/SPTK/src/levdur/_levdur.c
  ./source/SPTK/src/window/_window.c
  ./source/SPTK/lib/getmem.c
  ./source/SPTK/lib/movem.c
  ./source/SPTK/lib/fillz.c
  ./source/MultiPulse/PulseSearch.c
)
