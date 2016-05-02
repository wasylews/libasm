TOOLS_PATH = TOOLS
CC = $(TOOLS_PATH)\TASM.EXE
LD = $(TOOLS_PATH)\TLINK.EXE
RM = del
MKDIR = mkdir

CFLAGS = /z /zi
LDFLAGS = /v

BUILD_DIR = build
INCLUDE_DIR = libasm


all: test.exe


.asm.obj:
	# separate build and source folders
	@if not exist $(BUILD_DIR) $(MKDIR) $(BUILD_DIR)

	$(CC) $(CFLAGS) /i$(INCLUDE_DIR) $< $(BUILD_DIR)\$@

.obj.exe:
	$(LD) $(LDFLAGS) $(BUILD_DIR)\$<

clean:
	-@$(RM) $(BUILD_DIR)\*.obj
	-@$(RM) $(BUILD_DIR)\*.exe
	-@$(RM) $(BUILD_DIR)\*.map
