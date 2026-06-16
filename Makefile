.DEFAULT_GOAL := shell

TMUX_FILE := $(HOME)/.tmux.conf
TMUX_REAL_PATH := $(shell readlink -f $(TMUX_FILE))
TMUX_MOUNT := $(shell if [ -f $(TMUX_REAL_PATH) ]; then echo "-v $(TMUX_REAL_PATH):/root/.tmux.conf:Z" ; fi)


CONTAINER_CMD = podman
CONTAINER_NAME = apue
FILES_TO_MOUNT = -v $(shell pwd)/apue.3e:/apue/apue.3e:Z \
                 -v ./entrypoint/shell.sh:/usr/local/bin/shell.sh:Z \
                 -v ./entrypoint/dotfiles/.extrabashrc:/root/.extrabashrc:Z \
                 $(TMUX_MOUNT) \




USE_X = -e DISPLAY=$(DISPLAY) \
	-v /tmp/.X11-unix:/tmp/.X11-unix \
	--security-opt label=type:container_runtime_t
WAYLAND_FLAGS_FOR_CONTAINER = -e "WAYLAND_DISPLAY=${WAYLAND_DISPLAY}" \
                              -e "XDG_RUNTIME_DIR=${XDG_RUNTIME_DIR}" \
                              -v "${XDG_RUNTIME_DIR}:${XDG_RUNTIME_DIR}"

ALLOW_LLDB= --cap-add=SYS_ADMIN \
            --security-opt seccomp=unconfined


.PHONY: all
all: shell ## Build the image and get a shell in it

.PHONY: image
image: ## Build podman image to run the examples
	# build the container
	$(CONTAINER_CMD) build \
                         -t $(CONTAINER_NAME) \
                         .


.PHONY: shell
shell: image  ## Get Shell into a ephermeral container made from the image
	$(CONTAINER_CMD) run -it --rm \
		--entrypoint /bin/bash \
		$(FILES_TO_MOUNT) \
		$(USE_X) \
		$(WAYLAND_FLAGS_FOR_CONTAINER) \
		$(ALLOW_LLDB) \
		$(CONTAINER_NAME) \
		/usr/local/bin/shell.sh



.PHONY: format
format: image ## Format the C code
	$(CONTAINER_CMD) run -it --rm \
		--entrypoint /bin/bash \
		$(FILES_TO_MOUNT) \
		$(USE_X) \
		$(CONTAINER_NAME) \
		/usr/local/bin/format.sh


.PHONY: build
build: image ## Configure + compile + test the meson build inside the container
	$(CONTAINER_CMD) run -it --rm \
		--entrypoint /bin/bash \
		$(FILES_TO_MOUNT) \
		$(CONTAINER_NAME) \
		-c 'cd /apue/apue.3e && \
		    if [ -d build ]; then \
		        meson setup --wipe build --native-file native-linux.ini; \
		    else \
		        meson setup build --native-file native-linux.ini; \
		    fi && \
		    meson compile -C build && \
		    meson test -C build --print-errorlogs'


.PHONY: image-export
image-export: ## export the OCI image to a timestamped tar in the repo root
	$(CONTAINER_CMD) save $(CONTAINER_NAME) -o $(CONTAINER_NAME)-$(shell date +%m-%d-%Y_%H-%M-%S).tar

.PHONY: image-import
image-import: ## import an OCI image tar: make image-import FILE=foo.tar
	$(CONTAINER_CMD) load -i $(FILE)

.PHONY: help
help:
	@grep --extended-regexp '^[a-zA-Z0-9_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}'
