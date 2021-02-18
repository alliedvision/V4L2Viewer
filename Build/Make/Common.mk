#Build configurations (possible values: Debug, Release, DebugOpenMP, ReleaseOpenMP)
CONFIG                      = Debug
#Check build configuration
ifneq ($(CONFIG),Debug)
  ifneq ($(CONFIG),Release)
    $(error Unsupported configuration)
  endif
endif

#Helper constants
CONST_COMMA            = ,
UNAME                  = $(shell uname -m)
DEFAULT_CC             = gcc
DEFAULT_CXX            = g++
CONST_OS               = LINUX

#ARCH       possible values: x86, arm, ppc)
#WORDSIZE   possible values: 32, 64)
#FLOATABI   possible values: soft, hard or ignore
ifeq ($(UNAME),i386)
  AUTO_ARCH               = x86
  AUTO_WORDSIZE           = 32
  AUTO_FLOATABI           = ignore
endif
ifeq ($(UNAME),i486)
  AUTO_ARCH               = x86
  AUTO_WORDSIZE           = 32
  AUTO_FLOATABI           = ignore
endif
ifeq ($(UNAME),i586)
  AUTO_ARCH               = x86
  AUTO_WORDSIZE           = 32
  AUTO_FLOATABI           = ignore
endif
ifeq ($(UNAME),i686)
  AUTO_ARCH               = x86
  AUTO_WORDSIZE           = 32
  AUTO_FLOATABI           = ignore
endif
ifeq ($(UNAME),x86_64)
  AUTO_ARCH               = x86
  AUTO_WORDSIZE           = 64
  AUTO_FLOATABI           = ignore
endif
ifeq ($(UNAME),amd64)
  AUTO_ARCH               = x86
  AUTO_WORDSIZE           = 64
  AUTO_FLOATABI           = ignore
endif
ifeq ($(UNAME),armv6l)
  AUTO_ARCH               = arm
  AUTO_WORDSIZE           = 32
  AUTO_FLOATABI           = soft
endif
ifeq ($(UNAME),armv7l)
  AUTO_ARCH               = arm
  AUTO_WORDSIZE           = 32
  AUTO_FLOATABI           = hard
endif
ifeq ($(UNAME),aarch64)
  AUTO_ARCH               = arm
  AUTO_WORDSIZE           = $(shell getconf LONG_BIT)
  AUTO_FLOATABI           = hard
endif
ifeq ($(UNAME),ppc)
  AUTO_ARCH               = ppc
  AUTO_WORDSIZE           = 32
  AUTO_FLOATABI           = ignore
endif

#Check auto settings
ifneq ($(AUTO_ARCH),x86)
  ifneq ($(AUTO_ARCH),arm)
    ifneq ($(AUTO_ARCH),ppc)
      $(error Invalid auto target architecture $(AUTO_ARCH))
    endif
  endif
endif

ifneq ($(AUTO_WORDSIZE),32)
  ifneq ($(AUTO_WORDSIZE),64)
    $(error Invalid auto word size $(AUTO_WORDSIZE))
  endif
endif

ifneq ($(AUTO_FLOATABI),soft)
  ifneq ($(AUTO_FLOATABI),hard)
    ifneq ($(AUTO_FLOATABI),ignore)
      $(error Invalid auto float abi $(AUTO_FLOATABI))
    endif
  endif
endif

#Override the following variables for customizations
ARCH                    = $(AUTO_ARCH)
WORDSIZE                = $(AUTO_WORDSIZE)
FLOATABI                = $(AUTO_FLOATABI)

#Check custom settings
ifneq ($(ARCH),x86)
  ifneq ($(ARCH),arm)
    ifneq ($(ARCH),ppc)
      $(error Invalid target architecture $(ARCH))
    endif
  endif
endif

ifneq ($(WORDSIZE),32)
  ifneq ($(WORDSIZE),64)
    $(error Invalid word size $(WORDSIZE))
  endif
endif

ifneq ($(FLOATABI),soft)
  ifneq ($(FLOATABI),hard)
    ifneq ($(FLOATABI),ignore)
      $(error Invalid float abi $(FLOATABI))
    endif
  endif
endif

#MACRO_CXX Parameters:
#1. Output file
#2. Input files
#3. Include dirs
#4. Defines
#5. Packages (pkg-config)
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
MACRO_CXX               = $(CXX) -std=c++98 $(foreach dir,$(3),-I$(dir)) $(foreach def,$(4),-D$(def)) -fvisibility=hidden -fPIC $(CXX_CONFIG_FLAGS) $(CXX_ARCH) $(CXX_FLOATABI) $(CXX_THUMB) -o $(1) -c $(2) $(foreach pkg,$(5),$(shell pkg-config --cflags $(pkg)))

#MACRO_MKDIR Parameters:
#1. Directories to create
MACRO_MKDIR             = mkdir -p $(1)

#MACRO_RMDIR Parameters:
#1. Directories to delete
MACRO_RMDIR             = if [ -d "$(1)" ]; then rm -Rf $(1); fi

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
MACRO_LINK_BINARY       = $(CXX) $(CXX_ARCH) $(CXX_FLOATABI) $(CXX_THUMB) $(if $(6),-Wl$(CONST_COMMA)--retain-symbols-file=$(6) $(foreach symbol,$(shell cat $(6)),-Wl$(CONST_COMMA)-u$(CONST_COMMA)$(symbol))) -o $(1) $(2) $(foreach dir,$(4),-L$(dir)) $(foreach dir,$(7),-Wl$(CONST_COMMA)-rpath$(CONST_COMMA)$(dir)) $(foreach dir,$(8),-Wl$(CONST_COMMA)-rpath-link$(CONST_COMMA)$(dir)) $(foreach lib,$(3),$(if $(findstring STATIC_,$(lib)),$(patsubst STATIC_%,-Wl$(CONST_COMMA)-Bstatic -l%, $(lib)),-Wl$(CONST_COMMA)-Bdynamic -l$(lib))) $(patsubst %,-L%,$(subst :, ,$(ADD_LIBRARY_PATH))) $(foreach pkg,$(5),$(shell pkg-config --libs $(pkg))) -Wl,--no-undefined

#MACRO_MOC Parameters:
#1. Output file
#2. Input file
PKGCFG_MOC              = $(shell pkg-config --variable=moc_location QtCore)
MACRO_MOC               = $(if $(PKGCFG_MOC),$(PKGCFG_MOC),moc) -o $(1) $(2)

#MACRO_UIC Parameters:
#1. Output file
#2. Input file
PKGCFG_UIC              = $(shell pkg-config --variable=uic_location QtCore)
MACRO_UIC               = $(if $(PKGCFG_UIC),$(PKGCFG_UIC),uic) -o $(1) $(2)

#MACRO_RCC Parameters:
#1. Output file
#2. Input file
MACRO_RCC               = rcc -o $(1) $(2)
