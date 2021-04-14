#==============================================================================
# Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.
#
# Redistribution of this file, in original or modified form, without
# prior written consent of Allied Vision Technologies is prohibited.
#
#------------------------------------------------------------------------------
#
# File:         Makefile
#
# Description:  makefile for compiling V4L2 Viewer
#
#------------------------------------------------------------------------------
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF TITLE,
# NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR  PURPOSE ARE
# DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
# AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
# TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#==============================================================================
#
# general information
PROJECT_NAME            = V4L2Viewer

BIN_FILE = $(PROJECT_NAME)

# path variables
PROJECT_DIR             = ../..
MAKE_INCLUDE_DIR        = $(CURDIR)
CONFIG_DIR              = $(ARCH)_$(WORDSIZE)bit/$(CONFIG)
BIN_DIR                 = binary/$(CONFIG_DIR)
OBJ_DIR                 = object/$(CONFIG_DIR)
SOURCE_DIR              = $(PROJECT_DIR)/Source/Source
MAIN_DIR		= $(PROJECT_DIR)/Source
HEADER_DIR		= $(PROJECT_DIR)/Source/Headers
RESOURCE_DIR		= $(PROJECT_DIR)/Source/Resources
INCLUDE_DIRS            = $(HEADER_DIR) \
                          $(OBJ_DIR)
BIN_PATH                = $(BIN_DIR)/$(BIN_FILE)

# include OS dependent content
include $(MAKE_INCLUDE_DIR)/Common.mk

# required libs
LIBS = pthread

# require packages
PACKAGES     =     Qt5Core Qt5Widgets

DEFINES      =

# object files
OBJ_FILES   =   $(OBJ_DIR)/BaseLogger.o \
                $(OBJ_DIR)/Camera.o \
                $(OBJ_DIR)/CameraObserver.o \
                $(OBJ_DIR)/DeviationCalculator.o \
                $(OBJ_DIR)/FrameObserver.o \
                $(OBJ_DIR)/FrameObserverRead.o \
                $(OBJ_DIR)/FrameObserverMMAP.o \
                $(OBJ_DIR)/FrameObserverUSER.o \
                $(OBJ_DIR)/IOHelper.o \
                $(OBJ_DIR)/ImageProcessingThread.o \
                $(OBJ_DIR)/ImageTransform.o \
                $(OBJ_DIR)/Logger.o \
                $(OBJ_DIR)/main.o \
                $(OBJ_DIR)/moc_Camera.o \
                $(OBJ_DIR)/moc_CameraObserver.o \
                $(OBJ_DIR)/moc_DeviationCalculator.o \
                $(OBJ_DIR)/moc_FrameObserver.o \
                $(OBJ_DIR)/moc_ImageProcessingThread.o \
                $(OBJ_DIR)/moc_V4L2Viewer.o \
                $(OBJ_DIR)/MyFrame.o \
                $(OBJ_DIR)/MyFrameQueue.o \
                $(OBJ_DIR)/qrc_V4L2Viewer.o \
                $(OBJ_DIR)/Thread.o \
                $(OBJ_DIR)/V4L2Helper.o \
                $(OBJ_DIR)/V4L2Viewer.o

# generated header files
GEN_HEADERS    = $(OBJ_DIR)/ui_V4L2Viewer.h

.SECONDARY: $(GEN_HEADERS)

all: $(BIN_PATH)

$(OBJ_DIR):
	$(call MACRO_MKDIR,$(OBJ_DIR))

$(BIN_DIR):
	$(call MACRO_MKDIR,$(BIN_DIR))

$(OBJ_DIR)/moc_%.cpp: $(HEADER_DIR)/%.h
	$(call MACRO_MOC,$@,$<)

$(OBJ_DIR)/ui_%.h: $(RESOURCE_DIR)/Forms/%.ui
	$(call MACRO_UIC,$@,$<)

$(OBJ_DIR)/qrc_%.cpp: $(RESOURCE_DIR)/%.qrc
	$(call MACRO_RCC,$@,$<)

$(OBJ_DIR)/%.o: $(SOURCE_DIR)/%.cpp $(HEADER_DIR)/*.h $(GEN_HEADERS) | $(OBJ_DIR)
	$(call MACRO_CXX,$@,$<,$(INCLUDE_DIRS),$(DEFINES),$(PACKAGES))

$(OBJ_DIR)/%.o: $(MAIN_DIR)/%.cpp $(HEADER_DIR)/*.h $(GEN_HEADERS) | $(OBJ_DIR)
	$(call MACRO_CXX,$@,$<,$(INCLUDE_DIRS),$(DEFINES),$(PACKAGES))

$(OBJ_DIR)/%.o: $(OBJ_DIR)/%.cpp $(GEN_HEADERS) | $(OBJ_DIR)
	$(call MACRO_CXX,$@,$<,$(INCLUDE_DIRS),$(DEFINES),$(PACKAGES))

$(BIN_PATH): $(OBJ_FILES) $(BIN_DIR)
	$(call MACRO_LINK_BINARY,$(BIN_PATH),$(OBJ_FILES),$(LIBS),$(LIB_DIRS),$(PACKAGES),,'$$ORIGIN',$(BIN_DIR),,$(FRAMEWORKS))

clean:
	$(call MACRO_RMDIR,$(OBJ_DIR))
	$(call MACRO_RMDIR,$(BIN_DIR))
