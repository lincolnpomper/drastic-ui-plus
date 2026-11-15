ROOT := $(CURDIR)
IMAGE := drastic-ui-plus-build

.PHONY: image rgdsplus pack-rgdsplus

image:
	docker build -t $(IMAGE) toolchain/

rgdsplus:
	docker run --rm --user root -v "$(ROOT)":/workspace $(IMAGE) sh /workspace/build.sh rgdsplus

pack-rgdsplus:
	sh pack.sh
