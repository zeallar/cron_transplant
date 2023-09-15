include ../common.mk

LDFLAGS += -lumdp -lumdpcommon -lsqlite -lpthread  -lrt 
#FLAGS =-I /bin/snmpsql/sqlite/include
#CFLAGS + = -Wextra -std=c89 -s -Os -ffunction-sections -fdata-sections -Wl,--gc-sections -Wl,-z,norelro -static -DVERSION=\"$(VERSION)\" -DCRON_USE_LOCAL_TIME
CFLAGS + =  -Wall -Wextra -std=c89 -DCRON_TEST_MALLOC
OBJDIR = Object
SRCS_DIR := $(wildcard ./*.c)

OBJS := $(addsuffix .o,$(addprefix $(OBJDIR)/,$(basename $(notdir $(SRCS_DIR)))))
DEPENDS := $(addsuffix .d,$(OBJS))
  
TARGET = tinyCron
TARGET_EXE = $(OBJDIR)/$(TARGET)
all: distclean $(TARGET_EXE)
 
$(TARGET_EXE) : $(OBJS)
	@echo "linking $@"
	$(CC) -o $@ $(OBJS) -Wl,-start-group $(LDFLAGS) -Wl,-end-group $(CFLAGS)
	$(STRIP) -g $(TARGET_EXE)
	chmod a+x $@
#	$(INSTALL) -c -m 777 $(TARGET_EXE) $(APP_BIN)

define make-cmd-cc
$2 : $1
	@mkdir -p $(OBJDIR)
	$$(CC) $$(CFLAGS) -MMD -MT $$@ -MF $$@.d -c -o $$@ $$<   
endef
 
$(foreach afile,$(SRCS_DIR),\
	$(eval $(call make-cmd-cc,$(afile),\
        $(addsuffix .o,$(addprefix $(OBJDIR)/,$(basename $(notdir $(afile))))))))

clean:
	$(RM) -rf $(OBJS) $(TARGET_EXE) $(OBJDIR)

distclean: clean
	$(RM) $(APP_BIN)/$(TARGET) -f
 
-include $(DEPENDS)
