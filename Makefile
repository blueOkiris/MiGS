# Author: Dylan Turner
# Description:
#  - Build all parts of the project.
#  - Note requires no spaces in path to here (or change the _BUILD_PATH vars)

# Settings

## Arduino build System Settings

ARD_BASE_URL :=		https://github.com/arduino/arduino-cli/releases/download
ARD_VERS :=			0.19.3
ARD_FNAME :=		arduino-cli_$(ARD_VERS)_Linux_64bit.tar.gz
ARD_SAVED_NAME :=	arduino-cli.tar.gz
ARD_URL	:=			$(ARD_BASE_URL)/$(ARD_VERS)/$(ARD_FNAME)
ARD_FLD :=			arduino-cli
ARDC :=				$(ARD_FLD)/arduino-cli
ARD_CONF :=			arduino-cli.yaml

LIBS :=				SD

## Arduino project specific libraries

PGRMR_PROJNAME :=	MigsProgrammer
PGRMR_BOARD_FQBN :=	arduino:avr:uno
PGRMR_BUILD_PATH :=	build/$(PGRMR_PROJNAME)
PGRMR_SRC :=		$(PGRMR_PROJNAME)/$(PGRMR_PROJNAME).ino \
					$(wildcard $(PGRMR_PROJNAME)/*.cpp) \
					$(wildcard $(PGRMR_PROJNAME)/*.hpp)
PGRMR_OBJNAME :=	$(PGRMR_PROJNAME).ino.hex

MENU_PROJNAME :=	MigsMenu
MENU_BOARD_FQBN :=	arduino:avr:uno
MENU_BUILD_PATH :=	build/$(MENU_PROJNAME)
MENU_SRC :=			$(MENU_PROJNAME)/$(MENU_PROJNAME).ino \
					$(wildcard $(MENU_PROJNAME)/*.cpp) \
					$(wildcard $(MENU_PROJNAME)/*.hpp)
MENU_OBJNAME :=		$(MENU_PROJNAME).ino.hex

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
