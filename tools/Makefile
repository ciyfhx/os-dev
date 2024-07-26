CPP_SOURCES = $(wildcard ./*.cpp)
HEADERS = $(wildcard ./*.hpp)
# file extension replacement from cpp to o
OBJS = ${CPP_SOURCES:.cpp=.o}
# -g: debug flag -m32: 32bit object file
CPPFLAGS = -Wall -g -std=c++17

BIN_DIR=bin

all: ${BIN_DIR}/gpt ${BIN_DIR}/fat32

${BIN_DIR}/gpt: gpt.cpp
	${CXX} ${CPPFLAGS} $< -o $@

${BIN_DIR}/fat32: fat32.cpp
	${CXX} ${CPPFLAGS} $< -o $@

${BIN_DIR}/%.o: ${OBJS}
	${CXX} ${CPPFLAGS} -c $< -o $@

clean:
	rm -rf ${BIN_DIR}/*

.PHONY: clean all