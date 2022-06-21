# The directories containing the source files, separated by ':'
# "VPATH" is a builtin by make, do not rename
VPATH=src

# Output folders
OUT_DIR_DEP_DEBUG   = _dep.debug
OUT_DIR_OBJ_DEBUG   = _obj.debug
OUT_DIR_BIN_DEBUG   = _bin.debug
OUT_DIR_GEN_DEBUG   = _gen.debug
OUT_DIR_DEP_RELEASE = _dep.release
OUT_DIR_OBJ_RELEASE = _obj.release
OUT_DIR_BIN_RELEASE = _bin.release
OUT_DIR_GEN_RELEASE = _gen.release

# Default build type
ifeq ($(BUILD_TYPE),)
BUILD_TYPE = release
endif

ifeq ($(BUILD_TYPE),debug)
OUT_DIR_DEP = $(OUT_DIR_DEP_DEBUG)
OUT_DIR_OBJ = $(OUT_DIR_OBJ_DEBUG)
OUT_DIR_BIN = $(OUT_DIR_BIN_DEBUG)
OUT_DIR_GEN = $(OUT_DIR_GEN_DEBUG)
else
OUT_DIR_DEP = $(OUT_DIR_DEP_RELEASE)
OUT_DIR_OBJ = $(OUT_DIR_OBJ_RELEASE)
OUT_DIR_BIN = $(OUT_DIR_BIN_RELEASE)
OUT_DIR_GEN = $(OUT_DIR_GEN_RELEASE)
endif

# List of sources (regardless of directories), located by VPATH
Group0_SRC = \
	Main.c \
	Assert.c \
	Settings.c\
	InputOutput.c \
	Errors.c \
	Stack.c \
	Queue.c \
	List.c \
	Evaluator.c \
	EvaluatorFunctions.c \
	EvaluatorCommands.c \
	StringOps.c \
	Types.c

# Build a Dependency list and an Object list, by replacing the .c
# extension to .d for dependency files, and .o for object files.
Group0_DEP = $(patsubst %.c, $(OUT_DIR_DEP)/Group0_%.d, ${Group0_SRC})
Group0_OBJ = $(patsubst %.c, $(OUT_DIR_OBJ)/Group0_%.o, ${Group0_SRC})


# The final binary
TARGET = nopf

# What compiler to use for generating dependencies: 
# it will be invoked with -MM -MP
CCDEP = gcc
CC = gcc

# What include flags to pass to the compiler
INC_FLAGS = -I src -I $(OUT_DIR_GEN)

C_FLAGS_COMMON = -std=c99 -Wall -pedantic -Wshadow -Wunused-function -Wunused-label -Wunused-value -Wunused-variable

ifeq ($(BUILD_TYPE),debug)
C_FLAGS += -g $(C_FLAGS_COMMON) -D_DEBUG -DBUILD_TYPE=debug ${INC_FLAGS}
else
C_FLAGS += -std=c99 -O2 -Wuninitialized $(C_FLAGS_COMMON) ${INC_FLAGS}
endif

# Common linker flags for all build types
LD_FLAGS += -ltermcap -lreadline -lm

all: begin $(OUT_DIR_BIN)/${TARGET} done

# This is fugly, I don't know how to have a dependency on Errors.c before "all" rule
# Ugh, right now it works so meh.
src/Errors.c : $(OUT_DIR_GEN)/GenErrorData.h

begin:
ifneq ($(BUILD_TYPE),release)
ifneq ($(BUILD_TYPE),debug)
@echo "BUILD_TYPE '"$(BUILD_TYPE)"' is invalid."
@echo "You must specify a build type when running make, e.g."
@echo  "make BUILD_TYPE=debug"
@echo
@echo  "Valid BUILD_TYPE values 'release', 'debug'"
@exit 1
endif
endif

$(OUT_DIR_GEN)/GenErrorData.h: src/Errors.h
	@mkdir -p $(OUT_DIR_GEN)
	@echo "Generating error codes into $@"
	@sed -f src/ErrorData.sed src/Errors.h > $@

done:
	@echo "Done."

$(OUT_DIR_BIN)/${TARGET}: ${Group0_OBJ} | begin
	@mkdir -p $(dir $@)
	$(CC) -g -o $@ $^ ${LD_FLAGS}

$(OUT_DIR_OBJ)/Group0_%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) -c $(C_FLAGS) -o $@ $<

$(OUT_DIR_DEP)/Group0_%.d: %.c
	@mkdir -p $(dir $@)
	@echo Generating $(BUILD_TYPE) dependencies for $<
	@set -e ; $(CCDEP) -MM -MP $(INC_FLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,$(OUT_DIR_OBJ)\/Group0_\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

clean:
	@rm -rf \
	$(OUT_DIR_DEP_DEBUG) $(OUT_DIR_OBJ_DEBUG) $(OUT_DIR_BIN_DEBUG) $(OUT_DIR_GEN_DEBUG) \
	$(OUT_DIR_DEP_RELEASE) $(OUT_DIR_OBJ_RELEASE) $(OUT_DIR_BIN_RELEASE) $(OUT_DIR_GEN_RELEASE)

# Unless "make clean" is called, include the dependency files
# which are auto-generated. Don't fail if they are missing
# (-include), since they will be missing in the first invocation!
ifneq ($(MAKECMDGOALS),clean)
-include ${Group0_DEP}
endif

