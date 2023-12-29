SRC_DIR = src

CXX = g++
CXXFLAGS = -Wall -Wextra

ifeq ($(DEBUG),1)
    CXXFLAGS += -g
	BUILD_DIR = build/debug
else
    CXXFLAGS += -O3
	LDFLAGS = -static
	BUILD_DIR = build/release
endif

COMMON_OBJS_FN = utils.o compression.o encryption.o th128_core.o th128_fix.o th128_parse.o
MAIN_OBJS_FN = $(COMMON_OBJS_FN) main.o
TEST_OBJS_FN = $(COMMON_OBJS_FN) test.o
ALL_OBJS_FN = $(COMMON_OBJS_FN) main.o test.o

MAIN_OBJS = $(addprefix $(BUILD_DIR)/, $(MAIN_OBJS_FN))
TEST_OBJS = $(addprefix $(BUILD_DIR)/, $(TEST_OBJS_FN))

ALL_OBJS = $(addprefix $(BUILD_DIR)/, $(ALL_OBJS_FN))
DEPS = $(ALL_OBJS:%.o=%.d)

MAIN = $(BUILD_DIR)/main.exe
TEST = $(BUILD_DIR)/test.exe


.PHONY: main
main: $(MAIN)

.PHONY: test
test: $(TEST)

.PHONY: clean
clean:
	rm -rf build


$(BUILD_DIR):
	mkdir -p $@

-include $(DEPS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -MMD -MP -MT $@ -o $@ -c $<


$(MAIN): $(MAIN_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^

$(TEST): $(TEST_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^
