include $(MAKE_INCLUDE_DIR)/Common.mk

#Helper
UNAME                  = $(shell uname -m)
DEFAULT_CC             = gcc
DEFAULT_CXX            = g++
CONST_OS               = LINUX
#HINT: Not using the default compiler also requires to set the according strip command. If not set, we don't strip.
ifeq ($(STRIP),)
ifeq ($(CC),$(DEFAULT_CC))
STRIP                  = strip --strip-unneeded $(1)
endif
ifeq ($(CXX),$(DEFAULT_CXX))
STRIP                  = strip --strip-unneeded $(1)
endif
else
STRIP                  = $(STRIP) --strip-unneeded $(1)
endif

#ARCH       possible values: x86, arm, ppc)
#WORDSIZE   possible values: 32, 64)
#ENDIANESS  possible values: little, big)
#FLOATABI   possible values: soft, hard or ignore
ifeq ($(UNAME),i386)
AUTO_ARCH               = x86
AUTO_WORDSIZE           = 32
AUTO_ENDIANESS          = little
AUTO_FLOATABI           = ignore
endif
ifeq ($(UNAME),i486)
AUTO_ARCH               = x86
AUTO_WORDSIZE           = 32
AUTO_ENDIANESS          = little
AUTO_FLOATABI           = ignore
endif
ifeq ($(UNAME),i586)
AUTO_ARCH               = x86
AUTO_WORDSIZE           = 32
AUTO_ENDIANESS          = little
AUTO_FLOATABI           = ignore
endif
ifeq ($(UNAME),i686)
AUTO_ARCH               = x86
AUTO_WORDSIZE           = 32
AUTO_ENDIANESS          = little
AUTO_FLOATABI           = ignore
endif
ifeq ($(UNAME),x86_64)
AUTO_ARCH               = x86
AUTO_WORDSIZE           = 64
AUTO_ENDIANESS          = little
AUTO_FLOATABI           = ignore
endif
ifeq ($(UNAME),amd64)
AUTO_ARCH               = x86
AUTO_WORDSIZE           = 64
AUTO_ENDIANESS          = little
AUTO_FLOATABI           = ignore
endif
ifeq ($(UNAME),armv6l)
AUTO_ARCH               = arm
AUTO_WORDSIZE           = 32
AUTO_ENDIANESS          = little
AUTO_FLOATABI           = soft
endif
ifeq ($(UNAME),armv7l)
AUTO_ARCH               = arm
AUTO_WORDSIZE           = 32
AUTO_ENDIANESS          = little
AUTO_FLOATABI           = hard
endif
ifeq ($(UNAME),aarch64)
AUTO_ARCH               = arm
AUTO_WORDSIZE           = $(shell getconf LONG_BIT)
AUTO_ENDIANESS          = little
AUTO_FLOATABI           = hard
endif
ifeq ($(UNAME),ppc)
AUTO_ARCH               = ppc
AUTO_WORDSIZE           = 32
AUTO_ENDIANESS          = big
AUTO_FLOATABI           = ignore
endif

#Check auto settings
ifneq ($(AUTO_ARCH),x86)
ifneq ($(AUTO_ARCH),arm)
ifneq ($(AUTO_ARCH),ppc)
$(error Invalid auto target architecture)
endif
endif
endif

ifneq ($(AUTO_WORDSIZE),32)
ifneq ($(AUTO_WORDSIZE),64)
$(error Invalid auto word size)
endif
endif

ifneq ($(AUTO_ENDIANESS),little)
ifneq ($(AUTO_ENDIANESS),big)
$(error Invalid auto endianess)
endif
endif

ifneq ($(AUTO_FLOATABI),soft)
ifneq ($(AUTO_FLOATABI),hard)
ifneq ($(AUTO_FLOATABI),ignore)
$(error Invalid float abi)
endif
endif
endif

#Override the following variables for customizations
ARCH                    = $(AUTO_ARCH)
WORDSIZE                = $(AUTO_WORDSIZE)
ENDIANESS               = $(AUTO_ENDIANESS)
FLOATABI               	= $(AUTO_FLOATABI)

#Check custom settings
ifneq ($(ARCH),x86)
ifneq ($(ARCH),arm)
ifneq ($(ARCH),arm64)
ifneq ($(ARCH),ppc)
$(error Invalid target architecture)
endif
endif
endif
endif

