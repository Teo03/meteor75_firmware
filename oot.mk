EXTERNAL_SRC_DIR := $(ROOT)/src_passthrough

CXXFLAGS = $(filter-out -std=gnu17 -Wold-style-definition -Wstrict-prototypes,$(CFLAGS)) -fno-rtti -fno-exceptions -std=c++17 -I $(EXTERNAL_SRC_DIR) -Wno-unused-parameter -Wno-unused-variable -Wno-double-promotion -Wno-register

CXX_SRC = rl_tools/policy.cpp
TARGET_OBJS += $(addsuffix .o,$(addprefix $(TARGET_OBJ_DIR)/,$(basename $(CXX_SRC))))

$(TARGET_OBJ_DIR)/%.o: $(EXTERNAL_SRC_DIR)/%.cpp
	$(V1) mkdir -p $(dir $@)
	$(V1) $(CROSS_CXX) -c -o $@ $(CXXFLAGS) $(CC_DEFAULT_OPTIMISATION) $<
