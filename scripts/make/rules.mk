# Build rules

# Split xxx-y to xxx-dirs & xxx-files

obj-dirs := $(addprefix $(BUILD)/, $(filter %/, $(obj-y)))
obj-files := $(addprefix $(BUILD)/, $(filter %.o, $(obj-y)))
boot-dirs := $(addprefix $(BUILD)/, $(filter %/, $(boot-y)))
boot-files := $(addprefix $(BUILD)/, $(filter-out %/, $(boot-y)))
boot-files := $(boot-files:.o=.boot.o)

# Default build rule

all: $(BUILD)/kernel.o $(BUILD)/boot.o
	$(info $(RELAPATH): built all: $^)

# Make dependency files

bdeps := $(obj-files) $(boot-files)
bdeps := $(bdeps:.o=.d)
bdeps := $(bdeps:.i=.d)
bdeps := $(bdeps:.s=.d)
-include $(bdeps)

$(BUILD)/%.d:
	$(NOOP)

# Rule for errors

.DEFAULT:
	$(error no rule for $@ (deps=$^) (pwd=$(shell pwd)))

# Rule for kernel.o

.PHONY: $(BUILD)/kernel.o
$(BUILD)/kernel.o: $(obj-files) $(addsuffix kernel.o,$(obj-dirs))
	$(Q)$(LD) -r -o $@ /dev/null $^
ifeq ($(V),2)
	$(info $(RELAPATH): built kernel.o with $^)
endif

$(BUILD)/%/kernel.o: FORCE
ifeq ($(V),2)
	$(info $(RELAPATH): descending into $@)
endif
	$(Q)$(MKDIR) -p $(dir $@)
	$(Q)$(MAKE) $(MAKEOPTS) -C $(dir $(SRC_ROOT)/$(shell realpath -m --relative-to=$(BUILD_ROOT) $@)) $(MAKEARGS) $@

# Rule for boot.o

.PHONY: $(BUILD)/boot.o
$(BUILD)/boot.o: $(boot-files) $(addsuffix boot.o,$(boot-dirs))
	$(Q)$(LD) -r -o $@ /dev/null $^
ifeq ($(V),2)
	$(info $(RELAPATH): built boot.o with $^)
endif

$(BUILD)/%/boot.o: FORCE
ifeq ($(V),2)
	$(info $(RELAPATH): descending into $@)
endif
	$(Q)$(MKDIR) -p $(dir $@)
	$(Q)$(MAKE) $(MAKEOPTS) -C $(dir $(SRC_ROOT)/$(shell realpath -m --relative-to=$(BUILD_ROOT) $@)) $(MAKEARGS) $@

# Kconfig update rule
kconfig-output := $(BUILD)/include/generated/autoconf.h $(BUILD)/include/generated/rustc_cfg $(BUILD)/include/config/auto.conf $(BUILD)/include/config/auto.conf.cmd
$(kconfig-output): $(CONFIG)
ifeq ($(CC),include-what-you-use)
	$(NOOP)
else
	$(Q)$(CONF) --syncconfig Kconfig
endif

# Don't build source files

