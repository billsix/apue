.DEFAULT_GOAL := help

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


# --- shell / shell-exec share ONE container invocation, defined here so the two
# targets can never drift. Scoped to this pair ONLY. See runClaudeInContainer
# tasks/add-shell-exec-target.md. NOTE: only apue.3e is mounted (not the whole
# repo), so CMD='...' always works but SCRIPT=path only for a mounted path.
SHELL_RUN_FLAGS = \
		--entrypoint /bin/bash \
		$(FILES_TO_MOUNT) \
		$(USE_X) \
		$(WAYLAND_FLAGS_FOR_CONTAINER) \
		$(ALLOW_LLDB)

# In-container mount root (shell.sh cd's here; apue.3e is mounted under it).
REPO_MOUNT = /apue

# shell-exec payload: cd to the mount root, then run the inline CMD, else the
# (mounted) SCRIPT. Prefers CMD when both are set.
SHELL_EXEC_ARGS = -c 'cd $(REPO_MOUNT) && $(if $(CMD),$(CMD),exec bash $(SCRIPT))'

.PHONY: shell
shell: image  ## Get Shell into a ephermeral container made from the image
	$(CONTAINER_CMD) run -it --rm $(SHELL_RUN_FLAGS) $(CONTAINER_NAME) /usr/local/bin/shell.sh

.PHONY: shell-exec
shell-exec: image ## Run a script/command in the container env (no TTY): make shell-exec SCRIPT=path | CMD='...'
	@[ -n "$(SCRIPT)$(CMD)" ] || { echo 'usage: make shell-exec SCRIPT=<mounted path> | CMD="..."'; exit 2; }
	$(CONTAINER_CMD) run --rm $(SHELL_RUN_FLAGS) $(CONTAINER_NAME) /usr/local/bin/shell.sh $(SHELL_EXEC_ARGS)



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
