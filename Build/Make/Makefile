#==============================================================================
# Copyright (C) 2018 Allied Vision Technologies.  All Rights Reserved.
#
# Redistribution of this file, in original or modified form, without
# prior written consent of Allied Vision Technologies is prohibited.
#
#------------------------------------------------------------------------------
#
# File:         -Makefile
#
# Description:  -makefile for compiling v4l2 test tool
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
OS                      = $(shell uname -s)
PROJECT_NAME            = v4l2test
OUTPUT                  = binary

# path variables
PROJECT_ROOT_DIR        = ../..
EXTERNALS_DIR			= $(PROJECT_ROOT_DIR)/Externals
MAKE_INCLUDE_DIR		= $(CURDIR)/$(EXTERNALS_DIR)/Makefiles
PROJECT_DIR             = $(OS)_$(ARCH)_$(WORDSIZE)bit/$(CONFIG)
BIN_DIR                 = $(OUTPUT)/$(PROJECT_DIR)
OBJ_DIR                 = object/$(PROJECT_DIR)
BIN_PATH                = $(BIN_DIR)/$(BIN_FILE)
SOURCE_DIR              = $(PROJECT_ROOT_DIR)/Source
INCLUDE_DIRS            = $(SOURCE_DIR) \
                          $(PROJECT_ROOT_DIR) \
                          $(OBJ_DIR)
AVTIMAGETRANSFORM_DIR   = $(EXTERNALS_DIR)/AVTImageTransform

# include OS dependent content
include $(MAKE_INCLUDE_DIR)/$(OS).mk

BIN_FILE = $($(call MACRO_TOUPPER,$(OUTPUT))_PREFIX)$(PROJECT_NAME)$($(call MACRO_TOUPPER,$(OUTPUT))_SUFFIX)

# required libs
LIBS = pthread

# require packages
PACKAGES 	= 	QtCore \
				QtSvg \

# object files
OBJ_FILES	= 	$(OBJ_DIR)/BaseLogger.o \
			  	$(OBJ_DIR)/Camera.o \
			  	$(OBJ_DIR)/CameraObserver.o \
				$(OBJ_DIR)/FrameObserver.o \
			  	$(OBJ_DIR)/FrameObserverRead.o \
			  	$(OBJ_DIR)/FrameObserverMMAP.o \
			  	$(OBJ_DIR)/FrameObserverUSER.o \
			  	$(OBJ_DIR)/ImageProcessingThread.o \
			  	$(OBJ_DIR)/ImageTransf.o \
			  	$(OBJ_DIR)/V4l2Helper.o \
			  	$(OBJ_DIR)/Helper.o \
			  	$(OBJ_DIR)/v4l2test.o \
			  	$(OBJ_DIR)/Logger.o \
                $(OBJ_DIR)/main.o \
				$(OBJ_DIR)/MyFrame.o \
				$(OBJ_DIR)/MyFrameQueue.o \
				$(OBJ_DIR)/Thread.o \
			  	$(OBJ_DIR)/moc_Camera.o \
			  	$(OBJ_DIR)/moc_CameraObserver.o \
				$(OBJ_DIR)/moc_FrameObserver.o \
			  	$(OBJ_DIR)/moc_ImageProcessingThread.o \
				$(OBJ_DIR)/moc_v4l2test.o \
				$(OBJ_DIR)/qrc_v4l2test.o

# header files              
GEN_HEADERS	= $(OBJ_DIR)/ui_v4l2test.h

all: $(BIN_PATH)

$(OBJ_DIR)/moc_%.cpp: $(SOURCE_DIR)/%.h $(OBJ_DIR)
	$(call MACRO_MOC,$@,$<)

$(OBJ_DIR)/moc_%.cpp: $(SOURCE_DIR)/QHexEdit/%.h $(OBJ_DIR)
	$(call MACRO_MOC,$@,$<)

$(OBJ_DIR)/ui_%.h: $(SOURCE_DIR)/res/%.ui $(OBJ_DIR)
	$(call MACRO_UIC,$@,$<)

$(OBJ_DIR)/qrc_%.cpp: $(SOURCE_DIR)/res/%.qrc $(OBJ_DIR)
	$(call MACRO_RCC,$@,$<)

$(OBJ_DIR)/%.o: $(SOURCE_DIR)/%.c $(OBJ_DIR) $(GEN_HEADERS)
	$(call MACRO_CC,$@,$<,$(INCLUDE_DIRS),$(DEFINES),$(PACKAGES))

$(OBJ_DIR)/%.o: $(SOURCE_DIR)/%.cpp $(OBJ_DIR) $(GEN_HEADERS)
	$(call MACRO_CXX,$@,$<,$(INCLUDE_DIRS),$(DEFINES),$(PACKAGES))

$(OBJ_DIR)/%.o: $(SOURCE_DIR)/QHexEdit/%.cpp $(OBJ_DIR) $(GEN_HEADERS)
	$(call MACRO_CXX,$@,$<,$(INCLUDE_DIRS),$(DEFINES),$(PACKAGES))

$(OBJ_DIR)/%.o: $(OBJ_DIR)/%.cpp $(OBJ_DIR) $(GEN_HEADERS)
	$(call MACRO_CXX,$@,$<,$(INCLUDE_DIRS),$(DEFINES),$(PACKAGES))

$(BIN_PATH): $(DEPENDENCIES) $(OBJ_FILES) $(BIN_DIR)
	$(call MACRO_LINK_$(call MACRO_TOUPPER,$(OUTPUT)),$(BIN_PATH),$(OBJ_FILES),$(LIBS),$(LIB_DIRS),$(PACKAGES),,'$$ORIGIN',$(BIN_DIR),,$(FRAMEWORKS))

clean:
	$(call MACRO_RM,$(OBJ_FILES) $(BIN_PATH))
	$(call MACRO_RMDIR,$(OBJ_DIR) $(BIN_DIR))
	
$(OBJ_DIR):
	$(call MACRO_MKDIR,$(OBJ_DIR))

$(BIN_DIR):
	$(call MACRO_MKDIR,$(BIN_DIR))
