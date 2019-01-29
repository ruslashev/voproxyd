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
          $(wildcard deps/onvif/*.cpp)
cflags = -Wall -Wextra -g -Wno-unused-function -Wno-unused-variable -Wno-unused-parameter \
         -Wno-unused-but-set-variable
cxxflags = $(cflags) -Wno-nonnull-compare -Wno-address -Wno-misleading-indentation -O0
ldflags =
binname = voproxyd
wsdls = https://www.onvif.org/ver10/advancedsecurity/wsdl/advancedsecurity.wsdl \
        https://www.onvif.org/ver10/deviceio.wsdl \
        https://www.onvif.org/ver10/device/wsdl/devicemgmt.wsdl \
        https://www.onvif.org/ver10/events/wsdl/event.wsdl \
        https://www.onvif.org/ver10/media/wsdl/media.wsdl \
        https://www.onvif.org/ver10/network/wsdl/remotediscovery.wsdl \
        https://www.onvif.org/ver20/imaging/wsdl/imaging.wsdl \
        https://www.onvif.org/ver20/ptz/wsdl/ptz.wsdl
wsdls_all = https://www.onvif.org/ver10/actionengine.wsdl \
            https://www.onvif.org/ver10/advancedsecurity/wsdl/advancedsecurity.wsdl \
            https://www.onvif.org/ver10/authenticationbehavior/wsdl/authenticationbehavior.wsdl \
            https://www.onvif.org/ver10/credential/wsdl/credential.wsdl \
            https://www.onvif.org/ver10/deviceio.wsdl \
            https://www.onvif.org/ver10/device/wsdl/devicemgmt.wsdl \
            https://www.onvif.org/ver10/display.wsdl \
            https://www.onvif.org/ver10/events/wsdl/event.wsdl \
            https://www.onvif.org/ver10/media/wsdl/media.wsdl \
            https://www.onvif.org/ver10/pacs/accesscontrol.wsdl \
            https://www.onvif.org/ver10/pacs/doorcontrol.wsdl \
            https://www.onvif.org/ver10/provisioning/wsdl/provisioning.wsdl \
            https://www.onvif.org/ver10/receiver.wsdl \
            https://www.onvif.org/ver10/recording.wsdl \
            https://www.onvif.org/ver10/replay.wsdl \
            https://www.onvif.org/ver10/schedule/wsdl/schedule.wsdl \
            https://www.onvif.org/ver10/search.wsdl \
            https://www.onvif.org/ver10/thermal/wsdl/thermal.wsdl \
            https://www.onvif.org/ver10/uplink/wsdl/uplink.wsdl \
            https://www.onvif.org/ver20/analytics/wsdl/analytics.wsdl \
            https://www.onvif.org/ver20/imaging/wsdl/imaging.wsdl \
            https://www.onvif.org/ver20/media/wsdl/media.wsdl \
            https://www.onvif.org/ver20/ptz/wsdl/ptz.wsdl \
            https://www.onvif.org/ver10/network/wsdl/remotediscovery.wsdl

build_dir = .obj
objs = $(sources:%=$(build_dir)/%.o)
cc = gcc
cxx = g++

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

$(build_dir):
	@mkdir -p $(build_dir)
	@mkdir -p $(build_dir)/deps/onvif
	@mkdir -p $(build_dir)/deps/gsoap-2.8/gsoap

prepare-onvif: unzip-gsoap wsdl2h soapcpp copy-gsoap-sources

unzip-gsoap:
	@echo "unzip deps/gsoap_2.8.74.zip"
	@unzip -q deps/gsoap_2.8.74.zip -d deps

compile-gsoap: unzip-gsoap
	@cd deps/gsoap-2.8 && ./configure
	@cd deps/gsoap-2.8 && make -j 1
	@mkdir -p deps/gsoap-bin
	@cd deps/gsoap-2.8 && sudo make install exec_prefix="$(pwd)/../gsoap-bin"

wsdl2h: compile-gsoap
	@mkdir -p deps/onvif
	@echo wsdl2h -o deps/onvif/onvif.h
	@./deps/gsoap-bin/wsdl2h -c++11 -P -x -s -t deps/gsoap-2.8/gsoap/typemap.dat -o deps/onvif/onvif.h $(wsdls)

soapcpp: wsdl2h
	@echo soapcpp2 deps/onvif/onvif.h
	@./deps/gsoap-bin/soapcpp2 -2 -j -c++11 -x -d deps/onvif deps/onvif/onvif.h || true

copy-gsoap-sources: unzip-gsoap
	@mkdir -p deps/onvif
	@cp deps/gsoap-2.8/gsoap/stdsoap2.cpp deps/onvif/
	@cp deps/gsoap-2.8/gsoap/stdsoap2.h deps/onvif/

clean:
	@rm -rf $(build_dir)

clean-onvif:
	@rm -rf deps/gsoap-2.8/
	@sudo rm -rf deps/gsoap-bin/
	@rm -rf deps/onvif/

