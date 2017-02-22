#Build configurations (possible values: Debug, Release, DebugOpenMP, ReleaseOpenMP)
CONFIG                      = Debug
#Check build configuration
ifneq ($(CONFIG),Debug)
  ifneq ($(CONFIG),Release)
    ifneq ($(CONFIG),DebugOpenMP)
      ifneq ($(CONFIG),ReleaseOpenMP)
	$(error Unsupported configuration)
      endif
    endif
  endif
endif

#Common file prefixes and suffixes
TRANSPORTLAYER_PREFIX       = Vimba
TRANSPORTLAYER_SUFFIX       = .cti

#MACRO_MAKE Parameters:
#1. Directory of make project
#2. Targets
MACRO_MAKE                  = make -C $(1) OS=$(OS) ARCH=$(ARCH) WORDSIZE=$(WORDSIZE) ENDIANESS=$(ENDIANESS) CONFIG=$(CONFIG) CC=$(CC) CXX=$(CXX) FLOATABI=$(FLOATABI) $(2)

#MACRO_MAKE_CUSTOM Parameters:
# 1. Directory of make project
# 2. Targets
# 3. OS
# 4. ARCH
# 5. WORDSIZE
# 6. ENDIANESS
# 7. CONFIG
# 8. CC
# 9. CXX
#10. FLOATABI
MACRO_MAKE_CUSTOM           = make -C $(1) OS=$(3) ARCH=$(4) WORDSIZE=$(5) ENDIANESS=$(6) CONFIG=$(7) CC=$(8) CXX=$(9) FLOATABI=$(10) $(2)

#MACRO_TOUPPER Parameters:
#1. Variable
MACRO_TOUPPER               = $(subst a,A,$(subst b,B,$(subst c,C,$(subst d,D,$(subst e,E,$(subst f,F,$(subst g,G,$(subst h,H,$(subst i,I,$(subst j,J,$(subst k,K,$(subst l,L,$(subst m,M,$(subst n,N,$(subst o,O,$(subst p,P,$(subst q,Q,$(subst r,R,$(subst s,S,$(subst t,T,$(subst u,U,$(subst v,V,$(subst w,W,$(subst x,X,$(subst y,Y,$(subst z,Z,$(1)))))))))))))))))))))))))))

#MACRO_TOLOWER Parameters:
#1. Variable
MACRO_TOLOWER               = $(subst A,a,$(subst B,b,$(subst C,c,$(subst D,d,$(subst E,e,$(subst F,f,$(subst G,g,$(subst H,h,$(subst I,i,$(subst J,j,$(subst K,k,$(subst L,l,$(subst M,m,$(subst N,n,$(subst O,o,$(subst P,p,$(subst Q,q,$(subst R,r,$(subst S,s,$(subst T,t,$(subst U,u,$(subst V,v,$(subst W,w,$(subst X,x,$(subst Y,y,$(subst Z,z,$(1)))))))))))))))))))))))))))

#Helper constants
CONST_CONFIG                = $(call MACRO_TOUPPER,$(CONFIG))
CONST_COMMA                 = ,