ifneq ($(WORDSIZE),32)
ifneq ($(WORDSIZE),64)
$(error Invalid word size)
endif
endif

ifneq ($(ENDIANESS),little)
ifneq ($(ENDIANESS),big)
$(error Invalid endianness)
endif
endif

ifneq ($(FLOATABI),soft)
ifneq ($(FLOATABI),hard)
ifneq ($(FLOATABI),ignore)
$(error Invalid float abi)
endif
endif
endif

#MACRO_CC Parameters:
#1. Output file
#2. Input files
#3. Include dirs
#4. Defines
#5. Packages (pkg-config)
#6. Options (possible values: CC_OPTION_VISIBLE)
CC                      = $(DEFAULT_CC)
ifeq ($(WORDSIZE),32)
ifeq ($(ARCH),x86)
CC_ARCH                 = -m$(WORDSIZE)
CC_THUMB                =
CC_FLOATABI             =
endif
ifeq ($(ARCH),arm)
ifeq ($(FLOATABI),soft)
CC_ARCH                 = -march=armv4t
CC_THUMB                = -marm
CC_FLOATABI             = -mfloat-abi=soft
endif
ifeq ($(FLOATABI),hard)
CC_ARCH                 = -march=armv7
CC_THUMB                = -mthumb
CC_FLOATABI             = -mfloat-abi=hard
endif
endif
endif
ifeq ($(WORDSIZE),64)
ifeq ($(ARCH),x86)
CC_ARCH                 = -m$(WORDSIZE)
CC_THUMB                =
CC_FLOATABI             =
endif
ifeq ($(ARCH),arm)
CC_ARCH                 = -march=armv8-a
CC_THUMB                =
CC_FLOATABI             =
endif		
endif
ifeq ($(CONFIG),Debug)
CC_CONFIG_FLAGS         = -O0 -g
endif
ifeq ($(CONFIG),Release)
CC_CONFIG_FLAGS         = -O3
endif
ifeq ($(CONFIG),DebugOpenMP)
CC_CONFIG_FLAGS         = -O0 -g -fopenmp
endif
ifeq ($(CONFIG),ReleaseOpenMP)
CC_CONFIG_FLAGS         = -O3 -fopenmp
endif
MACRO_CC                = $(CC) $(foreach dir,$(3),-I$(dir)) $(foreach def,$(4),-D$(def)) $(if $(filter CC_OPTION_VISIBLE,$(6)),,-fvisibility=hidden) -fPIC $(CC_CONFIG_FLAGS) $(CC_ARCH) $(CC_FLOATABI) $(CC_THUMB) -o $(1) -c $(2) $(foreach pkg,$(5),$(shell pkg-config --cflags $(pkg)))

#MACRO_CXX Parameters:
#1. Output file
#2. Input files
#3. Include dirs
#4. Defines
#5. Packages (pkg-config)
#6. Options (possible values: CXX_OPTION_VISIBLE)
CXX                     = $(DEFAULT_CXX)
ifeq ($(WORDSIZE),32)
ifeq ($(ARCH),x86)
CXX_ARCH                = -m$(WORDSIZE)
CXX_THUMB               =
CXX_FLOATABI            =
endif
ifeq ($(ARCH),arm)
ifeq ($(FLOATABI),soft)
CXX_ARCH                = -march=armv4t
CXX_THUMB               = -marm
CXX_FLOATABI            = -mfloat-abi=soft
endif
ifeq ($(FLOATABI),hard)
CXX_ARCH                = -march=armv7
CXX_THUMB               = -mthumb
CXX_FLOATABI            = -mfloat-abi=hard
endif
endif
endif
ifeq ($(WORDSIZE),64)
ifeq ($(ARCH),x86)
CXX_ARCH                = -m$(WORDSIZE)
CXX_THUMB               =
CXX_FLOATABI            =
endif
ifeq ($(ARCH),arm)
CXX_ARCH                = -march=armv8-a
CXX_THUMB               =
CXX_FLOATABI            =
endif		
endif
ifeq ($(CONFIG),Debug)
CXX_CONFIG_FLAGS        = -O0 -g
endif
ifeq ($(CONFIG),Release)
CXX_CONFIG_FLAGS        = -O3
endif
ifeq ($(CONFIG),DebugOpenMP)
CXX_CONFIG_FLAGS        = -O0 -g -fopenmp
endif
ifeq ($(CONFIG),ReleaseOpenMP)
CXX_CONFIG_FLAGS        = -O3 -fopenmp
endif
MACRO_CXX               = $(CXX) -std=c++98 $(foreach dir,$(3),-I$(dir)) $(foreach def,$(4),-D$(def)) $(if $(filter CXX_OPTION_VISIBLE,$(6)),,-fvisibility=hidden) -fPIC $(CXX_CONFIG_FLAGS) $(CXX_ARCH) $(CXX_FLOATABI) $(CXX_THUMB) -o $(1) -c $(2) $(foreach pkg,$(5),$(shell pkg-config --cflags $(pkg)))

