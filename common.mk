INSTALL = install

CCTOOLBASE:=${CROSSTOOLS_DIR}
CCLIBS:=${CCTOOLBASE}/usr/arm-buildroot-linux-uclibcgnueabi/sysroot/usr/lib
CCINCS:=${CCTOOLBASE}/usr/arm-buildroot-linux-uclibcgnueabi/sysroot/usr/include


INC_PATHS := \
	${CCINCS} \
	${FIBO_AUTO_GEN} \
	${CCINCS}/qmi \
	${CCINCS}/qmi-framework \
	${CCINCS}/loc-api-v02 \
	${CCINCS}/dsutils \
	${CCINCS}/data \
	${INCLUDE_DIR} \

LIB_PATHS := \
        . \
        ${CCLIBS} \
        $(APP_LIB) 

SO_TYPE := share_lib

INC_FLAGS := $(foreach path,$(INC_PATHS),-I$(path) )
LIB_FLAGS := $(foreach path,$(LIB_PATHS),-L$(path) )

USR_WERROR = #-Werror

CFLAGS += -g $(INC_FLAGS) $(USR_WERROR)

CFLAGS += -DLINUX -D_FEATURE_SUPPORT_SYSLOG_

LDFLAGS= $(LIB_FLAGS)

%.o: %.c
	$(NECHO)echo "compiling $<"
	$(NECHO)$(CC) $(CFLAGS) -c -o $@  $<

%.o: %.cpp
	@echo "compileing $<"
	$(NECHO)g++ $(CFLAGS) -c -o $@  $<