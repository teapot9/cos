#!/bin/bash
set -x

mode="${1:-all}"
[ $# -ne 0 ] && shift

KERNEL="${KERNEL:-cos}"
BUILD="${BUILD:-build}"
DEBUG_LOG="${DEBUG_LOG:-debug.log}"
QEMU_LOG="${QEMU_LOG:-qemu.log}"
BOOT=arch/x86/boot

if [ "${mode}" = efi ]; then
	exec 3>&1 1>&2
	mkdir -pv "${BUILD}"/EFI/BOOT
	cp -v "${BOOT}/${KERNEL}.efi" "${BUILD}"/EFI/BOOT/BOOTX64.EFI
	chmod +x "${BUILD}"/EFI/BOOT/BOOTX64.EFI
	# tmp efi shell:
	#cp -v /usr/share/edk2-ovmf/Shell.efi "${BUILD}"/EFI/BOOT/BOOTX64.EFI
	#cp -v "${KERNEL}.efi" "${BUILD}"/EFI/BOOT/KERNEL.EFI
	cp -v /usr/share/edk2-ovmf/OVMF_CODE.fd "${BUILD}"/OVMF_CODE.fd
	cp -v /usr/share/edk2-ovmf/OVMF_VARS.fd "${BUILD}"/OVMF_VARS.fd
	rm -f "${DEBUG_LOG}" "${QEMU_LOG}"
	qemu-system-x86_64 \
		-nodefaults \
		-d int,cpu_reset,guest_errors \
		-D "${QEMU_LOG}" \
		-no-reboot \
		-no-shutdown \
		-cpu qemu64 \
		-machine q35 \
		-m 128 \
		-smp 2 \
		-drive if=pflash,format=raw,unit=0,file="${BUILD}"/OVMF_CODE.fd,readonly=on \
		-drive if=pflash,format=raw,unit=1,file="${BUILD}"/OVMF_VARS.fd,readonly=off \
		-drive file=fat:rw:"${BUILD}" \
		-debugcon file:"${DEBUG_LOG}" \
		-global isa-debugcon.iobase=0x402 \
		-serial file:debug.log \
		-vga std \
		-s \
		"$@"
	trap "kill $!" TERM
	trap "kill $!" INT
	wait
elif [ "${mode}" = gdb ]; then
	args=()
	for arg in "$@"; do [ -n "${arg}" ] && args+=( -ex "$arg" ); done
	gdb \
		-ex "info files" \
		-ex "file" \
		-ex "set architecture i386:x86-64:intel" \
		-ex "set disassembly-flavor intel" \
		-ex "target remote :1234" \
		"${args}"
elif [ "${mode}" = egdb ]; then
	args=()
	for arg in "$@"; do [ -n "${arg}" ] && args+=( -ex "$arg" ); done
	gdb \
		-ex 'source uefi.py' \
		-ex 'efi' \
		"$@"
elif [ "${mode}" = all ]; then
	rm debug.log
	"$0" efi &
	disown
	while [ ! -f debug.log ]; do sleep 0.1; done
	"$0" egdb  # -ex 'break entry_efi'
	kill $!
fi

