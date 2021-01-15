ifndef $(CONFIG)
	CONFIG := release
endif

CURR_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
BASE_BUILDDIR:=$(CURR_DIR)/build
BUILDDIR=$(BASE_BUILDDIR)/$(CONFIG)

SOURCES = $(wildcard $(CURR_DIR)/*.cpp) \
		  $(wildcard $(CURR_DIR)/traffic/*.cpp) \
		  $(wildcard $(CURR_DIR)/topology/*.cpp) \
		  $(wildcard $(CURR_DIR)/utils/*.cpp) \
		  $(wildcard $(CURR_DIR)/simulation/*.cpp)

OBJECTS = $(patsubst $(CURR_DIR)/%,$(BUILDDIR)/%,$(SOURCES:.cpp=.o))
DEPENDS = $(patsubst $(CURR_DIR)/%,$(BUILDDIR)/%,$(SOURCES:.cpp=.d))

RELEASE_BINARY := syndb-sim
DEBUG_BINARY := syndb-sim-debug

CXXFLAGS  = -DBOOST_LOG_DYN_LINK \
			-I$(CURR_DIR) \
			-std=c++11

LDLIBS = -lboost_log \
		 -lpthread \
		 -lfmt

ifeq ($(CONFIG), release)
CXXFLAGS += -O3
OUTPUT_BINARY := $(RELEASE_BINARY)
else
CXXFLAGS += -O0 -g3 -DDEBUG
OUTPUT_BINARY := $(DEBUG_BINARY)
endif


.PHONY: all clean cleaner

all: $(OUTPUT_BINARY)

clean:
	@$(RM) -rf $(BUILDDIR)
	@$(RM) $(OUTPUT_BINARY)

cleaner:
	@$(RM) -rf $(BASE_BUILDDIR)
	@$(RM) $(RELEASE_BINARY) $(DEBUG_BINARY)


# Linking the executable from the object files
$(OUTPUT_BINARY): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDLIBS)

-include $(DEPENDS)

# -MMD -MP are related to generating the .d depends file
$(BUILDDIR)/%.o: $(CURR_DIR)/%.cpp Makefile
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@
