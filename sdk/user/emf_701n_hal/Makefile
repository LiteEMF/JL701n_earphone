

DIR_BASE = $(shell pwd)
#当前目录SRC
SRC = $(wildcard *.c)	
#当前.o 文件目录改成$(BUILD_DIR)/*.o目录
OBJ = $(patsubst %.c, $(BUILD_DIR)/%.o,$(SRC))
# OBJ := $(SRC:.c=.o)
# OBJ := $(OBJ:%=$(BUILD_DIR)/%)
#源文件目录
SUB_DIR = $(shell find $(DIR_BASE) -maxdepth 1 -mindepth 1 -type d)


.PHONY: all $(SUB_DIR)

all : $(OBJ) sub_mudule

#.c 编译生成.o
$(OBJ):$(BUILD_DIR)/%.o:%.c
	$(DEBUG) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

sub_mudule:
ifneq ($(SUB_DIR),)
	@for dir in $(SUB_DIR); do \
		if [ -e "$$dir/Makefile" ]; then \
			$(MAKE) -C $$dir; \
		fi \
	done 
endif