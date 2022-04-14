OVMFDIR = ovmf
OSNAME = HydrogenOS

all:
	cd boot; make; cd ../;
	mv boot/*.efi bin/
	rm boot/*.o
	dd if=/dev/zero of=$(OSNAME).img bs=512 count=93750
	mformat -i $(OSNAME).img
	mmd -i $(OSNAME).img ::/EFI
	mmd -i $(OSNAME).img ::/EFI/BOOT
	mcopy -i $(OSNAME).img bin/startup.nsh ::
	mcopy -i $(OSNAME).img boot/assets/rb.png ::
	mcopy -i $(OSNAME).img bin/boot.efi ::/EFI/BOOT
	# mcopy -i $(OSNAME).img $(BUILDDIR)/kernel.elf ::
	mcopy -i $(OSNAME).img bin//zap-light16.psf ::

run:
	qemu-system-x86_64 -drive file=$(OSNAME).img -m 256M -cpu qemu64 -drive if=pflash,format=raw,unit=0,file="$(OVMFDIR)/OVMF_CODE-pure-efi.fd",readonly=on -drive if=pflash,format=raw,unit=1,file="$(OVMFDIR)/OVMF_VARS-pure-efi.fd" -net none -serial stdio -d int -no-reboot -D logfile.txt -M smm=off

run_loopback:
	qemu-system-x86_64 -drive file=/dev/loop0 -m 256M -cpu qemu64 -drive if=pflash,format=raw,unit=0,file="ovmf/OVMF_CODE-pure-efi.fd",readonly=on -drive if=pflash,format=raw,unit=1,file="ovmf/OVMF_VARS-pure-efi.fd" -net none -serial stdio -d int -no-reboot -D logfile.txt -M smm=off
