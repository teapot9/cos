source -v scripts/gdb/qemu-efi.py
efi

# Reload symbols to kernel offset (0xffff...)
tbreak entry_efi_wrapper_s2
commands
symbol-file
symbol-file -readnow build/kernel.elf
cont
end
