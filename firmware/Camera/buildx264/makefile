# Load the config.mk file and get the variables in it.
include ../config.mk
include ../server.mk

# Configuration and installation path
PREFIX := ./x264install
# x264 Project source code directory
X264_SRC_DIR := ./x264

# Number of threads compiled in parallel (adjusted according to the number of cores of the machine)
JOBS := $(shell nproc)

# The lib folder path of the parent directory.
LIB_DIR := ../lib

# Default targets: configuration, build and installation
all: configure build install post-install

# Configuration Target: Run the. /configure script.
configure:
	@echo "Configuring x264..."
	@cd $(X264_SRC_DIR) && \
		./configure --prefix=$(PREFIX) --disable-shared --enable-static --disable-asm CC=$(CC)
	@echo "Configuration succeeded."

# Build goal: compile static library
build: configure
	@echo "Building x264 static library..."
	@cd $(X264_SRC_DIR) && $(MAKE) -j$(JOBS)
	@echo "The static library was built successfully."

# Installation Target: Install to the specified directory
install: build
	@echo "Installing x264 to $(PREFIX)..."
	@cd $(X264_SRC_DIR) && $(MAKE) install
	@echo "Installation succeeded."
	
# Post-processing target: copy the static library to the lib folder of the upper directory.
post-install: install
	@echo "Copy static library files to $(LIB_DIR)..."
	@mkdir -p $(LIB_DIR)  # Ensure that the target lib directory exists.
	@cp $(X264_SRC_DIR)/libx264.a $(LIB_DIR)  # Copy static library files to lib folder
	@echo "The static library file was copied successfully."
	
# Clean target: delete intermediate files and build results.
clean:
	@echo "Cleaning up..."
	@cd $(X264_SRC_DIR) && $(MAKE) clean
	@echo "Cleaning succeeded."

