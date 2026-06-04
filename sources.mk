# List your include directories here.
INCLUDES = -I./src

# This is a makefile for our source files

SRC_DIR = Files_c
INCLUDE_DIR = Files_h 
OBJ_DIR = Files_o

#List of my source files here
SRCS = \
	$(wildcard $(SRC_DIR)/*.c) 
	

# List all your include directories here
INCLUDES = \
	-I$(INCLUDE_DIR) \
	-I./CMSIS/STM32F4xx/Include \
	-I./CMSIS/Core/Include \
	-I./STM_files
