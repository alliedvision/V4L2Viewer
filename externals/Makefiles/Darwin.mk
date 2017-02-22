include $(MAKE_INCLUDE_DIR)/Common.mk

UNAME                   = $(shell uname -m)

#ARCH       possible values: x86)
#WORDSIZE   possible values: 32, 64)
#ENDIANESS  possible values: little)
ifeq ($(UNAME),x86_64)
AUTO_ARCH               = x86
AUTO_WORDSIZE           = 64
AUTO_ENDIANESS          = little
endif

#Check auto settings
ifneq ($(AUTO_ARCH),x86)
ifneq ($(AUTO_ARCH),arm)
$(error Invalid auto target architecture)
endif
endif

ifneq ($(AUTO_WORDSIZE),32)
ifneq ($(AUTO_WORDSIZE),64)
$(error Invalid auto word size)
endif
endif

ifneq ($(AUTO_ENDIANESS),little)
$(error Invalid auto endianess)
endif

#Override the following variables for customizations
ARCH                    = $(AUTO_ARCH)
WORDSIZE                = $(AUTO_WORDSIZE)
ENDIANESS               = $(AUTO_ENDIANESS)

#Check custom settings
ifneq ($(ARCH),x86)
ifneq ($(ARCH),arm)
$(error Invalid target architecture)
endif
endif

ifneq ($(WORDSIZE),32)
ifneq ($(WORDSIZE),64)
$(error Invalid word size)
endif
endif

ifneq ($(ENDIANESS),little)
$(error Invalid endianess)
endif

#Helper constants
CONST_OS                = OSX

#MACRO_CC Parameters:
#1. Output file
#2. Input files
#3. Include dirs
#4. Defines
#5. Packages (pkg-config)
#6. Options (possible values: CC_OPTION_VISIBLE)
DEFAULT_CC              = clang
CC                      = $(DEFAULT_CC)
ifeq ($(ARCH),x86)
CC_ARCH                 = -m$(WORDSIZE)
endif
ifeq ($(CONFIG),Debug)
CC_CONFIG_FLAGS         = -O0 -g
endif
ifeq ($(CONFIG),Release)
CC_CONFIG_FLAGS         = -O3
endif
MACRO_CC                = $(CC) $(foreach dir,$(3),-I$(dir)) $(foreach def,$(4),-D$(def)) $(if $(filter CC_OPTION_VISIBLE,$(6)),,-fvisibility=hidden) -fPIC $(CC_CONFIG_FLAGS) $(CC_ARCH) -o $(1) -c $(2) $(foreach pkg,$(5),$(shell pkg-config --cflags $(pkg)))

#MACRO_CXX Parameters:
#1. Output file
#2. Input files
#3. Include dirs
#4. Defines
#5. Packages (pkg-config)
#6. Options (possible values: CXX_OPTION_VISIBLE)
DEFAULT_CXX             = clang++
CXX                     = $(DEFAULT_CXX)
ifeq ($(ARCH),x86)
CXX_ARCH                 = -m$(WORDSIZE)
endif
ifeq ($(CONFIG),Debug)
CXX_CONFIG_FLAGS        = -O0 -g
endif
ifeq ($(CONFIG),Release)
CXX_CONFIG_FLAGS        = -O3
endif
MACRO_CXX               = $(CXX) -mmacosx-version-min=10.5 -std=c++98 $(foreach dir,$(3),-I$(dir)) $(foreach def,$(4),-D$(def)) $(if $(filter CXX_OPTION_VISIBLE,$(6)),,-fvisibility=hidden) -fPIC $(CXX_CONFIG_FLAGS) $(CXX_ARCH) -o $(1) -c $(2) $(foreach pkg,$(5),$(shell pkg-config --cflags $(pkg)))

#MACRO_MKDIR Parameters:
#1. Directories to create
MACRO_MKDIR             = mkdir -p $(1)

#MACRO_RMDIR Parameters:
#1. Directories to delete
MACRO_RMDIR             = rm -dfR $(1)

#MACRO_RM Parameters:
#1. Files to delete
MACRO_RM                = rm -f $(1)

#MACRO_LINK_BINARY Parameters:
#1. Output file
#2. Input files
#3. Libraries
#4. Library dirs
#5. Packages (pkg-config)
#6. Symbols file
#7. rpath linker paths
#8. rpath-link linker paths
#9. install name
#10. additional framework
MACRO_LINK_BINARY       = $(CXX) -mmacosx-version-min=10.5 $(CXX_ARCH) $(if $(6),-Wl$(CONST_COMMA) $(foreach symbol,$(shell cat $(6)),-Wl$(CONST_COMMA)-u$(CONST_COMMA)_$(symbol))) -o $(1) $(2) $(foreach dir,$(4),-L$(dir)) -framework CoreFoundation $(foreach dir,$(7),-Wl$(CONST_COMMA)-rpath$(CONST_COMMA)$(dir)) $(foreach name,$(9),-install_name $(name)) $(foreach lib,$(3),$(if $(findstring STATIC_,$(lib)),$(patsubst STATIC_%,-Wl$(CONST_COMMA)-l%, $(lib)),-Wl$(CONST_COMMA)-l$(lib))) $(foreach pkg,$(5),$(shell pkg-config --libs $(pkg))) $(foreach fwk,$(10),-framework $(fwk))

#MACRO_LINK_STATIC Parameters:
#1. Output file
#2. Input files
MACRO_LINK_STATIC       = ar r $(1) $(2)

#MACRO_LINK_DYNAMIC Parameters:
#1. Output file
#2. Input files
#3. Libraries
#4. Library dirs
#5. Packages (pkg-config)
#6. Symbols file
#7. Runtime linker paths
#8. rpath-link linker paths
#9. install name
#10. additional framework
MACRO_LINK_DYNAMIC      = $(call MACRO_LINK_BINARY,$(1),$(2),$(3),$(4),$(5),$(6),$(7),$(8),$(9),$(10)) -shared

#MACRO_MOC Parameters:
#1. Output file
#2. Input file
PKGCFG_MOC				= $(shell pkg-config --variable=moc_location QtCore)
MACRO_MOC               = $(if $(PKGCFG_MOC),$(PKGCFG_MOC),moc) -o $(1) $(2)

#MACRO_UIC Parameters:
#1. Output file
#2. Input file
PKGCFG_UIC				= $(shell pkg-config --variable=uic_location QtCore)
MACRO_UIC               = $(if $(PKGCFG_UIC),$(PKGCFG_UIC),uic) -o $(1) $(2)

#MACRO_RCC Parameters:
#1. Output file
#2. Input file
MACRO_RCC               = rcc -o $(1) $(2)

#MACRO_CP Parameters:
#1. Source
#2. Destination
MACRO_CP                = cp $(1) $(2)

#MACRO_RSYNC Parameters:
#1. Source
#2. Destination
#3. Excludes
MACRO_RSYNC             = rsync -avz $(foreach ex,$(3),--exclude $(ex)) $(1) $(2)

#OS specific file prefixes and suffixes
BINARY_PREFIX           =
BINARY_SUFFIX           =
STATIC_PREFIX           = lib
STATIC_SUFFIX           = .a
DYNAMIC_PREFIX          = lib
DYNAMIC_SUFFIX          = .dylib

#OS specific libraries
SHO_LIB                 =
