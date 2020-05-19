SRCDIR = ../../source
SRCS = $(SRCDIR)/RenderingPlugin.cpp \
$(SRCDIR)/RenderAPI.cpp \
$(SRCDIR)/RenderAPI_Vulkan.cpp
OBJS = ${SRCS:.cpp=.o}
CXXFLAGS = $(UNITY_DEFINES) -O2 -fPIC
LDFLAGS = -shared -rdynamic
LIBS = 
PLUGIN_SHARED = libRenderingPlugin.so
CXX ?= g++

.cpp.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: shared

clean:
	rm -f $(OBJS) $(PLUGIN_SHARED)

shared: $(OBJS)
	$(CXX) $(LDFLAGS) -o $(PLUGIN_SHARED) $(OBJS) $(LIBS)
