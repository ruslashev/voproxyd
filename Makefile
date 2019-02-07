sources = avltree.c \
          bridge.c \
          bridge_commands.c \
          bridge_inquiries.c \
          buffer.c \
          daemonize.c \
          epoll.c \
          main.c \
          socket.c \
          visca.c \
          visca_commands.c \
          visca_inquiries.c \
          worker.c \
          $(wildcard deps/onvif/*.cpp) \
          $(wildcard deps/onvif/wsdd/*.cpp)
cflags = -Wall -Wextra -g -Wno-unused-function -Wno-unused-variable -Wno-unused-parameter \
         -Wno-unused-but-set-variable
cxxflags = $(cflags) -Wno-nonnull-compare -Wno-address -Wno-misleading-indentation -O0 \
           -DWITH_OPENSSL -DWITH_DOM -DWITH_ZLIB -I deps/onvif/
ldflags =
binname = voproxyd
wsdls = https://www.onvif.org/ver10/device/wsdl/devicemgmt.wsdl \
        https://www.onvif.org/ver10/events/wsdl/event.wsdl \
        https://www.onvif.org/ver10/pacs/accesscontrol.wsdl \
        https://www.onvif.org/ver10/actionengine.wsdl \
        https://www.onvif.org/ver20/analytics/wsdl/analytics.wsdl \
        https://www.onvif.org/ver10/authenticationbehavior/wsdl/authenticationbehavior.wsdl \
        https://www.onvif.org/ver10/credential/wsdl/credential.wsdl \
        https://www.onvif.org/ver10/deviceio.wsdl \
        https://www.onvif.org/ver10/display.wsdl \
        https://www.onvif.org/ver10/pacs/doorcontrol.wsdl \
        https://www.onvif.org/ver20/imaging/wsdl/imaging.wsdl \
        https://www.onvif.org/ver10/media/wsdl/media.wsdl \
        https://www.onvif.org/ver20/media/wsdl/media.wsdl \
        https://www.onvif.org/ver10/network/wsdl/remotediscovery.wsdl \
        https://www.onvif.org/ver10/provisioning/wsdl/provisioning.wsdl \
        https://www.onvif.org/ver20/ptz/wsdl/ptz.wsdl \
        https://www.onvif.org/ver10/receiver.wsdl \
        https://www.onvif.org/ver10/recording.wsdl \
        https://www.onvif.org/ver10/search.wsdl \
        https://www.onvif.org/ver10/replay.wsdl \
        https://www.onvif.org/ver10/schedule/wsdl/schedule.wsdl \
        https://www.onvif.org/ver10/advancedsecurity/wsdl/advancedsecurity.wsdl \
        https://www.onvif.org/ver10/thermal/wsdl/thermal.wsdl \
        https://www.onvif.org/ver10/uplink/wsdl/uplink.wsdl
build_dir = .obj
objs = $(sources:%=$(build_dir)/%.o)
cc = gcc
cxx = g++
wsdlflags = -c++11 -x -O4 -t deps/gsoap-2.8/gsoap/typemap.dat -o deps/onvif/onvif.h $(wsdls)
soapcppflags = -2 -L -j -c++11 -x -C -d deps/onvif -I deps/gsoap-2.8/gsoap/
soapcpp_wsdd_flags = -a -L -c++11 -x -C -pwsdd -d deps/onvif/wsdd/ -I deps/gsoap-2.8/gsoap/import/
verbose = 0
ifeq ($(verbose),0)
    configure_verbosity = > ../logs/configure.log 2>&1
    make_verbosity = > ../logs/make.log 2>&1
    wsdl_verbosity = > deps/logs/wsdl.log 2>&1
    soapcpp_verbosity = > deps/logs/soapcpp.log 2>&1
    soapcpp_wsdd_verbosity = > deps/logs/soapcpp.log 2>&1
endif
example_sources = onvif_example/main.cpp \
                  deps/onvif/soapAdvancedSecurityServiceBindingProxy.cpp \
                  deps/onvif/soapDeviceBindingProxy.cpp \
                  deps/onvif/soapDeviceIOBindingProxy.cpp \
                  deps/onvif/soapImagingBindingProxy.cpp \
                  deps/onvif/soapMediaBindingProxy.cpp \
                  deps/onvif/soapPTZBindingProxy.cpp \
                  deps/onvif/soapPullPointSubscriptionBindingProxy.cpp \
                  deps/onvif/soapRemoteDiscoveryBindingProxy.cpp \
                  deps/onvif/stdsoap2.cpp \
                  deps/onvif/dom.cpp \
                  deps/onvif/soapC.cpp \
                  deps/onvif/wsdd/wsddClient.cpp \
                  deps/onvif/smdevp.cpp \
                  deps/onvif/mecevp.cpp \
                  deps/onvif/wsaapi.cpp \
                  deps/onvif/wsseapi.cpp \
                  deps/onvif/wsddapi.cpp
example_objs = $(example_sources:%=$(build_dir)/%.o)
example_ldflags = -L deps/gsoap-install/lib -lssl -lcrypto -lz
example_binname = example

all: $(binname)
	./$(binname)

$(binname): $(objs)
	@echo "ld $@"
	@$(cxx) $(objs) $(ldflags) -o $@

$(build_dir)/%.c.o: %.c
	@echo "cc $<"
	@$(cc) -c $< $(cflags) -o $@

$(build_dir)/%.cpp.o: %.cpp
	@echo "cxx $<"
	@$(cxx) -c $< $(cxxflags) -o $@

$(objs): | $(build_dir)
$(example_objs): | $(build_dir)

$(build_dir):
	@mkdir -p $(build_dir)
	@mkdir -p $(build_dir)/deps/onvif
	@mkdir -p $(build_dir)/deps/onvif/wsdd
	@mkdir -p $(build_dir)/deps/gsoap-2.8/gsoap
	@mkdir -p $(build_dir)/onvif_example

$(example_binname): $(example_objs)
	@echo "ld $@"
	@$(cxx) $(example_objs) $(example_ldflags) -o $@

prepare-onvif: unzip-gsoap compile-gsoap install-gsoap wsdl2h soapcpp soapcpp-wsdd copy-gsoap-sources

unzip-gsoap:
	@echo "unzip deps/gsoap_2.8.74.zip"
	@unzip -q deps/gsoap_2.8.74.zip -d deps

compile-gsoap: unzip-gsoap
	@echo configure gsoap
	@mkdir -p deps/logs
	@cd deps/gsoap-2.8 && ./configure $(configure_verbosity)
	@echo make gsoap
	@cd deps/gsoap-2.8 && make -j 1 $(make_verbosity)

install-gsoap: compile-gsoap
	@echo install gsoap
	@mkdir -p deps/gsoap-install/bin
	@cp deps/gsoap-2.8/gsoap/wsdl/wsdl2h  deps/gsoap-install/bin
	@cp deps/gsoap-2.8/gsoap/src/soapcpp2 deps/gsoap-install/bin
	@mkdir -p deps/gsoap-install/lib
	@cp deps/gsoap-2.8/gsoap/libgsoap++.a    deps/gsoap-install/lib
	@cp deps/gsoap-2.8/gsoap/libgsoapssl++.a deps/gsoap-install/lib
	@cp deps/gsoap-2.8/gsoap/libgsoap.a      deps/gsoap-install/lib
	@cp deps/gsoap-2.8/gsoap/libgsoapssl.a   deps/gsoap-install/lib

wsdl2h: install-gsoap
	@mkdir -p deps/onvif
	@echo wsdl2h -o deps/onvif/onvif.h
	@echo 'xsd__duration = #import "custom/duration.h" | xsd__duration' >> \
		deps/gsoap-2.8/gsoap/typemap.dat
	@./deps/gsoap-install/bin/wsdl2h $(wsdlflags) $(wsdl_verbosity)
	@sed -i '/#import/ a #import "wsse.h"' deps/onvif/onvif.h
	@sed -i 's/wsdd10.h/wsdd5.h/g' deps/onvif/onvif.h

soapcpp: wsdl2h
	@echo soapcpp2 deps/onvif/onvif.h
	@./deps/gsoap-install/bin/soapcpp2 $(soapcppflags) deps/onvif/onvif.h $(soapcpp_verbosity)

soapcpp-wsdd: wsdl2h
	@echo soapcpp2 deps/gsoap-2.8/gsoap/import/wsdd5.h
	@mkdir -p deps/onvif/wsdd
	@./deps/gsoap-install/bin/soapcpp2 $(soapcpp_wsdd_flags) deps/gsoap-2.8/gsoap/import/wsdd5.h \
		$(soapcpp_wsdd_verbosity)

copy-gsoap-sources: unzip-gsoap
	@mkdir -p deps/onvif
	@echo copy gsoap sources
	@cp deps/gsoap-2.8/gsoap/stdsoap2.cpp      deps/onvif
	@cp deps/gsoap-2.8/gsoap/stdsoap2.h        deps/onvif
	@cp deps/gsoap-2.8/gsoap/dom.cpp           deps/onvif
	@cp deps/gsoap-2.8/gsoap/plugin/wsaapi.c   deps/onvif/wsaapi.cpp
	@cp deps/gsoap-2.8/gsoap/plugin/wsaapi.h   deps/onvif
	@cp deps/gsoap-2.8/gsoap/plugin/wsddapi.c  deps/onvif/wsddapi.cpp
	@cp deps/gsoap-2.8/gsoap/plugin/wsddapi.h  deps/onvif
	@cp deps/gsoap-2.8/gsoap/plugin/wsseapi.c  deps/onvif/wsseapi.cpp
	@cp deps/gsoap-2.8/gsoap/plugin/wsseapi.h  deps/onvif
	@cp deps/gsoap-2.8/gsoap/plugin/threads.c  deps/onvif/threads.cpp
	@cp deps/gsoap-2.8/gsoap/plugin/threads.h  deps/onvif
	@cp deps/gsoap-2.8/gsoap/plugin/smdevp.c   deps/onvif/smdevp.cpp
	@cp deps/gsoap-2.8/gsoap/plugin/smdevp.h   deps/onvif
	@cp deps/gsoap-2.8/gsoap/plugin/mecevp.c   deps/onvif/mecevp.cpp
	@cp deps/gsoap-2.8/gsoap/plugin/mecevp.h   deps/onvif
	@cp deps/gsoap-2.8/gsoap/custom/duration.c deps/onvif/duration.cpp
	@cp deps/gsoap-2.8/gsoap/custom/duration.h deps/onvif

clean:
	@rm -rf $(build_dir)
	@rm -f $(binname)

clean-onvif:
	@rm -rf deps/gsoap-2.8/
	@rm -rf deps/gsoap-install/
	@rm -rf deps/logs/
	@rm -rf deps/onvif/

clean-all: clean clean-onvif

