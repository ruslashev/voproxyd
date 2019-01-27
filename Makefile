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
          worker.c
cflags = -Wall -Wextra -g -Wno-unused-function -Wno-unused-variable -Wno-unused-parameter \
         -Wno-unused-but-set-variable
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
cpp = g++

all: $(binname)
	./$(binname)

$(binname): $(objs)
	@echo "ld $@"
	@$(cc) $(objs) $(ldflags) -o $@

$(build_dir)/%.c.o: %.c
	@echo "cc $<"
	@$(cc) -c $< $(cflags) -o $@

$(objs): | $(build_dir)

$(build_dir):
	@mkdir -p $(build_dir)

clean:
	@rm -rf $(build_dir)

prepare-onvif: unzip-gsoap wsdl2h soapcpp

unzip-gsoap:
	@echo "unzip deps/gsoap_2.8.74.zip"
	@unzip -q deps/gsoap_2.8.74.zip -d deps

wsdl2h: unzip-gsoap
	@mkdir -p deps/onvif
	@echo wsdl2h -o deps/onvif/onvif.h
	@wsdl2h -c -P -x -t deps/gsoap-2.8/gsoap/typemap.dat -o deps/onvif/onvif.h $(wsdls)

soapcpp: wsdl2h
	cat deps/onvif/onvif.h | soapcpp2 -2 -c -L -x -d deps/onvif -I 'deps/gsoap-2.8/gsoap/import/;deps/gsoap-2.8/gsoap/'

