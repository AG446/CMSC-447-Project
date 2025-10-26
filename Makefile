BUILD_PATH := ./build
SRC_PATH := ./src
CXX := g++
EXTERNAL_LIBRARIES := gtk4
DEBUG := -g
#OPTIMIZATIONS := -O2
WARNING_FLAGS := -Wall
SHARED_CFLAGS := $(OPTIMIZATIONS) $(DEBUG) -fmax-errors=10
SHARED_LDFLAGS := -lm

#location of .o files
OBJS_BUILD_PATH := $(BUILD_PATH)/objs
#location of final executable binaries
EXE_BUILD_PATH := $(BUILD_PATH)/bin

CORE_PATH := $(SRC_PATH)/core
CORE_INCLUDES := -I$(CORE_PATH)/includes
CORE_CODE := $(CORE_PATH)/code
CORE_CXX_FILES := $(wildcard $(CORE_CODE)/*.cpp)
CORE_OBJS_BUILD_PATH := $(OBJS_BUILD_PATH)/core
CORE_OBJS := $(addprefix $(CORE_OBJS_BUILD_PATH)/,$(patsubst %.cpp,%.o,$(notdir $(CORE_CXX_FILES))))
$(CORE_OBJS_BUILD_PATH)/%.o: $(CORE_CODE)/%.cpp
	@mkdir -p $(CORE_OBJS_BUILD_PATH)
	$(CXX) $(SHARED_CFLAGS) $(WARNING_FLAGS) -MMD -MP $(CORE_INCLUDES) -c $< -o $@



UI_PATH := $(SRC_PATH)/ui
UI_INCLUDES := -I$(UI_PATH)/includes $(shell pkg-config --cflags $(EXTERNAL_LIBRARIES)) $(CORE_INCLUDES)
UI_CODE := $(UI_PATH)/code
UI_CXX_FILES := $(wildcard $(UI_CODE)/*.cpp)
UI_OBJS_BUILD_PATH := $(OBJS_BUILD_PATH)/ui
UI_OBJS := $(addprefix $(UI_OBJS_BUILD_PATH)/,$(patsubst %.cpp,%.o,$(notdir $(UI_CXX_FILES))))
$(UI_OBJS_BUILD_PATH)/%.o: $(UI_CODE)/%.cpp
	@mkdir -p $(UI_OBJS_BUILD_PATH)
	$(CXX) $(SHARED_CFLAGS) $(WARNING_FLAGS) -MMD -MP $(UI_INCLUDES) -c $< -o $@


TESTS_PATH := $(SRC_PATH)/tests
TESTS_INCLUDES := -I$(TESTS_PATH)/includes $(CORE_INCLUDES)
TESTS_CODE := $(TESTS_PATH)/code
TESTS_CXX_FILES := $(wildcard $(TESTS_CODE)/*.cpp)
TESTS_OBJS_BUILD_PATH := $(OBJS_BUILD_PATH)/tests
TESTS_OBJS := $(addprefix $(TESTS_OBJS_BUILD_PATH)/,$(patsubst %.cpp,%.o,$(notdir $(TESTS_CXX_FILES))))
$(TESTS_OBJS_BUILD_PATH)/%.o: $(TESTS_CODE)/%.cpp
	@mkdir -p $(TESTS_OBJS_BUILD_PATH)
	$(CXX) $(SHARED_CFLAGS) $(WARNING_FLAGS) -MMD -MP $(TESTS_INCLUDES) -c $< -o $@




TESTER_PROGRAM_NAME := tests
$(EXE_BUILD_PATH)/$(TESTER_PROGRAM_NAME) : $(CORE_OBJS) $(TESTS_OBJS)
	@mkdir -p $(EXE_BUILD_PATH)
	$(CXX) $(CORE_OBJS) $(TESTS_OBJS) $(TESTER_INCLUDES) -o $@ $(SHARED_LDFLAGS) $(WARNING_FLAGS)

UI_PROGRAM_NAME := navigator
$(EXE_BUILD_PATH)/$(UI_PROGRAM_NAME) : $(CORE_OBJS) $(UI_OBJS)
	@mkdir -p $(EXE_BUILD_PATH)
	$(CXX) $(CORE_OBJS) $(UI_OBJS) $(UI_INCLUDES)  -o $@ $(SHARED_LDFLAGS) $(shell pkg-config --libs $(EXTERNAL_LIBRARIES)) $(WARNING_FLAGS)

.PHONY: run_tests
run_tests: $(EXE_BUILD_PATH)/$(TESTER_PROGRAM_NAME)
	./$(EXE_BUILD_PATH)/$(TESTER_PROGRAM_NAME)

.PHONY: val_tests
val_tests: $(EXE_BUILD_PATH)/$(TESTER_PROGRAM_NAME)
	valgrind ./$(EXE_BUILD_PATH)/$(TESTER_PROGRAM_NAME)

.PHONY: run_ui
run_ui: $(EXE_BUILD_PATH)/$(UI_PROGRAM_NAME)
	./$(EXE_BUILD_PATH)/$(UI_PROGRAM_NAME)

.PHONY: clean
clean:
	rm -rf $(BUILD_PATH)

-include $(CORE_OBJS:.o=.d)
-include $(TESTS_OBJS:.o=.d)
