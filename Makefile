# Author: Dylan Turner
# Description:
#  - Build all parts of the project.
#  - Note requires no spaces in path to here (or change the _BUILD_PATH vars)

# Settings

## Arduino build system settings

ARD_BASE_URL :=		https://github.com/arduino/arduino-cli/releases/download
ARD_VERS :=			0.19.3
ARD_FNAME :=		arduino-cli_$(ARD_VERS)_Linux_64bit.tar.gz
ARD_SAVED_NAME :=	arduino-cli.tar.gz
ARD_URL	:=			$(ARD_BASE_URL)/$(ARD_VERS)/$(ARD_FNAME)
ARD_FLD :=			arduino-cli
ARDC :=				$(ARD_FLD)/arduino-cli
ARD_CONF :=			arduino-cli.yaml

LIBS :=				SD

## Arduino programmer project specific settings

PGRMR_PROJNAME :=	MigsProgrammer
PGRMR_BOARD_FQBN :=	arduino:avr:uno
PGRMR_BUILD_PATH :=	build/$(PGRMR_PROJNAME)
PGRMR_SRC :=		$(PGRMR_PROJNAME)/$(PGRMR_PROJNAME).ino \
					$(wildcard $(PGRMR_PROJNAME)/*.cpp) \
					$(wildcard $(PGRMR_PROJNAME)/*.hpp)
PGRMR_OBJNAME :=	$(PGRMR_PROJNAME).ino.hex

## Arduino menu program specific settings

MENU_PROJNAME :=	MigsMenu
MENU_BOARD_FQBN :=	arduino:avr:uno
MENU_BUILD_PATH :=	build/$(MENU_PROJNAME)
MENU_SRC :=			$(MENU_PROJNAME)/$(MENU_PROJNAME).ino \
					$(wildcard $(MENU_PROJNAME)/*.cpp) \
					$(wildcard $(MENU_PROJNAME)/*.hpp)
MENU_OBJNAME :=		$(MENU_PROJNAME).ino.hex

## GPU project settings

GPU_OBJNAME :=		MigsGpu
GPU_SRC :=			$(wildcard $(GPU_OBJNAME)/src/*.cpp)
GPU_HFILES :=		$(wildcard $(GPU_OBJNAME)/include/*.hpp)
# Different bc cmake sucks:
GPU_BUILD_PATH :=	$(GPU_OBJNAME)/build

# Targets

## Helper Targets

### Note: No "all" bc there are 3 projects

.PHONY: clean
clean:
	rm -rf $(ARD_FLD)
	rm -rf $(ARD_SAVED_NAME)
	rm -rf $(PGRMR_BUILD_PATH)
	rm -rf $(PGRMR_OBJNAME)
	rm -rf $(MENU_BUILD_PATH)
	rm -rf $(MENU_OBJNAME)
	rm -rf pico-sdk
	rm -rf $(GPU_OBJNAME).uf2
	rm -rf $(GPU_BUILD_PATH)
	rm -rf $(GPU_OBJNAME)/pico_sdk_import.cmake
	rm -rf $(GPU_OBJNAME)/pico_extras_import.cmake
	rm -rf build
	rm -rf libraries

### Download and locally install arduino-cli and libraries

$(ARDC):
	curl -L $(ARD_URL) -o $(ARD_SAVED_NAME)

	mkdir -p $(ARD_FLD)
	tar xf $(ARD_SAVED_NAME) -C $(ARD_FLD)

	$(ARDC) --config-file=$(ARD_CONF) core update-index
	$(ARDC) --config-file=$(ARD_CONF) core install arduino:avr

	$(foreach lib,$(LIBS),\
		$(ARDC) --config-file=$(ARD_CONF) lib download \
			"$(subst \_, ,$(lib))"; \
		$(ARDC) --config-file=$(ARD_CONF) lib install \
			"$(subst \_, ,$(lib))"; \
	)

### Download and locally install pico-sdk and pico-extras

pico-sdk:
	git clone -b master https://github.com/raspberrypi/pico-sdk.git
	cd pico-sdk; git submodule update --init

pico-extras:
	git clone -b master https://github.com/raspberrypi/pico-extras.git
	cd pico-extras; git submodule update --init

$(GPU_OBJNAME)/pico_sdk_import.cmake: pico-sdk
	cp pico-sdk/external/pico_sdk_import.cmake $(GPU_OBJNAME)

$(GPU_OBJNAME)/pico_extras_import.cmake: pico-extras
	cp pico-extras/external/pico_extras_import.cmake $(GPU_OBJNAME)

## Main Targets

### Build programmer

$(PGRMR_OBJNAME): $(ARDC) $(PGRMR_SRC)
	mkdir -p $(PGRMR_BUILD_PATH)
	$(ARDC) compile \
		--config-file=$(ARD_CONF) \
		--fqbn $(PGRMR_BOARD_FQBN) \
		--build-path $(PGRMR_BUILD_PATH) \
		$(PGRMR_PROJNAME)
	cp $(PGRMR_BUILD_PATH)/$@ .

### Build menu program

$(MENU_OBJNAME): $(ARDC) $(MENU_SRC)
	mkdir -p $(MENU_BUILD_PATH)
	$(ARDC) compile \
		--config-file=$(ARD_CONF) \
		--fqbn $(MENU_BOARD_FQBN) \
		--build-path $(MENU_BUILD_PATH) \
		$(MENU_PROJNAME)
	cp $(MENU_BUILD_PATH)/$@ .

### Upload programmer code
#### Takes paramater PORT

.PHONY: upload-pgrmr
upload-pgrmr: $(PGRMR_OBJNAME)
	$(ARDC) upload \
		--config-file=$(ARD_CONF) \
		--fqbn $(PGRMR_BOARD_FQBN) \
		-p $(PORT) -i $<

### Note: We don't need to upload the menu program

### Build gpu program

$(GPU_OBJNAME).uf2: $(GPU_OBJNAME)/pico_sdk_import.cmake $(GPU_SRC) $(GPU_HFILES) $(GPU_OBJNAME)/CMakeLists.txt $(GPU_OBJNAME)/pico_extras_import.cmake
	mkdir -p $(GPU_BUILD_PATH)/pico-sdk
	cp -r pico-sdk/lib $(GPU_BUILD_PATH)/pico-sdk
	cp -r pico-sdk/src $(GPU_BUILD_PATH)/pico-sdk
	cp -r pico-sdk/cmake $(GPU_BUILD_PATH)/pico-sdk
	cp -r pico-sdk/tools $(GPU_BUILD_PATH)/pico-sdk
	cp -r pico-sdk/external $(GPU_BUILD_PATH)/pico-sdk
	cp -r pico-sdk/test $(GPU_BUILD_PATH)/pico-sdk
	cp -r pico-sdk/docs $(GPU_BUILD_PATH)/pico-sdk
	cp -r pico-sdk/CMakeLists.txt $(GPU_BUILD_PATH)/pico-sdk
	cp pico-sdk/*.cmake $(GPU_BUILD_PATH)/pico-sdk

	mkdir -p $(GPU_BUILD_PATH)/pico-extras
	cp -r pico-extras/lib $(GPU_BUILD_PATH)/pico-extras
	cp -r pico-extras/src $(GPU_BUILD_PATH)/pico-extras
	cp -r pico-extras/external $(GPU_BUILD_PATH)/pico-extras
	cp -r pico-extras/test $(GPU_BUILD_PATH)/pico-extras
	cp -r pico-extras/CMakeLists.txt $(GPU_BUILD_PATH)/pico-extras
	cp pico-extras/*.cmake $(GPU_BUILD_PATH)/pico-extras

	cd $(GPU_BUILD_PATH); PICO_SDK_PATH=pico-sdk PICO_EXTRAS_PATH=pico-extras cmake ..
	make -C $(GPU_BUILD_PATH)
	cp $(GPU_BUILD_PATH)/$@ .