noop := $(wildcard $(SRC)/*.c $(SRC_ROOT)/make/*) Makefile
$(noop):
	$(NOOP)

# Force rule
FORCE:

# Clean rule
clean-all := $(obj-y) $(obj-) $(boot-y:.o=.boot.o) $(boot-:.o=.boot.o)
clean-dirs := $(addprefix $(SRC)/, $(filter %/, $(clean-all)))
clean-files := $(addprefix $(BUILD)/, $(clean-y) $(filter-out %/, $(clean-all)))
.PHONY: clean
clean:
	$(Q)for dir in $(clean-dirs); do $(MAKE) $(MAKEOPTS) -C "$${dir}" $(MAKEARGS) clean; done
	$(Q)rm -rfv $(clean-files) $(bdeps) $(BUILD)/kernel.o $(BUILD)/boot.o

# C build rules

$(BUILD)/%.i: $(SRC)/%.c
	$(Q)$(CC) $(CFLAGS) $(CPPFLAGS) -MMD -MP -E -o $@ $<

$(BUILD)/%.i: $(BUILD)/%.c
	$(Q)$(CC) $(CFLAGS) $(CPPFLAGS) -MMD -MP -E -o $@ $<

$(BUILD)/%.s: $(SRC)/%.c
	$(Q)$(CC) $(CFLAGS) $(CPPFLAGS) -MMD -MP -S -o $@ $<

$(BUILD)/%.s: $(BUILD)/%.c
	$(Q)$(CC) $(CFLAGS) $(CPPFLAGS) -MMD -MP -S -o $@ $<

$(BUILD)/%.s: $(BUILD)/%.i
	$(Q)$(CC) $(CFLAGS) -S -o $@ $<

$(BUILD)/%.o: $(SRC)/%.c
	$(Q)$(CC) $(CFLAGS) $(CPPFLAGS) -MMD -MP -c -o $@ $<

$(BUILD)/%.o: $(BUILD)/%.c
	$(Q)$(CC) $(CFLAGS) $(CPPFLAGS) -MMD -MP -c -o $@ $<

$(BUILD)/%.o: $(BUILD)/%.i
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

# C++ build rules

$(BUILD)/%.ii: $(SRC)/%.cc
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MMD -MP -E -o $@ $<

$(BUILD)/%.ii: $(BUILD)/%.cc
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MMD -MP -E -o $@ $<

$(BUILD)/%.s: $(SRC)/%.cc
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MMD -MP -S -o $@ $<

$(BUILD)/%.s: $(BUILD)/%.cc
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MMD -MP -S -o $@ $<

$(BUILD)/%.s: $(BUILD)/%.ii
	$(CXX) $(CXXFLAGS) -S -o $@ $<

$(BUILD)/%.o: $(SRC)/%.cc
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MMD -MP -c -o $@ $<

$(BUILD)/%.o: $(BUILD)/%.cc
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MMD -MP -c -o $@ $<

$(BUILD)/%.o: $(BUILD)/%.ii
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Assembly build rules

$(BUILD)/%.s: $(SRC)/%.S
	$(Q)$(CC) $(ASFLAGS) $(CPPFLAGS) -MMD -MP -E -o $@ $<

$(BUILD)/%.s: $(BUILD)/%.S
	$(Q)$(CC) $(ASFLAGS) $(CPPFLAGS) -MMD -MP -E -o $@ $<

$(BUILD)/%.o: $(SRC)/%.S
	$(Q)$(CC) $(ASFLAGS) $(CPPFLAGS) -MMD -MP -c -o $@ $<

$(BUILD)/%.o: $(BUILD)/%.S
	$(Q)$(CC) $(ASFLAGS) $(CPPFLAGS) -MMD -MP -c -o $@ $<

$(BUILD)/%.o: $(BUILD)/%.s
	$(Q)$(CC) $(ASFLAGS) -c -o $@ $<

# Bootloader build rules

$(BUILD)/%.boot.o: $(SRC)/%.c
	$(Q)$(CC) $(CFLAGS) $(CPPFLAGS) $(BOOT_FLAGS) -MMD -MP -c -o $@ $<

$(BUILD)/%.boot.o: $(BUILD)/%.boot.c
	$(Q)$(CC) $(CFLAGS) $(CPPFLAGS) $(BOOT_FLAGS) -MMD -MP -c -o $@ $<

$(BUILD)/%.boot.o: $(BUILD)/%.boot.i
	$(Q)$(CC) $(CFLAGS) $(BOOT_FLAGS) -c -o $@ $<

$(BUILD)/%.boot.o: $(SRC)/%.S
	$(Q)$(CC) $(CPPFLAGS) $(BOOT_FLAGS) -MMD -MP -c -o $@ $<

$(BUILD)/%.boot.o: $(BUILD)/%.boot.S
	$(Q)$(CC) $(CPPFLAGS) $(BOOT_FLAGS) -MMD -MP -c -o $@ $<

$(BUILD)/%.boot.o: $(BUILD)/%.boot.s
	$(Q)$(CC) $(ASFLAGS) $(BOOT_FLAGS) -c -o $@ $<
