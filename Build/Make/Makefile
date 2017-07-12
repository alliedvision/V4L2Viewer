#Operating system
OS                      = $(shell uname -s)
PROJECT_NAME            = v4l2test
OUTPUT                  = binary

BASE_DIR        	= ../../../..
PROJECT_ROOT_DIR        = ../..
EXTERNALS_DIR		= $(PROJECT_ROOT_DIR)/externals
MAKE_INCLUDE_DIR	= $(CURDIR)/$(EXTERNALS_DIR)/Makefiles

include $(MAKE_INCLUDE_DIR)/$(OS).mk

PROJECT_DIR             = $(OS)_$(ARCH)_$(WORDSIZE)bit/$(CONFIG)
BIN_FILE                = $($(call MACRO_TOUPPER,$(OUTPUT))_PREFIX)$(PROJECT_NAME)$($(call MACRO_TOUPPER,$(OUTPUT))_SUFFIX)
BIN_DIR                 = $(OUTPUT)/$(PROJECT_DIR)
OBJ_DIR                 = object/$(PROJECT_DIR)
BIN_PATH                = $(BIN_DIR)/$(BIN_FILE)
AVTIMAGETRANSFORM_DIR   = $(PROJECT_ROOT_DIR)/externals/AVTImageTransform

SOURCE_DIR              = $(PROJECT_ROOT_DIR)/Source
INCLUDE_DIRS            = $(SOURCE_DIR) \
                          $(SOURCE_DIR)/QHexEdit \
                          $(PROJECT_ROOT_DIR) \
                          $(TINYXML_DIR) \
                          $(AVTIMAGETRANSFORM_DIR)/Include \
                          $(OBJ_DIR)

LIBS                    = pthread \
                          VimbaImageTransform \
                          $(SHO_LIB)

ifeq ($(ARCH),arm)
ifeq ($(WORDSIZE),32)
LIB_DIRS                = $(AVTIMAGETRANSFORM_DIR)/DynamicLib/arm_32bit \
                          $(BIN_DIR) 
endif
ifeq ($(WORDSIZE),64)
LIB_DIRS                = $(AVTIMAGETRANSFORM_DIR)/DynamicLib/arm_64bit \
                          $(BIN_DIR) 
endif
endif

ifeq ($(ARCH),x86)
ifeq ($(WORDSIZE),32)
LIB_DIRS                = $(AVTIMAGETRANSFORM_DIR)/DynamicLib/linux_pc_32bit \
                          $(BIN_DIR) 
endif
ifeq ($(WORDSIZE),64)
LIB_DIRS                = $(AVTIMAGETRANSFORM_DIR)/DynamicLib/linux_pc_64bit \
                          $(BIN_DIR) 
endif
endif
			  
DEFINES                 = TIXML_USE_STL

PACKAGES                = QtCore \
			  QtSvg \
                          QtGui \
			  opencv

OBJ_FILES               = $(OBJ_DIR)/BaseLogger.o \
			  $(OBJ_DIR)/Camera.o \
			  $(OBJ_DIR)/CameraObserver.o \
                          $(OBJ_DIR)/FrameObserver.o \
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
                          
GEN_HEADERS             = $(OBJ_DIR)/ui_v4l2test.h

DEPENDENCIES            = 
			             
FRAMEWORKS              = IOKit

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