#MACRO_MKDIR Parameters:
#1. Directories to create
MACRO_MKDIR             = mkdir -p $(1)

#MACRO_RMDIR Parameters:
#1. Directories to delete
MACRO_RMDIR             = if [ -d "$(1)" ]; then rmdir -p --ignore-fail-on-non-empty $(1); fi

#MACRO_RMDIR_FORCE Parameters:
#1. Directories to delete
MACRO_RMDIR_FORCE       = if [ -d "$(1)" ]; then rm -Rf $(1); fi

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
#9. used by Darwin.mk as install name
#10. used by Darwin.mk as additional framework
MACRO_LINK_BINARY       = $(CXX) $(CXX_ARCH) $(CXX_FLOATABI) $(CXX_THUMB) $(if $(6),-Wl$(CONST_COMMA)--retain-symbols-file=$(6) $(foreach symbol,$(shell cat $(6)),-Wl$(CONST_COMMA)-u$(CONST_COMMA)$(symbol))) -o $(1) $(2) $(foreach dir,$(4),-L$(dir)) $(foreach dir,$(7),-Wl$(CONST_COMMA)-rpath$(CONST_COMMA)$(dir)) $(foreach dir,$(8),-Wl$(CONST_COMMA)-rpath-link$(CONST_COMMA)$(dir)) $(foreach lib,$(3),$(if $(findstring STATIC_,$(lib)),$(patsubst STATIC_%,-Wl$(CONST_COMMA)-Bstatic -l%, $(lib)),-Wl$(CONST_COMMA)-Bdynamic -l$(lib))) $(patsubst %,-L%,$(subst :, ,$(ADD_LIBRARY_PATH))) $(foreach pkg,$(5),$(shell pkg-config --libs $(pkg))) -Wl,--no-undefined

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
#9. used by Darwin.mk as install name
#10. used by Darwin.mk as additional framework
MACRO_LINK_DYNAMIC      = $(call MACRO_LINK_BINARY,$(1),$(2),$(3),$(4),$(5),$(6),$(7),$(8)) -shared

#MACRO_MOC Parameters:
#1. Output file
#2. Input file
PKGCFG_MOC		= $(shell pkg-config --variable=moc_location QtCore)
MACRO_MOC               = $(if $(PKGCFG_MOC),$(PKGCFG_MOC),moc) -o $(1) $(2)

#MACRO_UIC Parameters:
#1. Output file
#2. Input file
PKGCFG_UIC		= $(shell pkg-config --variable=uic_location QtCore)
MACRO_UIC               = $(if $(PKGCFG_UIC),$(PKGCFG_UIC),uic) -o $(1) $(2)

#MACRO_RCC Parameters:
#1. Output file
#2. Input file
MACRO_RCC               = rcc -o $(1) $(2)

#MACRO_CP Parameters:
#1. Source
#2. Destination
MACRO_CP                = cp -r $(1) $(2) 

#MACRO_RSYNC Parameters:
#1. Source
#2. Destination
#3. Excludes
MACRO_RSYNC             = rsync -avz $(foreach ex,$(3),--exclude $(ex)) $(1) $(2)

#MACRO_RM_EXEC_AND_STRIP
#Removes the executable bit of a binary and strips unneeded symbols
#1. Binary
MACRO_RM_EXEC_AND_STRIP	= chmod -x $(1); $(STRIP)

#OS specific file prefixes and suffixes
BINARY_PREFIX           =
BINARY_SUFFIX           =
STATIC_PREFIX           = lib
STATIC_SUFFIX           = .a
DYNAMIC_PREFIX          = lib
DYNAMIC_SUFFIX          = .so

#OS specific libraries
SHO_LIB                 = rt
