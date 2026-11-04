# Lean OpenMS library for the fuzz/test builds (upstream's CMake needs Arrow, HiGHS, libxml2>=2.13, ...).
# usage: make -f mayhem/Makefile OUT=build/fuzz CXXFLAGS_X="<flags>" -j N
OMS := src/openms
SOURCES := $(shell cat mayhem/sources.txt)
OBJS := $(patsubst %.cpp,$(OUT)/obj/%.o,$(SOURCES)) $(OUT)/obj/arrow_stubs.o

$(OUT)/libopenms-lite.a: $(OBJS)
	@rm -f $@
	@echo AR $@
	@$(AR) rcs $@ $(OBJS)

$(OUT)/obj/arrow_stubs.o: mayhem/arrow_stubs.cc $(OUT)/flags.stamp
	@mkdir -p $(dir $@)
	@echo CXX $<
	@$(CXX) $(CXXFLAGS_X) -MMD -MP -c $< -o $@

$(OUT)/obj/%.o: $(OMS)/%.cpp $(OUT)/flags.stamp
	@mkdir -p $(dir $@)
	@echo CXX $<
	@$(CXX) $(CXXFLAGS_X) -MMD -MP -c $< -o $@

-include $(OBJS:.o=.d)
