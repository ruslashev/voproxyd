sources = address_manager.c \
          avltree.c \
          bridge_commands.c \
          bridge_inquiries.c \
          buffer.c \
          config.c \
          daemonize.c \
          discovery.c \
          epoll.c \
          main.c \
          soap_global.c \
          soap_instance.c \
          soap_ptz.c \
          soap_utils.c \
          socket.c \
          sony_visca.c \
          sony_visca_commands.c \
          sony_visca_inquiries.c \
          visca.c \
          worker.c \
          wsdd_callbacks.c \
          deps/inih/ini.c \
          $(wildcard deps/onvif/*.c)
cflags = -Wall -Wextra -g -Wno-unused-function -Wno-unused-variable -Wno-unused-parameter \
         -Wno-unused-but-set-variable -Wno-misleading-indentation -Wno-deprecated-declarations \
         -DWITH_OPENSSL -DWITH_DOM -DWITH_ZLIB -I deps/onvif
ldflags = -L deps/gsoap-install/lib -lssl -lcrypto -lz
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
wsdlflags = -c -x -O4 -t deps/gsoap-2.8/gsoap/typemap.dat -o deps/onvif/onvif.h $(wsdls)
soapcppflags = -2 -L -c -x -C -d deps/onvif -I 'deps/gsoap-2.8/gsoap/:deps/gsoap-2.8/gsoap/import'
soapcpp_wsdd_flags = -a -L -c -x -C -pwsdd -d deps/onvif/wsdd/ -I deps/gsoap-2.8/gsoap/import/
verbose = 0
ifeq ($(verbose),0)
    configure_verbosity = > ../logs/configure.log 2>&1
    make_verbosity = > ../logs/make.log 2>&1
    wsdl_verbosity = > deps/logs/wsdl.log 2>&1
    soapcpp_verbosity = > deps/logs/soapcpp.log 2>&1
    soapcpp_wsdd_verbosity = > deps/logs/soapcpp_wsdd.log 2>&1
endif
example_sources = onvif_example/main.c soap_utils.c $(wildcard deps/onvif/*.c)
example_objs = $(example_sources:%=$(build_dir)/%.o)
example_binname = example
inih_url = https://raw.githubusercontent.com/benhoyt/inih/master

all: $(binname)

exec: $(binname)
	./$(binname)

$(binname): $(objs)
	@echo "ld $@"
	@$(cc) $(objs) $(ldflags) -o $@
	@echo "successfully compiled $@"

$(build_dir)/%.c.o: %.c
	@echo "cc $<"
	@$(cc) -c $< $(cflags) -o $@

$(objs): deps/inih/ini.c | $(build_dir)
$(example_objs): | $(build_dir)

$(build_dir):
	@mkdir -p $(build_dir)
	@mkdir -p $(build_dir)/deps/onvif
	@mkdir -p $(build_dir)/deps/onvif/wsdd
	@mkdir -p $(build_dir)/onvif_example
	@mkdir -p $(build_dir)/deps/inih

$(example_binname): $(example_objs)
	@echo "ld $@"
	@$(cc) $(example_objs) $(ldflags) -o $@

deps/inih/ini.c:
	@echo "download inih"
	@mkdir -p deps/inih
	@wget -q -O deps/inih/ini.c $(inih_url)/ini.c
	@wget -q -O deps/inih/ini.h $(inih_url)/ini.h

prepare-onvif: unzip-gsoap compile-gsoap install-gsoap wsdl2h soapcpp soapcpp-wsdd \
    copy-gsoap-sources move-nsmaps
	@echo "prepare successul"

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
	@cp deps/gsoap-2.8/gsoap/stdsoap2.c        deps/onvif
	@cp deps/gsoap-2.8/gsoap/stdsoap2.h        deps/onvif
	@cp deps/gsoap-2.8/gsoap/dom.c             deps/onvif
	@cp deps/gsoap-2.8/gsoap/plugin/wsaapi.c   deps/onvif
	@cp deps/gsoap-2.8/gsoap/plugin/wsaapi.h   deps/onvif
	@cp deps/gsoap-2.8/gsoap/plugin/wsddapi.c  deps/onvif
	@cp deps/gsoap-2.8/gsoap/plugin/wsddapi.h  deps/onvif
	@cp deps/gsoap-2.8/gsoap/plugin/wsseapi.c  deps/onvif
	@cp deps/gsoap-2.8/gsoap/plugin/wsseapi.h  deps/onvif
	@cp deps/gsoap-2.8/gsoap/plugin/threads.c  deps/onvif
	@cp deps/gsoap-2.8/gsoap/plugin/threads.h  deps/onvif
	@cp deps/gsoap-2.8/gsoap/plugin/smdevp.c   deps/onvif
	@cp deps/gsoap-2.8/gsoap/plugin/smdevp.h   deps/onvif
	@cp deps/gsoap-2.8/gsoap/plugin/mecevp.c   deps/onvif
	@cp deps/gsoap-2.8/gsoap/plugin/mecevp.h   deps/onvif
	@cp deps/gsoap-2.8/gsoap/custom/duration.c deps/onvif
	@cp deps/gsoap-2.8/gsoap/custom/duration.h deps/onvif

move-nsmaps: soapcpp
	@mkdir -p deps/onvif/nsmaps
	@echo move nsmaps
	@mv deps/onvif/*.nsmap deps/onvif/nsmaps

clean:
	@rm -rf $(build_dir)
	@rm -f $(binname)
	@rm -f $(example_binname)
	@rm -rf deps/inih

clean-onvif:
	@rm -rf deps/gsoap-2.8/
	@rm -rf deps/gsoap-install/
	@rm -rf deps/logs/
	@rm -rf deps/onvif/

clean-all: clean clean-onvif

wcl:
	@wc -l $(wildcard *.c) $(wildcard *.h) Makefile

